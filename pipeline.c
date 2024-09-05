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
    pid_t pid;

    // Ejecutar ./publisher
    pid = fork();
    if (pid < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid == 0) {
        char *args[] = {"./publisher", NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
    } else {
        wait(NULL);  // Esperar a que termine ./publisher
    }

    // Ejecutar ./inverter
    pid = fork();
    if (pid < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid == 0) {
        char *args[] = {"./inverter", NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
    } else {
        wait(NULL);  // Esperar a que termine ./inverter
    }

    // Código restante
    BMP_Image *shmaddr = getSharedMemoryImage(key);
    writeImage("car_processed.bmp", shmaddr);
    shmdt(shmaddr);
    liberarMemoriaCompartida(key);

    return 0;
}