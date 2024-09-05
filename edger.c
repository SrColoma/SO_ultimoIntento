#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"


int main() {
    key_t key = ftok("shmfile", 65);  // Crear una clave única
    
    BMP_Image *shmaddr = getSharedMemoryImage(key);

    BMP_Image* image = shmaddr;
    int width = image->header.width_px;
    int height = image->norm_height;

    // Crear una copia de la imagen para almacenar los resultados
    size_t imageSize = sizeof(BMP_Image) + width * height * sizeof(Pixel);
    BMP_Image* resultImage = (BMP_Image*)malloc(imageSize);
    resultImage->header = image->header;
    resultImage->norm_height = image->norm_height;
    resultImage->bytes_per_pixel = image->bytes_per_pixel;

    // Definir los kernels de Sobel
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // Aplicar el filtro de Sobel
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sumX_red = 0, sumY_red = 0;
            int sumX_green = 0, sumY_green = 0;
            int sumX_blue = 0, sumY_blue = 0;

            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int index = (i + k) * width + (j + l);
                    sumX_red += image->pixels[index].red * Gx[k + 1][l + 1];
                    sumY_red += image->pixels[index].red * Gy[k + 1][l + 1];
                    sumX_green += image->pixels[index].green * Gx[k + 1][l + 1];
                    sumY_green += image->pixels[index].green * Gy[k + 1][l + 1];
                    sumX_blue += image->pixels[index].blue * Gx[k + 1][l + 1];
                    sumY_blue += image->pixels[index].blue * Gy[k + 1][l + 1];
                }
            }

            int index = i * width + j;
            resultImage->pixels[index].red = (uint8_t)fmin(sqrt(sumX_red * sumX_red + sumY_red * sumY_red), 255);
            resultImage->pixels[index].green = (uint8_t)fmin(sqrt(sumX_green * sumX_green + sumY_green * sumY_green), 255);
            resultImage->pixels[index].blue = (uint8_t)fmin(sqrt(sumX_blue * sumX_blue + sumY_blue * sumY_blue), 255);
        }
    }

    // Copiar los resultados de vuelta a la imagen original
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = i * width + j;
            image->pixels[index] = resultImage->pixels[index];
        }
    }

    free(resultImage);

    shmdt(shmaddr);  // Desadjuntar la memoria compartida
    return 0;
}