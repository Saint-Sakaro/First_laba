TARGET = main.exe
CC = gcc

$(TARGET): fullclean int.o float.o double.o complex.o main.o matrix.o error.o

	$(CC) int.o float.o double.o complex.o main.o matrix.o error.o -o $(TARGET)

main.o:
	$(CC) -c main.c -o main.o

int.o:
	$(CC) -c int.c -o int.o 

float.o:
	$(CC) -c float.c -o float.o

double.o:
	$(CC) -c double.c -o double.o

complex.o:
	$(CC) -c complex.c -o complex.o

matrix.o:
	$(CC) -c matrix.c -o matrix.o

error.o:
	$(CC) -c error.c -o error.o

.PHONY:
	clean

clean:
	rm -f *.o

fullclean: clean
	rm -f *.exe