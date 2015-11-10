# DSMViewer
3D visualization of Digital Surface Model (DSM) files
By Christopher Mitchell, Ph.D.

This simple OpenGL program lets you view GDAL-compatible files, including
Digital Surface Models (DSMs) stored as Erdas Imagine (.img) files, as a
3D point clouod. Among its features:
* To make large files more tractable, it can use only every
  nth pixel from the source file.
* Points are colored using a standard heatmapping scheme
* Mouse movement can be used to zoom, rotate and move the model
* WASD can also be used for camera movement.

Dependancies:
OpenGL, GLUT, libGDAL.

License: GPL
