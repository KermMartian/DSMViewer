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

/* Initialize OpenGL, set camera viewport, set background color. */
void winInit(void) {
    glewInit();
    if (glewGetExtension("GL_ARB_vertex_buffer_object") != GL_TRUE) {
        fprintf(stderr, "Driver does not support Vertex Buffer Objects\n");
        exit(EXIT_FAILURE);
    }
  
    camera.resize(SCREENWIDTH, SCREENHEIGHT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

/* Callback called when the window changes size. */
void reshape(int w, int h) {
    camera.resize(w, h);
}

/* Actually render the points onto the screen, including the
   current camera, the exaggeration factor, and color values. */
void display () {
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply();

	glBegin(GL_POINTS);
	for(size_t i = 0; i < numPoints; i++) {
		glColor3f(ca_r[i], ca_g[i], ca_b[i]);
		glVertex3f(pa_x[i], pa_y[i], pa_z[i] * exag_fac);
	}
	glEnd();

    glFlush();
    glutSwapBuffers();
}

/* While the program is otherwise idle, update the FPS counter
   in the program's titlebar. */
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

/* Handle keys that can be represented by ASCII chars
   or char codes. */
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
	case 'w':
		camera.move(0.f, 0.f, move_scale_fac);
		break;
	case 'a':
		camera.move(move_scale_fac, 0.f, 0.f);
		break;
	case 's':
		camera.move(0.f, 0.f, -move_scale_fac);
		break;
	case 'd':
		camera.move(-move_scale_fac, 0.f, 0.f);
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

/* Handle special keys and events, like the arrow keys */
void keySpecial(int keyPressed, int x, int y) {
    switch (keyPressed) {
	case GLUT_KEY_UP:
		exag_fac *= 2.f;
		fprintf(stdout, "Exaggeration factor increased to %f\n", exag_fac);
		break;
	case GLUT_KEY_DOWN:
		exag_fac *= 0.5f;
		fprintf(stdout, "Exaggeration factor decreased to %f\n", exag_fac);
		break;
	case GLUT_KEY_LEFT:
		move_scale_fac *= 2.f;
		fprintf(stdout, "Movement factor increased to %f\n", move_scale_fac);
		break;
	case GLUT_KEY_RIGHT:
		move_scale_fac *= 0.5f;
		fprintf(stdout, "Movement factor decreased to %f\n", move_scale_fac);
		break;
    default:
        break;
	}
    idle();
}

/* Handle mouse buttons, used in motion() */
void mouse(int button, int state, int x, int y) {
    // Turn CTRL+right click into middle click
    if ((button == GLUT_RIGHT_BUTTON) && (GLUT_ACTIVE_CTRL & glutGetModifiers())) {
        button = GLUT_MIDDLE_BUTTON;
    }

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

/* Callback triggered when the mouse moves around
   the screen. Used for moving the camera, rotating
   the model, and zooming. */
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
        camera.zoom(move_scale_fac * float(y - lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}

// Credit to http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients,
// because I was too lazy to port my own heatmap function from Prizm C.
void getHeatMapColor(float value, float *red, float *green, float *blue) {
	const int NUM_COLORS = 5;
	const static float color[NUM_COLORS][3] = { {0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1,0,0} };
	// blue, cyan, green, yellow, red
 
	int idx1;        // |-- Our desired color will be between these two indexes in "color".
	int idx2;        // |
	float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.
 
	if(value <= 0.) {					// accounts for input <= 0
		idx1 = idx2 = 0.;
	} else if (value >= 1.) {			// accounts for input >= 1
		idx1 = idx2 = NUM_COLORS - 1;
	} else {
		value = value * (NUM_COLORS-1);        // Will multiply value by 3.
		idx1  = floor(value);                  // Our desired color will be after this index.
		idx2  = idx1+1;                        // ... and before this index (inclusive).
		fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).
	}
 
	*red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
	*green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
	*blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void printUsage(char* argv[]) {
	fprintf(stderr, "Usage: %s -b <band> [-s <pixel_step>] <filename>\n", argv[0]);
	fprintf(stderr, "       <band>: Band to use from GDAL-compatible file.\n");
	fprintf(stderr, "       <pixel_step>: Use every nth pixel. Defaults to 1.\n");
	fprintf(stderr, "       <filename>: Path to GDAL-compatible file, eg Erdas Imagine file\n");
}

void printKeys(char* argv[]) {
	fprintf(stdout, "============ Interacting with %s: =============\n", argv[0]);
	fprintf(stdout, " Left-click and drag: rotate\n");
	fprintf(stdout, " Right-click and drag: move camera\n");
	fprintf(stdout, " Middle-click and drag up/down: zoom\n");
	fprintf(stdout, " [Q]: Quit\n");
	fprintf(stdout, " [WASD]: Standard FPS-like camera movement\n");
	fprintf(stdout, " [up]/[down]: Increase/decrease Z-axis exaggeration\n");
	fprintf(stdout, " [left]/[right]: Increase/decrease movement scaling\n");
	fprintf(stdout, "------------------------------------------------------\n");
}

int loadGDAL(std::string filename, int band, int pixel_step) {
	int rval;

	// Handle opening GDAL (DSM) dataset
	GDALHelper dataset = GDALHelper(filename);
	dataset.printGDALInfo();
	dataset.selectBand(band);

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

	float cam_center[3];
	cam_center[0] = (extents[0] + extents[1]) / 2.;
	cam_center[1] = (extents[2] + extents[3]) / 2.;
	cam_center[2] = (extents[4] + extents[5]) / 2.;

	// Reserve arrays for point X, Y, and Z coordinates and colors
	pa_x = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	pa_y = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	pa_z = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	ca_r = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	ca_g = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	ca_b = (float*)malloc(sizeof(float) * gdal_x * gdal_y);
	if (!pa_x || !pa_y || !pa_z || !ca_r || !ca_g || !ca_b) {
		fprintf(stderr, "Exhausted memory reserving point buffers\n");
		return EXIT_FAILURE;
	}

	// Now load actual points
	fprintf(stdout, "Loading band %d...\n", band);
	numPoints = 0;
	for(size_t y = 0; y < gdal_y; y += pixel_step) {
		float *scanline, *coords_x, *coords_y;
		if (dataset.getBandScanline(coords_x, coords_y, scanline, y)) {
			fprintf(stderr, "Failed to read scanline %zu\n", y);
			return EXIT_FAILURE;
		}

		// Store data
		for(size_t x = 0; x < gdal_x; x+= pixel_step, numPoints++) {
			pa_x[numPoints] = coords_x[x] - cam_center[0];
			pa_y[numPoints] = coords_y[x] - cam_center[1];
			pa_z[numPoints] = scanline[x] - cam_center[2]; //(rand() % 128) - 64 + cam_center[2];
			float depth_ratio = (pa_z[numPoints] - extents[4]) / (extents[5] - extents[4]);
			getHeatMapColor(depth_ratio, &ca_r[numPoints], &ca_g[numPoints], &ca_b[numPoints]);
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
	return 0;
}

int loadLAS(std::string filename, int band, int pixel_step) {
	// Try to open specified LAS file
	LASreadOpener las_read_opener;
	las_read_opener.set_file_name(filename.c_str());
	LASreader* las_reader = las_read_opener.open();
	if (NULL == las_reader) {
		fprintf(stderr, "Failed to open '%s'\n", filename.c_str());
		return EXIT_FAILURE;
	}
	fprintf(stdout, "LAS file contains %zu points\n", (size_t)las_reader->npoints);

	float extents[6];
	extents[0] = las_reader->get_min_x();
	extents[1] = las_reader->get_max_x();
	extents[2] = las_reader->get_min_y();
	extents[3] = las_reader->get_max_y();
	extents[4] = las_reader->get_min_z();
	extents[5] = las_reader->get_max_z();
	fprintf(stdout, "LAS extents: x=[%f,%f], y=[%f,%f], z=[%f,%f]\n",
	        extents[0], extents[1], extents[2],
	        extents[3], extents[4], extents[5]);

	float cam_center[3];
	cam_center[0] = (extents[0] + extents[1]) / 2.;
	cam_center[1] = (extents[2] + extents[3]) / 2.;
	cam_center[2] = (extents[4] + extents[5]) / 2.;

	// Reserve arrays for point X, Y, and Z coordinates and colors
	size_t restricted_points = (size_t)las_reader->npoints / pixel_step;
	pa_x = (float*)malloc(sizeof(float) * restricted_points);
	pa_y = (float*)malloc(sizeof(float) * restricted_points);
	pa_z = (float*)malloc(sizeof(float) * restricted_points);
	ca_r = (float*)malloc(sizeof(float) * restricted_points);
	ca_g = (float*)malloc(sizeof(float) * restricted_points);
	ca_b = (float*)malloc(sizeof(float) * restricted_points);
	if (!pa_x || !pa_y || !pa_z || !ca_r || !ca_g || !ca_b) {
		fprintf(stderr, "Exhausted memory reserving point buffers\n");
		return EXIT_FAILURE;
	}

	// Now load actual points
	fprintf(stdout, "Loading %zu points from LAS file...\n", restricted_points);
	for(size_t i = 0; i < restricted_points; i++) {
		// Store data
		las_reader->read_point();
		pa_x[i] = las_reader->get_x() - cam_center[0];
		pa_y[i] = las_reader->get_y() - cam_center[1];
		pa_z[i] = las_reader->get_z() - cam_center[2]; //(rand() % 128) - 64 + cam_center[2];
		float depth_ratio = (pa_z[i] - extents[4]) / (extents[5] - extents[4]);
		getHeatMapColor(depth_ratio, &ca_r[i], &ca_g[i], &ca_b[i]);

		if (!(i % 256)) {
			fprintf(stdout, "\r%.0f%% (%zu points)...      ",
					100. * ((float)i / (float)restricted_points), numPoints);
			fflush(stdout);
		}

		for(size_t j = 0; j < pixel_step - 1; j++) {
			las_reader->read_point();
		}
	}
	numPoints = restricted_points;
	fprintf(stdout, "\n");
	return 0;
}

int main(int argc, char* argv[]) {
	int c = 0;
	opterr = 0;

	std::string filename = "";
	int band = -1;
	size_t pixel_step = 1;

	while((c = getopt(argc, argv, "s:b:h")) != -1) {
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
			case 'h':
				printUsage(argv);
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

    winInit();

	int rval = EXIT_FAILURE;
	size_t ext_pos = filename.find_last_of('.');
	if (std::string::npos != ext_pos &&
        "las" == filename.substr(ext_pos + 1))
	{
		rval = loadLAS(filename, band, pixel_step);
	} else {
		rval = loadGDAL(filename, band, pixel_step);
	}
	if (rval) {
		fprintf(stderr, "Failed to load '%s'\n", filename.c_str());
		return rval;
	}
	printKeys(argv);
	
	// Move the camera to the center of the point cloud
	//fprintf(stdout, "Moving camera to (%f, %f, %f)\n",
	//        cam_center[0], cam_center[1], cam_center[2]);
	//camera.move(-cam_center[0], -cam_center[1], -cam_center[2]);
  
	// Set up OpenGL settings and callbacks for output
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glutIdleFunc(idle);
    glutDisplayFunc(display);

	// Set up OpenGL callbacks for input
    glutKeyboardFunc(key);
	glutSpecialFunc(keySpecial);
    glutReshapeFunc(reshape);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
 
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
  
	// Kick it off!
    glutMainLoop();
    return EXIT_SUCCESS;
}

