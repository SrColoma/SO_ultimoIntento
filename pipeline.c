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
    pid_t pid_publisher, pid_inverter, pid_blurrer;

    // Ejecutar ./publisher
    pid_publisher = fork();
    if (pid_publisher < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid_publisher == 0) {
        char *args[] = {"./publisher", NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
    } else {
        wait(NULL);  // Esperar a que termine ./publisher
    }

    // Ejecutar ./inverter
    pid_inverter = fork();
    if (pid_inverter < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid_inverter == 0) {
        char *args[] = {"./edger", NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
    }

    // Ejecutar ./blurrer
    pid_blurrer = fork();
    if (pid_blurrer < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid_blurrer == 0) {
        char *args[] = {"./blurrer", NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
    }

    // Esperar a que terminen ./inverter y ./blurrer
    waitpid(pid_inverter, NULL, 0);
    waitpid(pid_blurrer, NULL, 0);

    // Código restante
    BMP_Image *shmaddr = getSharedMemoryImage(key);
    writeImage("car_processed.bmp", shmaddr);
    shmdt(shmaddr);
    liberarMemoriaCompartida(key);

    return 0;
}