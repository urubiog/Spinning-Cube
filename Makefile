# Makefile

CC = gcc              # El compilador a usar
CFLAGS = -lm          # Las banderas de compilación
SRC = cube.c          # Los archivos fuente
OUT = cube            # El nombre del archivo de salida

# La regla por defecto que se ejecutará al llamar `make`
all: $(OUT)

# Cómo generar el archivo ejecutable
$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)

# Regla para limpiar los archivos generados
clean:
	rm -f $(OUT)

