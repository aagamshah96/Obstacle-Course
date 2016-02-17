all: sample3D

sample3D: Sample_GL3_3D.cpp glad.c
	g++ -o sample3D Sample_GL3_3D.cpp glad.c -lglfw -ldl -lftgl -lSOIL -lGL -I/usr/include/freetype2 -I/usr/local/lib/ -L/usr/local/lib

clean:
	rm sample3D
