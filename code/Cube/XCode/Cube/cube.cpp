/**
 * MTAT.03.296 Computer Graphics Seminar.
 * Introduction to GPU computing: Normal application of shaders
 *
 * Based on the code by Konstantin Tretyakov
 * https://github.com/konstantint/ComputerGraphics2013/tree/master/Practices/10-OpenGL-3.1-Plus
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLUT/glut.h>
#include <math.h>
#include <iostream>

using namespace std;

#include "MatrixStack.h"
using glutil::MatrixStack; // We shall be using a custom matrix stack implementation

#include "shader_util.h"
#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_ENTER 13

// --------------- Forward declarations ------------- //
int main(int argc, char* argv[]);
void display();
void idle();
void prepare_vertex_data();
void draw_cube(float t);
void calculate_fps();

// --------------------- Shader --------------------- //
shader_prog shader("../shaders/cube.vert.glsl", "../shaders/cube.frag.glsl");

// -------------------- Variables ------------------- //
GLuint cubeVertexArrayHandle;
GLuint cubeArrayBufferHandle;
int frameCount = 0;
float fps = 0;
int currentTime = 0;
int previousTime = 0;


/**
 * Program entry point
 */
int main(int argc, char* argv[]) {
    
    // Initialize GLUT and GLEW
    glutInit(&argc, argv);

    // Tell GLUT to set the context up so that we will not be able to use any of the old functions
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    
    // Initialize window
    glutInitWindowSize(800, 800);
    glutCreateWindow("Cube");

    // Initialize GLEW.
    glewExperimental = true; // This is a hack. Without it the current GLEW version fails to load
                             // some extension functions. See http://www.opengl.org/wiki/OpenGL_Loading_Library
    if (glewInit() != GLEW_OK) {
        cout << "Glew initialization failed" << endl;
        return 1;
    }

    // Register handlers
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    // Configuration
    glClearColor(0, 0, 0, 0);

    // Depth test
    glEnable(GL_DEPTH_TEST);

    // We *must* use a shader in a OpenGL 3.2+ core profile
    shader.use();

    // We must specify the name of the "output" variable of our
    // fragment shader. gl_FragColor does not work any more.
    // The good thing - you can output to several channels in parallel.
    glBindFragDataLocation(shader, 0, "fragColor");
    
    // Prepare the vertex data
    prepare_vertex_data();

    // Run the event loop
    glutMainLoop();

    // Finally, be polite and clean up the objects from GPU memory
    glDeleteVertexArrays(1, &cubeVertexArrayHandle);
    glDeleteBuffers(1, &cubeArrayBufferHandle);

    return 0;
    
}


/**
 * GLUT program can perform background processing tasks or continuous animation when
 * window system events are not being received. If enabled, the idle callback is
 * continuously called when events are not being received.
 */
void idle() {
    
    //  Calculate FPS
    calculate_fps();
    
    // Imagine you don't have a GPU and you want to computine lighting
    // Probably you can do it more efficiently than this but it will give and idea
    // Pretend that we do some meaningful floating points computations for 600*600 pixels
    // Uncomment this code and observe the resulting FPS
    /*
    float dummy = 0.0;
    for (int i = 0; i < 600; i++) {
        for (int j = 0; j < 600; j++) {
            
            // 3 floating point mulitplications of vectors of length 3
            // two dot products of vectors of length 3
            for (int t = 0; t < 15; t++) {
                dummy = 0.7 * 0.7;
            }
            
            // raising to power
            dummy = pow(dummy, 20);
        }
    }
    */
    
    //  Call display function (draw the current frame)
    glutPostRedisplay();
}


// In this function we:
//  * Initialize the vertex array object
//  * Create an array buffer with data
//  * Associate data from the array buffer with vertex attribute(s)
void prepare_vertex_data() {

    // First create the vertex array object (VAO).
    // We can't do anything without a VAO.
    // See http://www.opengl.org/wiki/Vertex_Specification
    glGenVertexArrays(1, &cubeVertexArrayHandle);
    
    // We can have a separate VAO for each object in our scene.
    // But we have to specify what is the current VAO we are working with.
    glBindVertexArray(cubeVertexArrayHandle); // This will be our current VAO now.

    // Next, we need to set up the data for our vertex objects
    // Data is provided in the form of "vertex attributes"
    // Before we can configure vertex attributes, however, we have
    // to upload the data itself, and for that we need to create an *array buffer*
    glGenBuffers(1, &cubeArrayBufferHandle);
    
    // Now let us fill the buffer with data
    // OpenGL wants you to first to "attach" the buffer object
    // you just created into the current context under the GL_ARRAY_BUFFER label.
    // (this makes it the "current buffer"). Whatever you will then make with this
    // current buffer

    const float cubeVertexData[] = {
        
        // left
        0, 0, 0,   -1, 0, 0,
        0, 1, 0,   -1, 0, 0,
        0, 1, 1,   -1, 0, 0,
        0, 1, 1,   -1, 0, 0,
        0, 0, 1,   -1, 0, 0,
        0, 0, 0,   -1, 0, 0,
        
        // right
        1, 0, 0,   1, 0, 0,
        1, 0, 1,   1, 0, 0,
        1, 1, 1,   1, 0, 0,
        1, 1, 1,   1, 0, 0,
        1, 1, 0,   1, 0, 0,
        1, 0, 0,   1, 0, 0,
        
        // top
        0, 1, 0,   0, 1, 0,
        0, 1, 1,   0, 1, 0,
        1, 1, 1,   0, 1, 0,
        1, 1, 1,   0, 1, 0,
        1, 1, 0,   0, 1, 0,
        0, 1, 0,   0, 1, 0,
        
        // bottom
        0, 0, 0,   0, -1, 0,
        0, 0, 1,   0, -1, 0,
        1, 0, 1,   0, -1, 0,
        1, 0, 1,   0, -1, 0,
        1, 0, 0,   0, -1, 0,
        0, 0, 0,   0, -1, 0,
        
        // front
        0, 0, 1,   0, 0, 1,
        0, 1, 1,   0, 0, 1,
        1, 1, 1,   0, 0, 1,
        1, 1, 1,   0, 0, 1,
        1, 0, 1,   0, 0, 1,
        0, 0, 1,   0, 0, 1,
        
        // back
        0, 0, 0,   0, 0, -1,
        0, 1, 0,   0, 0, -1,
        1, 1, 0,   0, 0, -1,
        1, 1, 0,   0, 0, -1,
        1, 0, 0,   0, 0, -1,
        0, 0, 0,   0, 0, -1,
        
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, cubeArrayBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexData), cubeVertexData, GL_STATIC_DRAW);
    
    // Note the last parameter of glBufferData. It specifies how often we plan to change the data
    // GL_STATIC_DRAW means the array is fixed. If we would plan to continuously write new
    // values there, it would be GL_STREAM_DRAW.

    // Creating an array buffer and filling it with numbers is still not enough:
    // OpenGL has no idea what those numbers are. Are they vertex coordinates? Normals? Colors?
    // We must now specify which elements in the array correspond to which *attributes*
    // In our case there will only be one attribute - vec4 position.
    // We shall assign this attribute to "attribute index 0".

    // Position
    glEnableVertexAttribArray(0); // Enable attribute 0
    // Configure attribute 0
    glVertexAttribPointer(0, // Attribute index
                          3, // Attribute is a three-element vector (vec3)
                          GL_FLOAT, // Means tt is a two-element vector of floats (not ivec2 or smth)
                          GL_FALSE, // It is not in a fixed-point format
                                    // This is something we did not cover in the course, so google it up, e.g. http://jet.ro/files/The_neglected_art_of_Fixed_Point_arithmetic_20060913.pdf
                                    // We did not cover it because we largely do not use it anywhere.
                                    // To indicate that we do not use it, we say GL_FALSE here.
                          6*sizeof(float), // 1.1 // Spacing between consequtive elements, in bytes.
                          (const GLvoid*)(0*sizeof(float)) // Offset to where the first element starts, in bytes.
                          );

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6*sizeof(float), // Spacing between consequtive elements, in bytes.
                          (const GLvoid*)(3*sizeof(float)) // Offset to where the first element starts, in bytes.
                          );

}


/**
 * When GLUT determines that the window needs to be redisplayed, the display callback for the window is called.
 */
void display() {
    float t = glutGet(GLUT_ELAPSED_TIME);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear screen
    draw_cube(t);
}


/**
 * Routine to draw the cube
 */
void draw_cube(float t) {
    
    // We bind VAO and buffer again here in case we have bound something else in between
    glBindVertexArray(cubeVertexArrayHandle);
    glBindBuffer(GL_ARRAY_BUFFER, cubeArrayBufferHandle);

    // Model View Projection matrix
    MatrixStack mvp;
    
    // Model View matrix (needed to compute normals)
    MatrixStack mv;
    
    mvp.Perspective(60, 1, 0.5, 100);
    mvp.LookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    mvp.Rotate(glm::vec3(0, 1, 1), t*0.1);
    mvp.Translate(glm::vec3(-0.5, -0.5, -0.5));
    
    mv.LookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    mv.Rotate(glm::vec3(0, 1, 1), t*0.1);
    mv.Translate(glm::vec3(-0.5, -0.5, -0.5));
    
    shader.uniformMatrix4fv("modelViewProjectionMatrix", glm::value_ptr(mvp.Top()));
    shader.uniformMatrix4fv("modelViewMatrix", glm::value_ptr(mv.Top()));

    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glutSwapBuffers();
}


/**
 * Calculates the frames per second
 */
void calculate_fps() {
    
    //  Increase frame count
    frameCount++;
    
    //  Get the number of milliseconds since glutInit called
    //  (or first call to glutGet(GLUT ELAPSED TIME)).
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    
    //  Calculate time passed
    int timeInterval = currentTime - previousTime;
    
    if (timeInterval > 1000)
    {
        //  calculate the number of frames per second
        fps = frameCount / (timeInterval / 1000.0f);
        
        //  Set time
        previousTime = currentTime;
        
        //  Reset frame count
        frameCount = 0;
        
        // Print out FPS to console
        printf("FPS: %4.2f\n", fps);
    }
}
