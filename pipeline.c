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
        //ejecuta .inverter
        char *args[] = {"./inverter", NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
        
    } else {
        wait(NULL);
        
        writeImage("modificado.bmp", shmaddr);
        shmdt(shmaddr);
        liberarMemoriaCompartida(key);
    }
    return 0;
}