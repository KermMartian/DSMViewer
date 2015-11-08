#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#include <gdal/gdal_priv.h>

#include "camera.h"

static GLint window;
static unsigned int SCREENWIDTH = 1024;
static unsigned int SCREENHEIGHT = 768;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static unsigned int FPS = 0;
static bool fullScreen = false;
static GLuint glID;

int main(int argc, char* argv[]);

void winInit (const std::string& filename);
void reshape(int w, int h);
void display(void);
void idle(void);
void key(unsigned char keyPressed, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
