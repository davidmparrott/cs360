CC=gcc
COPTS=-std=c99 -Wall -pedantic

all: daytime.c dayserve.c
	$(CC) $(COPTS) daytime.c -o daytime
	$(CC) $(COPTS) dayserve.c -o dayserve

time: daytime.c
	$(CC) $(COPTS) daytime.c -o daytime

serve: dayserve.c
	$(CC) $(COPTS) dayserve.c -o dayserve
