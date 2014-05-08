// Float array processing GPU processing lab
// New, better version of fix-array, similar to float-array but
// with a predefined data array and a sader that demonstrates re-arranging
// the array (not sorting but a good start).
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

/*float data[64] =
{
	1,2,3,4,5,6,7,10,9,0.1,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,53,27,28,29,30,31,32,
	33,34,35,36,37,38,39,40,41,42,46,44,45,43,47,48,
	49,50,51,52,26,54,55,56,57,58,59,60,61,62,63,64
};
*/
// => texSize måste vara 4!
float data[64] =
{
	24,2,3,4,5,64,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,1,25,53,27,28,29,30,31,32,
	33,34,35,36,37,38,39,40,41,42,46,44,45,43,47,48,
	49,50,51,52,26,54,55,56,57,58,59,60,61,62,63,6
};

#define texSize 4
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
    float* result = (float*)malloc(4*texSize*texSize*sizeof(float));
	
// Create an FBO for each texture
	fbo1 = initFloatFBO(m, n, data);
	fbo2 = initFloatFBO(m, n, NULL);
	fbo3 = initFloatFBO(m, n, NULL);

// Read and compile shader program(s)
	shaderProgramObject = loadAndCompileShaders("sort.vs", "sort.fs");
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
	if (texSize < 32)
	{
	    printf("Data before computing:\n");
	    for (i=0; i<texSize*texSize*4; i++)
	        printf("%f\n",data[i]);
	    printf("Data after computing:\n");
	    for (i=0; i<texSize*texSize*4; i++)
	        printf("%f\n",result[i]);
	}
	else
	{
	    printf("Data before computing:\n");
	    printf("%f\n",data[texSize*texSize*4-1]);
	    printf("Data after computing:\n");
	    printf("%f\n",result[texSize*texSize*4-1]);
	}
    
	printf("Total ms (GPU): %d\n", endTime - startTime);
	return 0;
}
