build : main.cpp vgl.cpp vgl.h
	g++ -g -O0 -I../include ../src/glad.c -c vgl.cpp 
	g++ -g -O0 -I../include main.cpp ../src/glad.c vgl.o -o main -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl 

clean : 
	rm -f main vgl.o
