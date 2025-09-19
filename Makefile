# ----- Ayarlar -----
CC=C:/msys64/mingw64/bin/gcc.exe
CFLAGS=-O2 -Wall -IC:/msys64/mingw64/include -I./kissfft
LDFLAGS=-LC:/msys64/mingw64/lib
LIBS=-lraylib -lopengl32 -lgdi32 -lwinmm -luser32 -lshell32 -lm


# ----- Hedefler -----
cember:
	$(CC) -o cember cember.c $(CFLAGS) $(LDFLAGS) $(LIBS)

cart:
	$(CC) -o cart cart.c $(CFLAGS) $(LDFLAGS) $(LIBS)

lorenz_system:
	$(CC) -o lorenz_system lorenz_system.c $(CFLAGS) $(LDFLAGS) $(LIBS)

music_visualizer:
	$(CC) -o music_visualizer music_visualizer.c kissfft/kiss_fft.c $(CFLAGS) $(LDFLAGS) $(LIBS)

music_visualizer2:
	$(CC) music_visualizer2.c kissfft/kiss_fft.c -o music_visualizer2.exe $(CFLAGS) $(LDFLAGS) $(LIBS)