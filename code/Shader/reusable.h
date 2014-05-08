#ifndef _REUSABLES_
#define _REUSABLES_


#ifdef __APPLE__
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
#else
	#include <GL/glut.h>
	#include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>


// Shader utilities

char* readFile(char * filename);
GLuint loadAndCompileShaders(char *vsFilename, char *fsFilename);

// FBO utilities

// A structure to collect FBO info
typedef struct FBOstruct
{
	GLuint texid;
	GLuint fb;
	GLuint rb;
	int width, height;
} FBOstruct;

struct FBOstruct *initFBO(int width, int height, char *image);
struct FBOstruct *initFloatFBO(int width, int height, float *data);


#endif
