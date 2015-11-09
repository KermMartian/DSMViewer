// --------------------------------------------------------------------------
// DSM Viewer
// Copyright(C) 2015
// Christopher Mitchell
//
// Portions adapted from gMini,
// a minimal Glut/OpenGL app to extend                              
// Copyright(C) 2007-2009                
// Tamy Boubekeur
//                                                                            
// All rights reserved.                                                       
//                                                                            
// This program is free software; you can redistribute it and/or modify       
// it under the terms of the GNU General Public License as published by       
// the Free Software Foundation; either version 2 of the License, or          
// (at your option) any later version.                                        
//                                                                            
// This program is distributed in the hope that it will be useful,            
// but WITHOUT ANY WARRANTY; without even the implied warranty of             
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              
// GNU General Public License (http://www.gnu.org/licenses/gpl.txt)           
// for more details.                                                          
//                                                                          
// --------------------------------------------------------------------------
   
#include "main.h"

void winInit(void) {
    glewInit();
    if (glewGetExtension("GL_ARB_vertex_buffer_object") != GL_TRUE) {
        fprintf(stderr, "Driver does not support Vertex Buffer Objects\n");
        exit(EXIT_FAILURE);
    }
  
    camera.resize(SCREENWIDTH, SCREENHEIGHT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void reshape(int w, int h) {
    camera.resize(w, h);
}

void display () {
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply();

	glBegin(GL_POINTS);
	glColor3f(1., 1., 1.);
	for(size_t i = 0; i < numPoints; i++) {
		glVertex3f(pa_x[i], pa_y[i], pa_z[i]);
	}
	glEnd();

    glFlush();
    glutSwapBuffers();
}

void idle() {
    static float lastTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
    static unsigned int counter = 0;
    counter++;
    float currentTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
    if (currentTime - lastTime >= 1000.0f) {
        FPS = counter;
        counter = 0;
        static char FPSstr[128];
		sprintf(FPSstr, "gMini: %zu points - solid - %d FPS.",
				numPoints, FPS);
        glutSetWindowTitle (FPSstr);
        lastTime = currentTime;
    
    }
    glutPostRedisplay();
}

void key(unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }      
        break;
    case 'q':
    case 27:
        exit(EXIT_SUCCESS);
        break;
    default:
        break;
    }
    idle();
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle();
}

void motion(int x, int y) {
    if (mouseRotatePressed == true)  {
        camera.rotate(x, y);
    } else if (mouseMovePressed == true) {
        camera.move((x-lastX)/static_cast<float>(SCREENWIDTH),
                    (lastY-y)/static_cast<float>(SCREENHEIGHT),
                    0.0);
        lastX = x;
        lastY = y;
    } else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}

int main(int argc, char ** argv) {
	int rval;

	int c = 0;
	opterr = 0;

	std::string filename = "";
	int band = -1;
	size_t pixel_step = 1;

	while((c = getopt(argc, argv, "s:b:")) != -1) {
		switch(c) {
			case 's':
				pixel_step = atoi(optarg);
				break;
			case 'b':
				band = atoi(optarg);
				break;
			default:
			case '?':
				fprintf(stderr, "Unrecognized option '%c' (arg '%s')\n",
				        optopt, argv[optind]);
				return EXIT_FAILURE; 
		}
	}
	if (optind < argc) {
		filename = argv[optind];
	}

    if (!filename.length() || band < 0 || pixel_step < 1) {
		fprintf(stderr, "Bad arguments!\n");
		return EXIT_FAILURE;
	}
  
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow("DSMViewer");

	// Handle opening GDAL (DSM) dataset
	GDALHelper dataset = GDALHelper(filename);
	dataset.printGDALInfo();
	dataset.selectBand(band);

    winInit();

	// Load band metadata
	size_t gdal_x, gdal_y;
	float extents[6];
	if (rval = dataset.getBandSize(gdal_x, gdal_y)) {
		fprintf(stderr, "Failed to fetch band size\n");
		return EXIT_FAILURE;
	}
	if (rval = dataset.getBandExtents(extents)) {
		fprintf(stderr, "Failed to fetch band extents\n");
		return EXIT_FAILURE;
	}
	
	// Reserve arrays for point X, Y, and Z coordinates
	pa_x = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	pa_y = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	pa_z = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	if (!pa_x || !pa_y || !pa_z) {
		fprintf(stderr, "Exhausted memory reserving point buffers\n");
		return EXIT_FAILURE;
	}

	// Now load actual points
	fprintf(stdout, "Loading band %d...\n", band);
	numPoints = 0;
	for(size_t y = 0; y < gdal_y; y += pixel_step) {
		float *scanline, *coords_x, *coords_y;
		if (dataset.getBandScanline(scanline, coords_x, coords_y, y)) {
			fprintf(stderr, "Failed to read scanline %zu\n", y);
			return EXIT_FAILURE;
		}

		// Store data
		for(size_t x = 0; x < gdal_x; x+= pixel_step) {
			pa_x[numPoints] = coords_x[x];
			pa_y[numPoints] = coords_y[x];
			pa_z[numPoints++] = scanline[x];
		}
		dataset.freeBandArray(scanline);
		dataset.freeBandArray(coords_x);
		dataset.freeBandArray(coords_y);

		if (!(y % 16)) {
			fprintf(stdout, "\r%.0f%% (%zu points)...      ",
					100. * ((float)y / (float)gdal_y), numPoints);
			fflush(stdout);
		}
	}
	fprintf(stdout, "\n");
	
	// Move the camera to the center of the point cloud
	camera.move((extents[0] + extents[1]) / 2.,
	            (extents[2] + extents[3]) / 2.,
	            (extents[4] + extents[5]) / 2.);
  
	// Set up OpenGL settings and callbacks
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutReshapeFunc(reshape);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
 
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
  
    glutMainLoop();
    return EXIT_SUCCESS;
}

