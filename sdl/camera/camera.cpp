#include "camera.h"
#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define CAM_W   640
#define CAM_H   480

camera::camera()
{
    printf("%s\n", __func__);
    fd = open("/dev/video0", O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        printf("open video error\n");
        exit(-1);
    }
    buffers = NULL;
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


void camera::read_frame()
{
	struct v4l2_buffer buf;
    void* temp = NULL;

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	ioctl(fd, VIDIOC_DQBUF, &buf);	//出列采集的帧缓冲
    //assert(buf.index < 1);

    //printf("buf.index:%x\n", buf.index);
	temp = buffers[buf.index].start;

    ov_sdl_display(((unsigned char*)temp));
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
}
