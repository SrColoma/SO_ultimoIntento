# Nombre del compilador
CC = gcc

# Opciones del compilador
CFLAGS = -Wall -Wextra -std=c11

# Nombre del archivo ejecutable
TARGET = pipeline

# Archivos fuente
SRCS = pipeline.c bmp.c

# Archivos objeto
OBJS = $(SRCS:.c=.o)

# Regla por defecto
all: $(TARGET)

# Regla para compilar el archivo ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Regla para compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Regla para ejecutar el programa
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run