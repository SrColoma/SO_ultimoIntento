# Nombre del compilador
CC = gcc
# Opciones del compilador
CFLAGS = -Wall -Wextra -std=c11
# Nombre del archivo ejecutable
TARGET = pipeline
INVERTER = inverter
# Archivos fuente
SRCS = pipeline.c bmp.c
INVERTER_SRCS = inverter.c bmp.c
# Archivos objeto
OBJS = $(SRCS:.c=.o)
INVERTER_OBJS = $(INVERTER_SRCS:.c=.o)
# Regla por defecto
all: $(TARGET) $(INVERTER)
# Regla para compilar el archivo ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
$(INVERTER): $(INVERTER_OBJS)
	$(CC) $(CFLAGS) -o $(INVERTER) $(INVERTER_OBJS)
# Regla para compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
# Regla para limpiar archivos generados
clean:
	rm -f $(OBJS) $(INVERTER_OBJS) $(TARGET) $(INVERTER)
# Regla para ejecutar el programa
run: $(TARGET)
	./$(TARGET)
.PHONY: all clean run