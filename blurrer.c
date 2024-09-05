#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"

#define BLUR_SIZE 3

int main() {
    key_t key = ftok("shmfile", 65);  // Crear una clave única
    
    BMP_Image *shmaddr = getSharedMemoryImage(key);

    BMP_Image* image = shmaddr;
    int width = image->header.width_px;
    int height = image->norm_height;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int red = 0, green = 0, blue = 0;
            int count = 0;

            for (int ki = -BLUR_SIZE; ki <= BLUR_SIZE; ki++) {
                for (int kj = -BLUR_SIZE; kj <= BLUR_SIZE; kj++) {
                    int ni = i + ki;
                    int nj = j + kj;

                    if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                        int nindex = ni * width + nj;
                        red += image->pixels[nindex].red;
                        green += image->pixels[nindex].green;
                        blue += image->pixels[nindex].blue;
                        count++;
                    }
                }
            }

            int index = i * width + j;
            image->pixels[index].red = red / count;
            image->pixels[index].green = green / count;
            image->pixels[index].blue = blue / count;
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = i * width + j;
            image->pixels[index] = image->pixels[index];
        }
    }


    shmdt(shmaddr);  // Desadjuntar la memoria compartida
    return 0;
}