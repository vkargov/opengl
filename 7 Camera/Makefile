build : main.cpp vgl.cpp vgl.h
	g++ -g -O0 -I../include ../src/glad.c -c vgl.cpp 
	g++ -g -O0 -I../include ../src/glad.c -c camera.cpp
	g++ -g -O0 -I../include ../src/glad.c -c controls.cpp
	g++ -g -O0 -I../include main.cpp ../src/glad.c camera.o controls.o vgl.o -o main -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl 

clean : 
	rm -f main *.o
