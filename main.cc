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

void winInit (const std::string & filename) {
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
    // drawSolidModel ();
    glFlush ();
    glutSwapBuffers ();
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
        size_t numPoints = 0;
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

void printGDALInfo(GDALDataset* poDataset) {
	double adfGeoTransform[6];
	printf("Driver: %s/%s\n",
		poDataset->GetDriver()->GetDescription(), 
		poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
	printf("Size is %dx%dx%d\n", 
		poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
		poDataset->GetRasterCount() );
	if(poDataset->GetProjectionRef()  != NULL ) {
		printf( "Projection is `%s'\n", poDataset->GetProjectionRef() );
	}
	if(poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
		printf( "Origin = (%.6f,%.6f)\n",
				adfGeoTransform[0], adfGeoTransform[3] );
		printf( "Pixel Size = (%.6f,%.6f)\n",
				adfGeoTransform[1], adfGeoTransform[5] );
	}
}

int main(int argc, char ** argv) {
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ( "DSMViewer");

    if (argc != 2) {
		fprintf(stderr, "Need 2 arguments\n");
		return EXIT_FAILURE;
	}
  
	// Handle opening GDAL (DSM) dataset
	GDALDataset* poDataset;
	GDALAllRegister();
	poDataset = (GDALDataset*)GDALOpen(argv[1], GA_ReadOnly);
	if (poDataset == NULL) {
		fprintf(stderr, "Failed to open GDAL dataset '%s'\n", argv[1]);
	}
	printGDALInfo(poDataset);

    winInit(std::string(argv[1]));
  
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

