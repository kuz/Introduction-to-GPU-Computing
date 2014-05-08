// Float array processing GPU processing lab
// New, better version of process-array
// by Ingemar Ragnemalm 2010

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <GLUT/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glut.h>
#endif

#ifndef GL_VERSION_2_0
#error This program needs OpenGL 2.0 libraries to compile
#endif

#include "reusable.h"

// Globals

// Image dimensions
int n;
int m;

int lastw, lasth; // Needed to restore the viewport

// Reshape, set viewport and save it
void reshape(int h, int w)
{
	lastw = w;
	lasth = h;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat) n, 0.0, (GLfloat) m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,n,m);
}


// Choose input data (textures) and output data (FBO)
void useFBO(struct FBOstruct *out, struct FBOstruct *in1, struct FBOstruct *in2)
{
	if (out != NULL && (out == in1 || out == in2))
		printf("Warning! Input and output identical!\n");
	
	if (out != 0L)
		glViewport(0, 0, out->width, out->height);
	else
		glViewport(0, 0, lastw, lasth);
	
	if (out != 0L)
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, out->fb);
	else
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glActiveTexture(GL_TEXTURE1);
	if (in2 != 0L)
		glBindTexture(GL_TEXTURE_2D, in2->texid);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	if (in1 != 0L)
		glBindTexture(GL_TEXTURE_2D, in1->texid);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
}

// Draw a single quad using the selected shader
void applyFilter(GLuint shaderProgramObject)
{
    // Erase in case we need it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	// run the edge detection filter over the geometry texture
	// Activate the edge detection filter program
	glUseProgram(shaderProgramObject);
	
	// Viewport-Sized Quad = Data Stream Generator.
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(0, 0);
		glTexCoord2f(0, 1);
		glVertex2i(0, m);
		glTexCoord2f(1, 1);
		glVertex2i(n ,m);
		glTexCoord2f(1, 0);
		glVertex2i(n, 0);
	glEnd();
	
	// disable the filter
	glUseProgramObjectARB(0);
}

// ---------------------------------------------------------------------

#define texSize 256
#define loopCount 1000

// Set the texture unit uniforms for a shader.
// For this application, these can be the same in all shaders.
void setUniforms(GLuint s)
{
    GLint loc;
    
    glUseProgram(s);
    loc = glGetUniformLocation(s, "texUnit");
    glUniform1i(loc, 0); // texture unit 0
    loc = glGetUniformLocation(s, "texUnit2");
    glUniform1i(loc, 1); // texture unit 1
    glUseProgram(0);
}

// Application variables

struct FBOstruct *fbo1, *fbo2, *fbo3;
GLuint	shaderProgramObject;	// the program used to update


void display()
{
	useFBO(NULL, fbo1, NULL); // Render to screen using fbo1 (input image) as input
	applyFilter(shaderProgramObject);

	glFlush();

// Other examples:	
// Render to screen without shader:
//	useFBO(NULL, fbo1, NULL); // Render to screen using fbo1 (input image) as input
//	applyFilter(0); // No shader (just copy the image from fbo1)
// Render from one fbo to another with shader:
//	useFBO(fbo2, fbo1, NULL); // Render to fbo2 using fbo1 as input
//	applyFilter(shaderProgramObject);
}

void keydown(unsigned char key, int x, int y)
{
}

int main(int argc, char **argv)
{	
	char *vs;
	char *fs;
	long i;

    glutInit(&argc, argv);
	
// Standard OpenGL stuff
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(64, 20);
    glutInitWindowPosition(50,50);
    glutCreateWindow("computing");
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glFlush();
	
// Input data
    // create test data
    n = texSize;
    m = texSize;
    float* data = (float*)malloc(4*texSize*texSize*sizeof(float));
    float* result = (float*)malloc(4*texSize*texSize*sizeof(float));
    for (i=0; i<texSize*texSize*4; i++)
        data[i] = i+1.0;

	
// Create an FBO for each texture
	fbo1 = initFloatFBO(m, n, data);
	fbo2 = initFloatFBO(m, n, NULL);
	fbo3 = initFloatFBO(m, n, NULL);

// Read and compile shader program(s)
	shaderProgramObject = loadAndCompileShaders("sqrt.vs", "sqrt.fs");
// Pass texture units to the shader program
// This is necessary to be able to use more than one texture!
    setUniforms(shaderProgramObject);
	
	reshape(n, m);
	
// draw
	int startTime = glutGet(GLUT_ELAPSED_TIME);
	int loop;
	for (loop = 0; loop < loopCount; loop++)
	{
		useFBO(fbo2, fbo1, NULL);
		applyFilter(shaderProgramObject);
	
		glFlush();
	}
	int endTime = glutGet(GLUT_ELAPSED_TIME);

// and read back
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadPixels(0, 0, texSize, texSize,GL_RGBA,GL_FLOAT, result);
// print out results
	long loopSize = 16;
	if (texSize < 32)
		loopSize = texSize*texSize*4;

    printf("Data before computing:\n");
    for (i=0; i<loopSize; i++)
        printf("%f\n",data[i]);
    printf("Data after computing:\n");
    for (i=0; i<loopSize; i++)
        printf("%f\n",result[i]);
    
    printf("Total ms (GPU): %d\n", endTime - startTime);

// Same thing on the CPU! Should take longer time.
	startTime = glutGet(GLUT_ELAPSED_TIME);
	for (loop = 0; loop < loopCount; loop++)
 	   for (i=0; i<texSize*texSize*4; i++)
 	       result[i] = sqrt(data[i]);
	endTime = glutGet(GLUT_ELAPSED_TIME);

	printf("Total ms (CPU): %d\n", endTime - startTime);
}
