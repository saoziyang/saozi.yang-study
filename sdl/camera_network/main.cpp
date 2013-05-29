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
    pthread_t id;

    ARGS* args = new ARGS();

    args->pThis = &cam;
    args->temp = cam.recv_buffer;

    //cam.setup_camera();
    //cam.cam_mmap();
    //cam.qbuf();
    pthread_create(&id, NULL, &camera::thread_loop, (void *)args);

    while (quite != true) {
        cam.start();

		cam.read_frame();

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
