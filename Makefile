cember:
	clang -o cember cember.c -lraylib -lm
cart:
	clang -o cart cart.c -lraylib -lm
lorenz_system:
	clang -o lorenz_system lorenz_system.c -lraylib -lm
music_visualizer:
	clang -o music_visualizer music_visualizer.c kissfft/kiss_fft.c -lraylib -lm -I./kissfft/