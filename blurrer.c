#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"

typedef struct {
    int startHeight;
    int endHeight;
    int width;
    Pixel *originalPixels;
    BMP_Image *image;
    int kernel[3][3];
    int kernelSum;
} ThreadData;

void* applyGaussianBlur(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    printf("blurrer: aplicando filtro Gaussiano empezando en %i\n",data->startHeight);
    int kernelSize = 3;

    for (int y = data->startHeight; y < data->endHeight; y++) {
        for (int x = 1; x < data->width - 1; x++) {
            int red = 0, green = 0, blue = 0;

            for (int ky = 0; ky < kernelSize; ky++) {
                for (int kx = 0; kx < kernelSize; kx++) {
                    int pixelPos = (y + ky - 1) * data->width + (x + kx - 1);
                    red += data->originalPixels[pixelPos].red * data->kernel[ky][kx];
                    green += data->originalPixels[pixelPos].green * data->kernel[ky][kx];
                    blue += data->originalPixels[pixelPos].blue * data->kernel[ky][kx];
                }
            }

            int pixelPos = y * data->width + x;
            data->image->pixels[pixelPos].red = red / data->kernelSum;
            data->image->pixels[pixelPos].green = green / data->kernelSum;
            data->image->pixels[pixelPos].blue = blue / data->kernelSum;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <altura_inicio> <altura_fin> <num_hilos>\n", argv[0]);
        return 1;
    }

    int startHeight = atoi(argv[1]);
    int endHeight = atoi(argv[2]);
    int numThreads = atoi(argv[3]);


    if (startHeight < 0 || endHeight < 0 || startHeight >= endHeight || numThreads <= 0) {
        fprintf(stderr, "Parámetros inválidos\n");
        return 1;
    }

    printf("blurer: startHeight: %d\n", startHeight);
    printf("blurer: endHeight: %d\n", endHeight);
    printf("blurer: numThreads: %d\n", numThreads);

    printf("blurer: obteniendo imagen de memoria compartida\n");
    key_t key = ftok("shmfile", 65);
    BMP_Image *shmaddr = getSharedMemoryImage(key);
    BMP_Image* image = shmaddr;
    int width = image->header.width_px;
    int height = image->norm_height;

    if (endHeight > height) {
        fprintf(stderr, "Altura fin excede la altura de la imagen\n");
        return 1;
    }

    printf("blurer: preparando hilos\n");

    Pixel* originalPixels = (Pixel*)malloc(width * height * sizeof(Pixel));
    if (originalPixels == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        return 1;
    }

    for (int i = 0; i < width * height; i++) {
        originalPixels[i] = image->pixels[i];
    }

    int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };
    int kernelSum = 16;

    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    int heightPerThread = (endHeight - startHeight) / numThreads;

    for (int i = 0; i < numThreads; i++) {
        threadData[i].startHeight = startHeight + i * heightPerThread;
        threadData[i].endHeight = (i == numThreads - 1) ? endHeight : threadData[i].startHeight + heightPerThread;
        threadData[i].width = width;
        threadData[i].originalPixels = originalPixels;
        threadData[i].image = image;
        threadData[i].kernelSum = kernelSum;
        memcpy(threadData[i].kernel, kernel, sizeof(kernel));

        pthread_create(&threads[i], NULL, applyGaussianBlur, &threadData[i]);
        printf("blurer: hilo %d creado\n", i);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(originalPixels);
    shmdt(shmaddr);
    return 0;
}