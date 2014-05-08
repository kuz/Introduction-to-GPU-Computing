
all : process-array fix-array

process-array : process-array.c reusable.c
	gcc -Wall -o process-array -framework OpenGL -framework GLUT process-array.c reusable.c

fix-array : fix-array.c reusable.c
	gcc -Wall -o fix-array -framework OpenGL -framework GLUT fix-array.c reusable.c

clean :
	rm process-array
	rm fix-array
