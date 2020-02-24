CC=gcc
CFLAGS=-pthread -lm
EXTRA_CFLAGS=-Wall -Wextra

main: main.c
	$(CC) main.c -o main $(CFLAGS) $(EXTRA_CFLAGS)

clean :
	rm main
