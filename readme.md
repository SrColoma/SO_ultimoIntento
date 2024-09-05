
## Archivos Principales

- **blurrer.c**: Aplica un filtro de desenfoque gaussiano a la imagen.
- **edger.c**: Aplica un filtro de detección de bordes (Sobel) a la imagen.
- **inverter.c**: Invierte los colores de la imagen.
- **bmp.c** y **bmp.h**: Manejo de imágenes BMP y memoria compartida.
- **publisher.c**: Publica una imagen en la memoria compartida.
- **pipeline.c**: Coordina el procesamiento de imágenes a través de diferentes filtros.

## Compilación y Ejecución

### Requisitos

- GCC
- Make

### Compilación

Para compilar el proyecto, ejecuta:

```sh
make
```

para limpiar todo
    
```sh
make clean
```

### uso

```sh
./pipeline <input.bmp> <output.bmp> <num_threads> [recurrente (-r)]
```

### cli

la bandera -r al final es opcional, si se coloca el programa se ejecutara de manera recurrente, y se pedira una imagen de entrada, salida y otro numero de hilos cada vez que se termine de procesar una imagen.