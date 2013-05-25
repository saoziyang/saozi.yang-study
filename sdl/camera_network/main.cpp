#include <iostream>
#include <string>
#include "def_inc.h"
#include "camera.h"

void clean_up()
{
    printf("%s\n", __func__);
    SDL_Quit();
    return;
}

int main()
{
    static bool quite = false;
    class camera cam;

    cam.setup_camera();
    cam.cam_mmap();
    cam.qbuf();
    cam.init_client();

    while (quite != true) {
        cam.start();
#if 0
        fd_set fds;
		struct timeval tv;

		FD_ZERO(&fds);	//将指定的文件描述符集清空
		FD_SET(cam.get_network_fd(), &fds);	//在文件描述符集合中增加一个新的文件描述符

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		//tv.tv_usec = 100*1000;

        //判断是否可读（即摄像头是否准备好），tv是定时
		switch (select(cam.get_network_fd() + 1, &fds, NULL, NULL, &tv)) {
            case -1:
				continue;
			    printf("select err/n");
            case 0:
                fprintf(stderr, "select timeout\n");
                break;
            default:
                printf("read frame");
		        cam.read_frame();
                break;
        }
#endif
		cam.read_frame();
        printf("read end\n");
        while(SDL_PollEvent(cam.get_event())) {
            if (cam.get_event()->type == SDL_QUIT) {
                quite = true;
            }
        }

        cam.update_screen();
        if (cam.get_ticks() < 1000 / 20) {
            SDL_Delay((1000/20) - cam.get_ticks());
        }
    }

    clean_up();
    return 0;
}
