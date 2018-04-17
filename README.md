# CG_Rendering-two-geometric-shapes-and-providing-keyboard-mouse-controls

The scene will consist of  50 randomly placed boxes and 50 randomly placed copies of a second shape. The boxes and shapes all rest with one of 
their faces on the plane y=0. Each box is rendered by reusing a single OGL VAO (vertex array object) and by applying a different affine 
transformation (translation, scale, and rotation) to each box so that the VAO is rendered multiple times at different locations.   

The second shape is rendered by creating a second OGL VAO to store the shape’s vertices. This second VAO will be rendered 50 times, but each 
time with a different affine transformation, so 50 copies appear at 50 different positions (i.e. location/orientation/scale). These second set 
of shapes use a different fragment shader and different vertex shader.

An interactive keyboard and mouse controls for controlling the view, like rotating or moving camera, toggle backface culling, modifying 
clipping distance etc.,  are implemented. 

## Requirements
C++ 
OpenGL 4.5
Visual Studio Enterprise 2017
