#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bmp.h"

int main() {
    key_t key = ftok("shmfile", 65);  // Crear una clave única
    FILE *file = fopen("car.bmp", "rb");
    if (file == NULL) {
        perror("Error al abrir car.bmp");
        exit(1);
    }

    BMP_Image* tempImage = readImage(file);
    if (tempImage == NULL) {
        fclose(file);
        exit(1);
    }
    fclose(file);

    int imageSize = tempImage->header.size;
    int shmid = shmget(key, imageSize, 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("Error al crear el segmento de memoria compartida");
        freeImage(tempImage);
        exit(1);
    }

    BMP_Image* shmaddr = (BMP_Image*)shmat(shmid, (void*)0, 0);
    if (shmaddr == (BMP_Image*)(-1)) {
        perror("Error al adjuntar el segmento de memoria compartida");
        freeImage(tempImage);
        exit(1);
    }

    memcpy(shmaddr, tempImage, imageSize);
    freeImage(tempImage);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid == 0) {
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
        shmdt(shmaddr);
        exit(0);
    } else {
        wait(NULL);
        file = fopen("modificado.bmp", "wb");
        if (file == NULL) {
            perror("Error al abrir modificado.bmp");
            exit(1);
        }
        writeImage("modificado.bmp", shmaddr);
        fclose(file);
        shmdt(shmaddr);
        shmctl(shmid, IPC_RMID, NULL);
    }
    return 0;
}