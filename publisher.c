#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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
    // liberarMemoriaCompartida(key);
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
}