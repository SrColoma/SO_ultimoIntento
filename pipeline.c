#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bmp.h"

void process_image(char *inputfile, char *outputfile, char *num_threads) {
    key_t key = ftok("shmfile", 65);  // Crear una clave única
    pid_t pid_publisher, pid_inverter, pid_blurrer;

    // Ejecutar ./publisher
    printf("ejecutanod publisher\n");
    pid_publisher = fork();
    if (pid_publisher < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid_publisher == 0) {
        char *args[] = {"./publisher", inputfile, NULL};
        execvp(args[0], args);
        perror("Error en execvp");
        exit(1);
    } else {
        wait(NULL);  // Esperar a que termine ./publisher
    }

    BMP_Image *shmaddr = getSharedMemoryImage(key);
    int height = shmaddr->norm_height;
    int halfheight = shmaddr->norm_height / 2;
    char height_str[10]; // Allocate enough space for the height string
    char halfheight_str[10]; // Allocate enough space for the height string
    sprintf(height_str, "%d", height); // Convert height to string
    sprintf(halfheight_str, "%d", halfheight); // Convert height to string

    // Ejecutar ./blurrer
    printf("ejecutando blurrer\n");
    pid_inverter = fork();
    if (pid_inverter < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid_inverter == 0) {
        char *args[] = {"./blurrer", halfheight_str, height_str,num_threads, NULL};
        execvp(args[0], args);
        perror("Error en execvp up");
        exit(1);
    }

    // Ejecutar ./edger
    printf("ejecutando edger\n");
    pid_blurrer = fork();
    if (pid_blurrer < 0) {
        perror("Error en fork");
        exit(1);
    } else if (pid_blurrer == 0) {
        char *args[] = {"./edger", "1", halfheight_str,num_threads, NULL};
        execvp(args[0], args);
        perror("Error en execvp down");
        exit(1);
    }

    // Esperar a que terminen ./inverter y ./blurrer
    waitpid(pid_inverter, NULL, 0);
    waitpid(pid_blurrer, NULL, 0);


    // Código restante
    printf("escribiendo imagen en %s \n", outputfile);
    writeImage(outputfile, shmaddr);
    shmdt(shmaddr);
    liberarMemoriaCompartida(key);
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile> <num_threads> [recurrent < -r >]\n", argv[0]);
        exit(1);
    }

    char *inputfile = argv[1];
    char *outputfile = argv[2];
    char *num_threads = argv[3];
    int recurrent_mode = (argc == 5 && strcmp(argv[4], "-r") == 0);

    do {
        printf("input file: %s\n", inputfile);
        printf("output file: %s\n", outputfile);
        printf("num_threads: %s\n", num_threads);
        process_image(inputfile, outputfile, num_threads);

        if (recurrent_mode) {
            printf("Enter input file: ");
            scanf("%s", inputfile);
            printf("Enter output file: ");
            scanf("%s", outputfile);
            printf("Enter number of threads: ");
            scanf("%s", num_threads);
        }
    } while (recurrent_mode);

    return 0;
}