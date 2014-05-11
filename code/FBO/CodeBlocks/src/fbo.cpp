/**
 * MTAT.03.296 Computer Graphics Seminar.
 * Introduction to GPU computing: Oldschool way to compute using GLSL and OpenGL pipeline
 *
 * Based on the code by Ingemar Ragnemalm
 * http://www.computer-graphics.se/gpu-computing/lab1.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <iostream>

using namespace std;

#include "shader_util.h"
#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_ENTER 13

// -------------------- Variables ------------------- //
int texSize = 256;
int loopCount = 1000;
int n, m;

// A structure to collect FBO info
typedef struct FBOstruct {
    GLuint texid;       // Texture ID
    GLuint fb;          // Frame Buffer ID
    GLuint rb;          // Render Buffer ID
    int width, height;
} FBOstruct;

// --------------------- Shader --------------------- //
shader_prog shader("../src/fbo.vert.glsl", "../src/fbo.frag.glsl");

// Application variables
struct FBOstruct *fbo1, *fbo2;

//Create FrameBuffer Object
struct FBOstruct *initFloatFBO(int width, int height, float *data) {

    struct FBOstruct *fbo = (struct FBOstruct *)malloc(sizeof(struct FBOstruct));

    fbo->width = width;
    fbo->height = height;

    // initialize texture
    glGenTextures(1, &fbo->texid);
    glBindTexture(GL_TEXTURE_2D, fbo->texid);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // create framebuffer object and render buffer
    glGenFramebuffers(1, &fbo->fb);      // frame buffer id
    glGenRenderbuffers(1, &fbo->rb);     // render buffer id
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texid, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}


// Choose input data (textures) and output data (FBO)
void useFBO(struct FBOstruct *in, struct FBOstruct *out) {
    glViewport(0, 0, out->width, out->height);
    glBindFramebuffer(GL_FRAMEBUFFER, out->fb);
    glBindTexture(GL_TEXTURE_2D, in->texid);
}


// Draw a single quad using the selected shader
void runComputations() {

    // Erase in case we need it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
}


/**
 * Program entry point
 */
int main(int argc, char **argv) {

    // initialize GLUT
    glutInit(&argc, argv);

    // standard OpenGL stuff
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(64, 20);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("computing");

    // Initialize GLEW.
    glewExperimental = true; // This is a hack. Without it the current GLEW version fails to load
                             // some extension functions. See http://www.opengl.org/wiki/OpenGL_Loading_Library
    if (glewInit() != GLEW_OK) {
        cout << "Glew initialization failed" << endl;
        return 1;
    }

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glFlush();

    // create test data
    n = texSize;
    m = texSize;
    float* data = (float*)malloc(4 * texSize * texSize * sizeof(float));
    float* result = (float*)malloc(4 * texSize * texSize * sizeof(float));
    for (long i = 0; i < texSize * texSize * 4; i++) {
        data[i] = i + 1.0;
    }

    // create an FBO for each texture
    fbo1 = initFloatFBO(m, n, data);
    fbo2 = initFloatFBO(m, n, NULL);

    // read and compile shader programs
    shader.use();

    // Set orthographic projection
    gluOrtho2D(0.0, (GLfloat) n, 0.0, (GLfloat) m);

    // draw
    int startTime = glutGet(GLUT_ELAPSED_TIME);
    for (int loop = 0; loop < loopCount; loop++) {
        useFBO(fbo1, fbo2);
        runComputations();
        glFlush();
    }
    int endTime = glutGet(GLUT_ELAPSED_TIME);

    // and read back
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, texSize, texSize, GL_RGBA, GL_FLOAT, result);

    // print out results
    long loopSize = 16;
    printf("Data before computing:\n");
    for (long i = 0; i < loopSize; i++) {
        printf("%f\n",data[i]);
    }
    printf("Data after computing:\n");
    for (long i = 0; i < loopSize; i++) {
        printf("%f\n",result[i]);
    }
    printf("Total ms (GPU): %d\n", endTime - startTime);

    // Same thing on the CPU should take longer.
    startTime = glutGet(GLUT_ELAPSED_TIME);
    for (int loop = 0; loop < loopCount; loop++) {
        for (long i=0; i < texSize * texSize * 4; i++) {
            result[i] = sqrt(data[i]);
        }
    }
    endTime = glutGet(GLUT_ELAPSED_TIME);
    printf("Total ms (CPU): %d\n", endTime - startTime);

    system("pause");
}
