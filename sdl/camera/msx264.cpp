#include "msx264.h"

msx264::msx264()
{
    pNals = NULL;
    iNal = 0;
    x264_param_default(&params);
    //x264_param_default_preset(&params,"fast","zerolatency");


#if 0
    params.i_threads = X264_SYNC_LOOKAHEAD_AUTO;

	params.i_width 	= 640;
	params.i_height	= 480;

    params.i_frame_total = 0;

    // Set parameters
	params.i_keyint_max         = 25;
	params.i_keyint_min         = 1;

	params.i_bframe             = 0;
    params.b_open_gop = 0;

	params.i_bframe_pyramid = 0;
	params.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

	params.i_log_level = X264_LOG_DEBUG;
	params.i_frame_reference    = 0;
	//params.rc.i_bitrate         = (int)( ( ((float)d->bitrate)*0.8)/1000.0);

	params.rc.i_bitrate         = 250000;
	params.rc.b_stat_write      = 0;

	//params.i_slice_max_size     = ms_get_payload_max_size()-100; /*arlready */
	params.i_slice_max_size     = 1340; /*arlready */
	params.rc.i_lookahead       = 0;
	params.b_annexb		    = 0;
	params.b_repeat_headers     = 1;

	params.rc.i_qp_min          = 2;
	params.rc.i_qp_max          = 31;
	params.rc.i_qp_step         = 29;

	params.i_fps_num 		= 25;
	params.i_fps_den 		= 1;
#endif
    params.i_threads = X264_SYNC_LOOKAHEAD_AUTO;	//* 取空缓冲区继续使用不死锁的保证.
//* video Properties
	params.i_width = 640;	//* 宽度.
	params.i_height = 480;	//* 高度
	params.i_frame_total = 0;	//* 编码总帧数.不知道用0.
	params.i_keyint_max = 10;
//* bitstream parameters
	params.i_bframe = 5;
	params.b_open_gop = 0;
	params.i_bframe_pyramid = 0;
	params.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

//* 宽高比,有效果,但不是想要的.
//pX264Param->vui.i_sar_width = 1080;
//pX264Param->vui.i_sar_height = 720;

//* Log
	params.i_log_level = X264_LOG_DEBUG;
//* Rate control Parameters
	params.rc.i_bitrate = 1024 * 10;	//* 码率(比特率,单位Kbps)
//* muxing parameters
	params.i_fps_den = 1;	//* 帧率分母
	params.i_fps_num = 25;	//* 帧率分子
	params.i_timebase_den = params.i_fps_num;
	params.i_timebase_num = params.i_fps_den;


	// Set profile level constrains
	x264_param_apply_profile(&params,"baseline");
	params.rc.i_rc_method = X264_RC_ABR;

    file = open("./file.h264", O_RDWR | O_NONBLOCK, 0);
    if (file < 0) {
        printf("open video error\n");
        return ;
    }

}


x264_t* msx264::msx264_encoder_open()
{
    int iResult;
    pX264Handle = x264_encoder_open(&params);
	assert(pX264Handle);

	iResult = x264_encoder_headers(pX264Handle, &pNals, &iNal);
	assert(iResult >= 0);

//* 获取整个流的PPS和SPS,不需要可以不调用.
	iResult = x264_encoder_headers(pX264Handle, &pNals, &iNal);
	assert(iResult >= 0);
//* PPS SPS 总共只有36B,如何解析出来呢?
	for (int i = 0; i < iNal; ++i) {
		switch (pNals[i].i_type) {
		case NAL_SPS:
			break;
		case NAL_PPS:
			break;
		default:
			break;
		}
	}
}

void msx264::msx264_pic_init()
{
    pPicIn = new x264_picture_t;
    pPicOut = new x264_picture_t;

	x264_picture_init(pPicOut);

    x264_picture_alloc(pPicIn, X264_CSP_I420, params.i_width,
			   params.i_height);

	pPicIn->img.i_csp = X264_CSP_I420;
	pPicIn->img.i_plane = 3;
}

//x264_t* msx264::msx264_encoder(void* data)
int msx264::msx264_encoder(void* data)
{
    int ret = 0;

	memcpy(pPicIn->img.plane[0], data, 640*480);
	memcpy(pPicIn->img.plane[1], data+640*480, 640*480 / 4);
	memcpy(pPicIn->img.plane[2], data+640*480+640*480/4, 640*480 / 4);

	//encode(pX264Handle, pPicIn, pPicOut);
    ret = x264_encoder_encode(pX264Handle, &pNals, &iNal, pPicIn, pPicOut);
    if (ret == 0) {
        printf("succes\n");
    } else if (ret < 0) {
        printf("encode error\n");
    } else if (ret > 0) {
        printf("get encode data\n");
    }

    for (int i = 0; i < iNal; ++i) {
		write(file, pNals[i].p_payload, pNals[i].i_payload);
	}

	int iFrames = x264_encoder_delayed_frames(pX264Handle);

    return iFrames;
}

msx264::~msx264()
{
//* 清除图像区域
	x264_picture_clean(pPicIn);
	x264_picture_clean(pPicOut);
//* 关闭编码器句柄
	x264_encoder_close(pX264Handle);
	pX264Handle = NULL;

	delete pPicIn;
	pPicIn = NULL;

	delete pPicOut;
	pPicOut = NULL;

    close(file);
	//delete params;
	//params = NULL;
}
