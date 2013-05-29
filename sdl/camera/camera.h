#pragma once

#include "def_inc.h"
#include "sdl_opt.h"
#include "timer.h"
#include "network.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/videodev2.h>

class camera;

struct ARGS{
    camera* pThis;
    void* temp;
};

class camera : public sdl_opt, public Timer, public network
{
    private:
        int fd;
        int file;
        int w;
        int h;
        struct v4l2_capability cap;
        struct v4l2_format fmt;
        struct v4l2_requestbuffers req;
        enum v4l2_buf_type type;

        struct buffer {
            void *start;
            size_t length;
        };
        struct buffer *buffers;
    public:
        void *send_buffer;
        camera();
        static void* thread_loop(void* args);
        void setup_camera();
        void cam_mmap();
        int get_fd();
        void qbuf();
        void read_frame();
        void flip_color(void* t);
        void yuv422toyuv420(unsigned char* Y, unsigned char* Cb,
                unsigned char* Cr, unsigned char* out, int w, int h);

        void encodec_init();
        void encodec();
        ~camera();
};
