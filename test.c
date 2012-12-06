#include <stdio.h>

int main(int argc, char** argv)
{
    short int x = 0x0002;
    if ( ((char *) &x)[3] == 2) {
        printf("big\n");
    } else {
        printf("little\n");
    }
}