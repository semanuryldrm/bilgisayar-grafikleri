cember:
	clang -o cember cember.c -lraylib -lm
cart:
	clang -o cart cart.c -lraylib -lm
lorenz_system:
	clang -o lorenz_system lorenz_system.c -lraylib -lm
music_visualizer:
	clang -o music_visualizer music_visualizer.c kissfft/kiss_fft.c -lraylib -lm -I./kissfft/
music_visualizer2:
	clang music_visualizer2.c -o music_visualizer2 kissfft/kiss_fft.c -lraylib -lm -I./kissfft/ -ldl -lpthread -lGL -lX11
