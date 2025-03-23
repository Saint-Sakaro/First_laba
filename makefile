TARGET = main.exe
CC = gcc

$(TARGET): fullclean types/int.o types/float.o types/double.o types/complex.o src/main.o src/matrix.o src/error.o

	$(CC) types/int.o types/float.o types/double.o types/complex.o src/main.o src/matrix.o src/error.o -o $(TARGET)

main.o:
	$(CC) -c src/main.c -o src/main.o

int.o:
	$(CC) -c types/int.c -o types/int.o 

float.o:
	$(CC) -c types/float.c -o types/float.o

double.o:
	$(CC) -c types/double.c -o types/double.o

complex.o:
	$(CC) -c types/complex.c -o types/complex.o

matrix.o:
	$(CC) -c src/matrix.c -o src/matrix.o

error.o:
	$(CC) -c src/error.c -o src/error.o

.PHONY:
	clean

clean:
	rm -f *.o
	rm -f types/*.o
	rm -f src/*.o

fullclean: clean
	rm -f *.exe