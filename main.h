// --------------------------------------------------------------------------
// DSM Viewer
// Copyright(C) 2015
// Christopher Mitchell
//                                                                            
// All rights reserved. See main.cc for license.                              
//                                                                          
// --------------------------------------------------------------------------
   
#pragma once

#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#include "camera.h"
#include "gdal-helper.h"

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

// Store pre-computed point positions
static float* pa_x = NULL;
static float* pa_y = NULL;
static float* pa_z = NULL;

// Store pre-computed point colors
static float* ca_r = NULL;
static float* ca_g = NULL;
static float* ca_b = NULL;

// Other global point state
static size_t numPoints = 0;
static float exag_fac = 1.;
static float move_scale_fac = 256.f;

int main(int argc, char* argv[]);
void printUsage(char* argv[]);
void printKeys(char* argv[]);

void getHeatMapColor(float value, float *red, float *green, float *blue);

void winInit(void);
void reshape(int w, int h);
void display(void);
void idle(void);
void key(unsigned char keyPressed, int x, int y);
void keySpecial(int keyPressed, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

