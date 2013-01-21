/* Simple program:  Fill a colormap with gray and stripe it down the screen */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>



#include <SDL/SDL.h>

#ifdef TEST_VGA16 /* Define this if you want to test VGA 16-color video modes */
#define NUM_COLORS  16
#else
#define NUM_COLORS	256
#endif

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define  RGB(v) ({  \
            int value = (v); \
            (value > 0) * value | (255 * (value > 255));\
        })


struct buffer {
	void *start;
	size_t length;
};

struct buffer *buffers = NULL;

static int fd = -1; 
SDL_Surface *screen;
SDL_Surface *rgb_screen;
SDL_Overlay *overlay;
unsigned char *rgb_pixel ;
unsigned char *rgb_framebuffer ;



SDL_Surface *CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags)
{
	SDL_Surface *screen;
	int i;
	SDL_Color palette[NUM_COLORS];

	/* Set the video mode */
	screen = SDL_SetVideoMode(w, h, bpp, flags);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set display mode: %s\n",
							SDL_GetError());
		return(NULL);
	}
	fprintf(stderr, "Screen is in %s mode\n",
		(screen->flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

	if (bpp==8) {
		/* Set a gray colormap, reverse order from white to black */
		for ( i=0; i<NUM_COLORS; ++i ) {
			palette[i].r = (NUM_COLORS-1)-i * (256 / NUM_COLORS);
			palette[i].g = (NUM_COLORS-1)-i * (256 / NUM_COLORS);
			palette[i].b = (NUM_COLORS-1)-i * (256 / NUM_COLORS);
		}
		SDL_SetColors(screen, palette, 0, NUM_COLORS);
	}

	return(screen);
}


/* Draw a randomly sized and colored box centered about (X,Y) */
void DrawBox(SDL_Surface *screen, int X, int Y, int width, int height)
{
	static unsigned int seeded = 0;
	SDL_Rect area;
	Uint32 color;
        Uint32 randc;

	/* Seed the random number generator */
	if ( seeded == 0 ) {
		srand(time(NULL));
		seeded = 1;
	}

	/* Get the bounds of the rectangle */
	area.w = (rand()%width);
	area.h = (rand()%height);
	area.x = X-(area.w/2);
	area.y = Y-(area.h/2);
    randc = (rand()%NUM_COLORS);

    if (screen->format->BytesPerPixel==1)
    {
        color = randc;
    }
    else
    {
        color = SDL_MapRGB(screen->format, randc, randc, randc);
    }

	/* Do it! */
	SDL_FillRect(screen, &area, color);
	if ( screen->flags & SDL_DOUBLEBUF ) {
		SDL_Flip(screen);
	} else {
		SDL_UpdateRects(screen, 1, &area);
	}
}

#if 0
void Dispaly_Camera(SDL_Surface *screen)
{
	int i, j, k;
	Uint8  *buffer;
	Uint16 *buffer16;
    Uint16 color;
    Uint8  gradient;

	/* Set the surface pixels and refresh! */
	/* Use two loops in case the surface is double-buffered (both sides) */

	for ( j=0; j<2; ++j ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			fprintf(stderr, "Couldn't lock display surface: %s\n",
								SDL_GetError());
			return;
		}
		buffer = (Uint8 *)screen->pixels;

		if (screen->format->BytesPerPixel!=2) {
			for ( i=0; i<screen->h; ++i ) {
				memset(buffer,(i*(NUM_COLORS-1))/screen->h, screen->w * screen->format->BytesPerPixel);
				buffer += screen->pitch;
			}
		} else {
			for ( i=0; i<screen->h; ++i ) {
				//gradient=((i*(NUM_COLORS-1))/screen->h);
				//gradient=;
                color = SDL_MapRGB(screen->format, gradient, gradient, gradient);
                buffer16=(Uint16*)buffer;
                for (k=0; k<screen->w; k++) {
                   *(buffer16+k)=color;
                }
				buffer += screen->pitch;
			}
        }

		SDL_UnlockSurface(screen);
		if ( screen->flags & SDL_DOUBLEBUF ) {
			SDL_Flip(screen);
		} else {
			SDL_UpdateRect(screen, 0, 0, 0, 0);
                        break;
		}
	}
}

#endif

void DrawBackground(SDL_Surface *screen)
{
	int i, j, k;
	Uint8  *buffer;
	Uint16 *buffer16;
    Uint16 color;
    Uint8  gradient;

	/* Set the surface pixels and refresh! */
	/* Use two loops in case the surface is double-buffered (both sides) */

	for ( j=0; j<2; ++j ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			fprintf(stderr, "Couldn't lock display surface: %s\n",
								SDL_GetError());
			return;
		}
		buffer = (Uint8 *)screen->pixels;

		if (screen->format->BytesPerPixel!=2) {
			for ( i=0; i<screen->h; ++i ) {
				memset(buffer,(i*(NUM_COLORS-1))/screen->h, screen->w * screen->format->BytesPerPixel);
				buffer += screen->pitch;
			}
		} else {
			for ( i=0; i<screen->h; ++i ) {
				gradient=((i*(NUM_COLORS-1))/screen->h);
                color = SDL_MapRGB(screen->format, gradient, gradient, gradient);
                buffer16=(Uint16*)buffer;
                for (k=0; k<screen->w; k++) {
                   *(buffer16+k)=color;
                }
				buffer += screen->pitch;
			}
        }

		SDL_UnlockSurface(screen);
		if ( screen->flags & SDL_DOUBLEBUF ) {
			SDL_Flip(screen);
		} else {
			SDL_UpdateRect(screen, 0, 0, 0, 0);
                        break;
		}
	}
}


static int clamp(double x)
{

  int r = x;

  if (r < 0)

    return 0;

  else if (r > 255)

    return 255;

  else

    return r;

}



static void yuv2rgb(unsigned char Y, unsigned char Cb, unsigned char Cr,
                int *ER, int *EG, int *EB)

{

    //printf("%s\n", __func__);
    double y1, pb, pr, r, g, b;
    y1 = (255 / 219.0) * (Y - 16);
    pb = (255 / 224.0) * (Cb - 128);
    pr = (255 / 224.0) * (Cr - 128);
    r = 1.0 * y1 + 0 * pb + 1.402 * pr;
    g = 1.0 * y1 - 0.344 * pb - 0.714 * pr;
    b = 1.0 * y1 + 1.722 * pb + 0 * pr;

  /* 用GDB调试了这么久终于将BUG找出来了:), 是v4l2的文档有问题 */
  /* 不应该为clamp(r * 255) */

    *ER = clamp(r);
    *EG = clamp(g);
    *EB = clamp(b);

}

static void update_rgb_pixels(const void *start)
{
  unsigned char *data = (unsigned char *)start;
  //rgb_pixel = malloc(640*480*4);
  int width = 640;
  int height = 480;

  unsigned char Y, Cr, Cb;
  int r, g, b;
  int x, y;
  int p1, p2, p3, p4;

    //printf("%s\n", __func__);
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            p1 = y * width * 2 + x * 2;
            Y = data[p1];
            if (x % 2 == 0) {
                p2 = y * width * 2 + (x * 2 + 1);
                p3 = y * width * 2 + (x * 2 + 3);
            } else {
                p2 = y * width * 2 + (x * 2 - 1);
                p3 = y * width * 2 + (x * 2 + 1);
            }

            Cb = data[p2];
            Cr = data[p3];
            yuv2rgb(Y, Cb, Cr, &r, &g, &b);
            p4 = y * width * 4 + x * 4;
            rgb_pixel[p4 + 0] = r;
            rgb_pixel[p4 + 1] = g;
            rgb_pixel[p4 + 2] = b;
            rgb_pixel[p4 + 3] = 255;

        }
    }
    //printf("%s\n", __func__);
}

static void update_rgb_surface()
{
    //update_rgb_pixels(buffers->start);
    update_rgb_pixels(rgb_framebuffer);
    SDL_BlitSurface(rgb_screen, NULL, screen, NULL);
    //printf("%s\n", __func__);
    SDL_Flip(screen);

}

static void vo_sdl_display(void *p)
{
    SDL_Rect rect;
    //AVPicture p;
    SDL_LockYUVOverlay(overlay);
    //p.data[0] = overlay->pixels[0];
    //p.data[1] = overlay->pixels[2];
    //p.data[2] = overlay->pixels[1];
    //p.linesize[0] = 
    //p.linesize[1] = 
    //p.linesize[2] = 
    //vo_sdl_sws(&p, pict); /* only do memcpy */
    SDL_UnlockYUVOverlay(overlay);
    overlay->pixels[0]=p;
    overlay->pixels[1]=p+(640*480)/4;
    overlay->pixels[2]=p+(640*480)/4;
    rect.x = 0;
    rect.y = 0;
    rect.w = 640;
    rect.h = 480;
    SDL_DisplayYUVOverlay(overlay,&rect);
}

void yuv422_888(__u8 *to, __u8 *from, int width, int height)
{
	__u32 length;
	__u32 i, j;
	int y, u, v;
//	int y, u, v, r, g, b, u1, uv1, v1, y1;
	//length = width * height * 2;
	length = width * height * 3;
	i = 0;
	j = 0;
	while(i < length) {
		y = from[i];
		u = from[i+1] - 128;
		v = from[i+3] - 128;

		to[j++] = RGB(y + v + (v >> 2) + (v >> 3) + (v >> 5));
		to[j++] = RGB(y - ((u >> 2) + (u >> 4) + (u >> 5)) - ((v >> 1) + (v >> 3) + (v >> 4) + (v >> 5)));
		to[j++] = RGB(y + u + (u >> 1) + (u >> 2) + (u >> 6));
		y = from[i+2];
		to[j++] = RGB(y + v + (v >> 2) + (v >> 3) + (v >> 5));
		to[j++] = RGB(y - ((u >> 2) + (u >> 4) + (u >> 5)) - ((v >> 1) + (v >> 3) + (v >> 4) + (v >> 5)));
		to[j++] = RGB(y + u + (u >> 1) + (u >> 2) + (u >> 6));
		i += 4;
	}
	j = (width / 2 + height * width / 2) * 3;
	to[j++] = 255;to[j++] = 0;to[j++] = 0;
}


static int read_frame()
{
	struct v4l2_buffer buf;
    unsigned char* temp_buf = NULL;
	temp_buf = buffers->start;


	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	ioctl(fd, VIDIOC_DQBUF, &buf);	//出列采集的帧缓冲
    assert(buf.index < 1);

    memcpy((void *)rgb_framebuffer, (void *)temp_buf, (size_t)buffers->length);
    update_rgb_surface();
    //vo_sdl_display((buffers->start));
#if 0
    yuv422_888(rgb_pixel, rgb_framebuffer, 640, 480);
    SDL_BlitSurface(rgb_screen, NULL, screen, NULL);
    SDL_Flip(screen);
#endif

	ioctl(fd, VIDIOC_QBUF, &buf);	//再将其入列
	return 1;
}


int main(int argc, char** argv)
{
	Uint32 videoflags;
	int    done;
	SDL_Event event;
	int width, height, bpp;


    struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	enum v4l2_buf_type type;

    fd = open("/dev/video0", O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        printf("open video error\n");
        exit(-1);
    }

	ioctl(fd, VIDIOC_QUERYCAP, &cap);	//获取摄像头参数
    printf("cap.driver:\t%s\n", cap.driver);
    printf("cap.card:\t%s\n", cap.card);
    //printf("cap.businfo:\t%s\n", cap.businfo);

	CLEAR(fmt);
	ioctl(fd, VIDIOC_G_FMT, &fmt);	//设置图像格式
    //printf("\n\n\n");
    //printf("width:\t%x\n", fmt.fmt.pix.width);
    //printf("height:\t%x\n", fmt.fmt.pix.height);
    fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
    //fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    //fmt.fmt.pix.field = V4L2_FIELD_ANY;
	ioctl(fd, VIDIOC_S_FMT, &fmt);	//设置图像格式

	CLEAR(req);
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	ioctl(fd, VIDIOC_REQBUFS, &req);	//申请缓冲，count是申请的数量
    //printf("req.count:%\t%x\n", req.count);
    if (req.count < 1)
	    printf("Insufficient buffer memory\n");

	buffers = malloc(sizeof(buffers));	//内存中建立对应空间

    struct v4l2_buffer buf;	//驱动中的一帧
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))	//映射用户空间
		printf("VIDIOC_QUERYBUF error\n");

    printf("buf.length:%x\n", buf.length);
    buffers->length = buf.length;
    buffers->start = mmap(NULL, 
                    buf.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					fd, buf.m.offset);


    //printf("start:%x\n", buffers->start);
	if (MAP_FAILED == buffers->start)
		printf("mmap failed\n");

	CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))	//申请到的缓冲进入列队
		printf("VIDIOC_QBUF failed\n");

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))	//开始捕捉图像数据
		printf("VIDIOC_STREAMON failed\n");


	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}

	width = 640;
	height = 480;
	bpp = 32;
	//bpp = 24;
	videoflags = SDL_SWSURFACE | SDL_DOUBLEBUF;

	/* Set a video mode */
	screen = CreateScreen(width, height, bpp, videoflags);
	if ( screen == NULL ) {
		exit(2);
	}

    rgb_framebuffer = (unsigned char*)malloc(640*480*4);
    memset(rgb_framebuffer, 0, 640*480*4);
    rgb_pixel = (unsigned char*)malloc(640*480*4);
    memset(rgb_pixel, 0, 640*480*4);

    rgb_screen = SDL_CreateRGBSurfaceFrom(rgb_pixel, 640, 480, 32, 640*4, 0x000000ff,
                                        0x0000ff00, 0x00ff0000, 0xFF000000);
    if (rgb_screen == NULL) {
        printf("rgb screen is error\n");
        exit(2);
    }
        
    ///* Set a overlay mode */
	//overlay = SDL_CreateYUVOverlay(640, 480, SDL_YV12_OVERLAY, screen);
	//if ( overlay == NULL ) {
	//	exit(2);
	//}
    //printf("------------------\n");
    //DrawBackground(screen);
	
	/* Wait for a keystroke */
	done = 0;
	while ( !done && SDL_WaitEvent(&event) ) {
	
        fd_set fds;
		struct timeval tv;

		FD_ZERO(&fds);	//将指定的文件描述符集清空
		FD_SET(fd, &fds);	//在文件描述符集合中增加一个新的文件描述符

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		switch (select(fd + 1, &fds, NULL, NULL, &tv)) {//判断是否可读（即摄像头是否准备好），tv是定时
            case -1:
				continue;
			    printf("select err/n");
            case 0:
                fprintf(stderr, "select timeout\n");
                exit(EXIT_FAILURE);
            default:
		        read_frame();
                break;
        }

		switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
				//DrawBox(screen, event.button.x, event.button.y, width, height);
				break;
			case SDL_KEYDOWN:
				break;
				/* Any other key quits the application... */
			case SDL_QUIT:
				done = 1;
				break;
			case SDL_VIDEOEXPOSE:
				break;
			case SDL_VIDEORESIZE:
#if 0
					screen = CreateScreen(
						event.resize.w, event.resize.h,
						screen->format->BitsPerPixel,
								videoflags);
					if ( screen == NULL ) {
						fprintf(stderr,
					"Couldn't resize video mode\n");
						done = 1;
					}
					DrawBackground(screen);
#endif
				break;
			default:
				break;
		}
	}
	SDL_Quit();
    return 0;
}
