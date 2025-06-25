PASTINHA_CONJUNTA=../
CC = $(PASTINHA_CONJUNTA)\MINGW\bin\gcc
PATH_ALLEGRO=$(PASTINHA_CONJUNTA)\allegro-5.0.10-mingw-4.7.0
LIBS=-lallegro-5.0.10-monolith-mt

all: tp.exe
	
tp.exe: tp.o
	$(CC) -O2 -o tp.exe tp.o -L $(PATH_ALLEGRO)\lib $(LIBS)


tp.o: tp.c
	$(CC) -O2 -I $(PATH_ALLEGRO)\include -c tp.c 	