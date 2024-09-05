#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <altura_inicio> <altura_fin>\n", argv[0]);
        return 1;
    }

    int startHeight = atoi(argv[1]);
    int endHeight = atoi(argv[2]);

    if (startHeight < 0 || endHeight < 0 || startHeight >= endHeight) {
        fprintf(stderr, "Alturas inválidas\n");
        return 1;
    }

    key_t key = ftok("shmfile", 65);  // Crear una clave única
    
    BMP_Image *shmaddr = getSharedMemoryImage(key);

    BMP_Image* image = shmaddr;
    int width = image->header.width_px;
    int height = image->norm_height;

    if (endHeight > height) {
        fprintf(stderr, "Altura fin excede la altura de la imagen\n");
        return 1;
    }

    Pixel* originalPixels = (Pixel*)malloc(width * height * sizeof(Pixel));
    if (originalPixels == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        return 1;
    }

    // Copiar los píxeles originales
    for (int i = 0; i < width * height; i++) {
        originalPixels[i] = image->pixels[i];
    }

    // Kernel de desenfoque gaussiano 3x3
    int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };
    int kernelSize = 3;
    int kernelSum = 16;  // Suma de todos los valores en el kernel

    // Aplicar el kernel de desenfoque gaussiano
    for (int y = startHeight; y < endHeight; y++) {
        for (int x = 1; x < width - 1; x++) {
            int red = 0, green = 0, blue = 0;

            // Aplicar el kernel
            for (int ky = 0; ky < kernelSize; ky++) {
                for (int kx = 0; kx < kernelSize; kx++) {
                    int pixelPos = (y + ky - 1) * width + (x + kx - 1);
                    red += originalPixels[pixelPos].red * kernel[ky][kx];
                    green += originalPixels[pixelPos].green * kernel[ky][kx];
                    blue += originalPixels[pixelPos].blue * kernel[ky][kx];
                }
            }

            // Promediar los valores
            int pixelPos = y * width + x;
            image->pixels[pixelPos].red = red / kernelSum;
            image->pixels[pixelPos].green = green / kernelSum;
            image->pixels[pixelPos].blue = blue / kernelSum;
        }
    }

    free(originalPixels);

    shmdt(shmaddr);  // Desadjuntar la memoria compartida
    return 0;
}