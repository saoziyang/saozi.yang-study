#pragma once

#include "def_inc.h"
#include "sdl_opt.h"
#include "timer.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/videodev2.h>

class camera : public sdl_opt, public Timer
{
    private:
        int fd;
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
        camera();
        void setup_camera();
        void cam_mmap();
        int get_fd();
        void qbuf();
        void read_frame();
        ~camera();
};
