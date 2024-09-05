#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"

#define SHM_SIZE sizeof(BMP_Image)  // Tamaño del segmento de memoria compartida

int main() {
    key_t key = ftok("shmfile", 65);  // Crear una clave única
    int shmid = shmget(key, SHM_SIZE, 0666);  // Obtener el segmento de memoria compartida
    if (shmid < 0) {
        perror("Error en shmget");
        exit(1);
    }
    BMP_Image *shmaddr = (BMP_Image *)shmat(shmid, (void *)0, 0);  // Adjuntar el segmento de memoria compartida
    if (shmaddr == (BMP_Image *)-1) {
        perror("Error en shmat");
        exit(1);
    }

    BMP_Image* image = shmaddr;
        int width = image->header.width_px;
        int height = image->norm_height;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int index = i * width + j;
                image->pixels[index].red = 255 - image->pixels[index].red;
                image->pixels[index].green = 255 - image->pixels[index].green;
                image->pixels[index].blue = 255 - image->pixels[index].blue;
            }
        }

    shmdt(shmaddr);  // Desadjuntar la memoria compartida
    return 0;
}