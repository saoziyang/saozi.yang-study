#include "camera.h"
#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define CAM_W   640
#define CAM_H   480

static int is_send_over = 0;
camera::camera()
{
    printf("%s\n", __func__);
    fd = open("/dev/video0", O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        printf("open video error\n");
        exit(-1);
    }

    

    buffers = NULL;
    send_buffer = malloc(640*480*2);
}

void camera::setup_camera()
{
	ioctl(fd, VIDIOC_QUERYCAP, &cap);	//获取摄像头参数

    CLEAR(fmt);
	ioctl(fd, VIDIOC_G_FMT, &fmt);	//获取图像格式

    fmt.fmt.pix.width = CAM_W;
	fmt.fmt.pix.height = CAM_H;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	ioctl(fd, VIDIOC_S_FMT, &fmt);	//设置图像格式

    CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ioctl(fd, VIDIOC_REQBUFS, &req);	//申请缓冲，count是申请的数量

    if (req.count < 1)
	    printf("Insufficient buffer memory\n");

    return;
}

void camera::cam_mmap()
{
	//buffers = (struct buffer*)malloc(sizeof(buffers)*req.count);	//内存中建立对应空间
    buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));

    for(unsigned int i=0; i < req.count; i++) {
        struct v4l2_buffer buf;	//驱动中的一帧
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))	//映射用户空间
            printf("VIDIOC_QUERYBUF error\n");

        buffers[i].length = buf.length;
        printf("buffers[i].length:%x\n", buffers[i].length);

        buffers[i].start = mmap(NULL, 
                    buf.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					fd, buf.m.offset);

        if (MAP_FAILED == buffers[i].start)
            printf("mmap failed\n");

    }

}

void camera::qbuf()
{
    for(unsigned int i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;	//驱动中的一帧
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

    	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))	//申请到的缓冲进入列队
		    printf("VIDIOC_QBUF failed\n");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))	//开始捕捉图像数据
	    printf("VIDIOC_STREAMON failed\n");
}

void camera::yuv422toyuv420(unsigned char* Y, unsigned char* Cb,
                unsigned char* Cr, unsigned char* out, int w, int h)
{
    unsigned char* p = NULL;
    unsigned char* p1 = Y;
    unsigned char* p2 = Cb;
    unsigned char* p3 = Cr;

    memcpy(out, p1, w*h);
    p = out + w*h;

    int x, y;
    for (y = 0; y < h/2; y++) {
        for (x = 0; x < w/2; x++) {
            *p = *p2;
            p++;
            p2++;
        }
        p2 += w/2;
    }

    for (y = 0; y < h/2; y++) {
        for (x = 0; x < w/2; x++) {
            *p = *p3;
            p++;
            p3++;
        }
        p3 += w/2;
    }

}

void camera::flip_color(void* t)
{
    int i = 0;
    int m = 0;
    unsigned char* temp = (unsigned char*) t; 

    unsigned char*  Y = (unsigned char*) malloc(640*480);
    unsigned char* Cb = (unsigned char*) malloc(640*480/2);
    unsigned char* Cr = (unsigned char*) malloc(640*480/2);
    //unsigned char* out = (unsigned char*) malloc((640*480*3)/2);
    unsigned char* out = (unsigned char*) malloc((640*480*2));

    //分离Y
    while (i < 640*480*2) {
        Y[m] = *((unsigned char *)(temp + i));
        i = i + 2;
        m++;
    }

    //分离Cb
    i = 1;
    m = 0;
    while (i < 640*480*2) {
        Cb[m] = *((unsigned char *)(temp + i));
        i = i + 4;
        m++;
    }

    //分离Cr
    i = 3;
    m = 0;
    while (i < 640*480*2) {
        Cr[m] = *((unsigned char *)(temp + i));
        i = i + 4;
        m++;
    }

    memcpy(out, Y, 640*480);
    memcpy((void *)(out+640*480), Cb, 640*480/2);
    memcpy((void *)(out+640*480+640*480/2), Cr, 640*480/2);

    //memcpy((void *)(out+640*480), Cr, 640*480/2);
    //memcpy((void *)(out+640*480+640*480/2), Cb, 640*480/2);

    yuv422toyuv420(Y, Cb, Cr, out, 640, 480);
    msx264_encoder(out);
    
    free(Y);
    free(Cb);
    free(Cr);
    Y = NULL;
    Cb = NULL;
    Cr = NULL;

    //write(file, out, (640*480*3)/2);
    //send_buff((void *)out);
    free(out);
    out = NULL;
}

void* camera::thread_loop(void* args)
{
    ARGS* arg = (ARGS *)args;
    camera* pthis = arg->pThis;
    void* t = arg->temp;
#if 1
    unsigned char* temp = (unsigned char*)malloc(640*480*2);

    pthis->init_server();
    pthis->msx264_encoder_open();
    pthis->msx264_pic_init();

    //pthis->send_buff((void *)t);
    while (1) {
            //printf("send buff\n");
            pthis->m_lock();
            memcpy(temp, t, 640*480*2);
            pthis->m_unlock();
            pthis->flip_color(temp);
            //pthis->send_buff((void*)temp);
    }


    free(temp);
    temp = NULL;
#endif
    return NULL;
}

void camera::read_frame()
{

	struct v4l2_buffer buf;
    int i = 0;
    int m = 0;
    void* temp = NULL;

    CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	ioctl(fd, VIDIOC_DQBUF, &buf);	//出列采集的帧缓冲
    //assert(buf.index < 1);

    //printf("buf.index:%x\n", buf.index);
	temp = buffers[buf.index].start;

    ov_sdl_display(((unsigned char*)temp));

    m_lock();
    //printf("create thread\n");
    memcpy(send_buffer, (const void*)temp, 640*480*2);
    m_unlock();
    //flip_color(temp);
    //update_screen();

	ioctl(fd, VIDIOC_QBUF, &buf);	//再将其入列
    return;
}

int camera::get_fd()
{
    return fd;
}
camera::~camera()
{
    free(buffers); 
    close(fd);
    close(file);
}
