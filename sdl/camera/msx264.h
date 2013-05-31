#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

extern "C" {
#include <x264.h>
}

class msx264 {
    private:
        int file;
        x264_param_t params;
        //x264_picture_t pic;
	    x264_picture_t *pPicIn;
	    x264_picture_t *pPicOut;
        x264_t* pX264Handle;
        x264_nal_t *pNals;
        int iNal;
    public:
        msx264();
        ~msx264();
        void msx264_pic_init();
        x264_t* msx264_encoder_open();
        //int msx264_encoder(void* Y, void* Cb, void* Cr);
        int msx264_encoder(void* data);
};
