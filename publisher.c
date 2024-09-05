#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    printf("Publisher: abriendo archivo\n");
    char *filename = argv[1];
    key_t key = ftok("shmfile", 65);  // Crear una clave única
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    BMP_Image* tempImage = readImage(file);
    if (tempImage == NULL) {
        fclose(file);
        exit(1);
    }
    fclose(file);

    printf("Publisher: recservando memoria compartida\n");
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
    printf("Publisher: pblicando imagen en memoria compartida\n");
    memcpy(shmaddr, tempImage, imageSize);
    freeImage(tempImage);

    printf("Publisher: imagen publicada en memoria compartida\n");
    return 0;
}