#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bmp.h"
#include <math.h>

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

    // Aplicar el filtro de Sobel solo en el rango especificado
    for (int i = startHeight; i < endHeight; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sumX = 0, sumY = 0;

            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int index = (i + k) * width + (j + l);
                    int gray = (image->pixels[index].red + image->pixels[index].green + image->pixels[index].blue) / 3;
                    sumX += gray * Gx[k + 1][l + 1];
                    sumY += gray * Gy[k + 1][l + 1];
                }
            }

            int index = i * width + j;
            int magnitude = (int)fmin(sqrt(sumX * sumX + sumY * sumY), 255);

            image->pixels[index].red = magnitude;
            image->pixels[index].green = magnitude;
            image->pixels[index].blue = magnitude;
        }
    }

    shmdt(shmaddr);  // Desadjuntar la memoria compartida
    return 0;
}