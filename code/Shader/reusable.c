// Reusable code for using shaders. By Ingemar Ragnemalm.

// Includes:
// - Loading text files from disk.
// - Compiling shaders, with infolog printout.
// - Create FBOs
// - Checking framebuffer status

#include "reusable.h"




// --------------------- Shader utilities ------------------------

char* readFile(char * filename)
{
	char * data;
	FILE *theFile;
	char c;
	long howMuch;
	
	// Get file length
	theFile = fopen(filename, "rb");
	if (theFile == NULL)
	{
		printf("%s not found\n", filename);
		return NULL;
	}
	howMuch = 0;
	c = 0;
	while (c != EOF)
	{
		c = getc(theFile);
		howMuch++;
	}
	fclose(theFile);

	printf("%ld bytes\n", howMuch);
	
	// Read it again
	data = (char *)malloc(howMuch);
	theFile = fopen(filename, "rb");
	fread(data, howMuch-1,1,theFile);
	fclose(theFile);
	data[howMuch-1] = 0;

//	printf("%s\n-----\n", data);
	printf("Loaded %s\n", filename);

	return data;
}

// Infolog: Show result of shader compilation
static void printShaderInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

static void printProgramInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

// Load and compile a shader, return reference to it
GLuint loadAndCompileShaders(char *vsFilename, char *fsFilename)
{
	char *vs, *fs;
	GLuint v,f,p;
	
	vs = readFile(vsFilename);
	if (vs != NULL)
	{
		v = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(v, 1, &vs, NULL);
		glCompileShader(v);
		printShaderInfoLog(v);
		printf("-----\n");	
	}

	fs = readFile(fsFilename);
	if (fs != NULL)
	{
		f = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(f, 1, &fs, NULL);
		glCompileShader(f);
		printShaderInfoLog(f);
		printf("-----\n");
	}

	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);
	glLinkProgram(p);
	glUseProgram(p);
	
	printProgramInfoLog(p);
	
	return p;
}


// ---------------------Framebuffer Object utilities ------------------------

// Error checking utility routine, good when dealing with FBOs
static void checkFrameBufferStatus()
{
	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//	printf("Framebuffer status %d\n", (int)status);
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			printf("Framebuffer complete\n");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			printf("Framebuffer unsuported\n");
			/* choose different formats */
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			printf("GL_FRAMEBUFFER_MISSING_ATTACHMENT_EXT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n");
			break;
		default:
			printf("Framebuffer error\n");
	}
}


// *** Create FBO ***
struct FBOstruct *initFBO(int width, int height, char *image)
{
	struct FBOstruct *fbo = malloc(sizeof(FBOstruct));
	
	fbo->width = width;
	fbo->height = height;
	
// initialize texture, empty if no texture given
	glGenTextures(1, &fbo->texid);
	glBindTexture(GL_TEXTURE_2D, fbo->texid);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); // NULL = Empty texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // FBO safe
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
// create objects
	glGenFramebuffersEXT(1, &fbo->fb); // frame buffer id
	glGenRenderbuffersEXT(1, &fbo->rb); // render buffer id
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo->fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
							GL_TEXTURE_2D, fbo->texid, 0);
	
// Renderbuffer (not needed here)
// initialize depth renderbuffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo->rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
// attach renderbuffer to framebuffer depth buffer
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->rb);
	
	checkFrameBufferStatus();
	
	printf("Framebuffer object %d\n", (int)fbo->fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	return fbo;
}

// *** Create FBO ***
struct FBOstruct *initFloatFBO(int width, int height, float *data)
{
	struct FBOstruct *fbo = malloc(sizeof(FBOstruct));
	
	fbo->width = width;
	fbo->height = height;
	
// initialize texture, empty if no texture given
	glGenTextures(1, &fbo->texid);
	glBindTexture(GL_TEXTURE_2D, fbo->texid);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, data); // NULL = Empty texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // FBO safe
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
// create objects
	glGenFramebuffersEXT(1, &fbo->fb); // frame buffer id
	glGenRenderbuffersEXT(1, &fbo->rb); // render buffer id
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo->fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
							GL_TEXTURE_2D, fbo->texid, 0);
	
// Renderbuffer (not needed here)
// initialize depth renderbuffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo->rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
// attach renderbuffer to framebuffer depth buffer
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->rb);
	
	checkFrameBufferStatus();
	
	printf("Framebuffer object %d\n", (int)fbo->fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	return fbo;
}
