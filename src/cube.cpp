//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "vgl.h"
#include "vmath.h"
#include "LoadShaders.h"
#include <stdlib.h>

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];
GLuint  VAOs2[NumVAOs];
GLuint vColor2;
GLuint vPosition2;
GLuint vColor;
GLuint vPosition1;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
const int NumVertices2 = 36;

GLenum glErr;

using namespace vmath;

vec4 points[NumVertices];
vec4 colors[NumVertices];

vec4 points2[NumVertices2];
vec4 colors2[NumVertices2];

// Vertices of a unit cube centered at origin, sides aligned with axes
vec4 vertices[16] = {
	vec4(-0.5, -0.5,  0.5, 1.0),
	vec4(-0.5,  0.5,  0.5, 1.0),
	vec4(0.5,  0.5,  0.5, 1.0),
	vec4(0.5, -0.5,  0.5, 1.0),
	vec4(-0.5, -0.5, -0.5, 1.0),
	vec4(-0.5,  0.5, -0.5, 1.0),
	vec4(0.5,  0.5, -0.5, 1.0),
	vec4(0.5, -0.5, -0.5, 1.0),

	vec4(-0.5, 1.0,  1.5, 1.0),
	vec4(-0.5,  2.0,  1.5, 1.0),
	vec4(0.5,  2.0,  1.5, 1.0),
	vec4(0.5, 1.0,  1.5, 1.0),
	vec4(-0.5, 1.0, -1.5, 1.0),
	vec4(-0.5,  2.0, -1.5, 1.0),
	vec4(0.5,  2.0, -1.5, 1.0),
	vec4(0.5, 1.0, -1.5, 1.0)
};

// RGBA colors
vec4 vertex_colors[16] = {
	vec4(0.0, 0.0, 0.0, 1.0),  // black
	vec4(1.0, 0.0, 0.0, 1.0),  // red
	vec4(1.0, 1.0, 0.0, 1.0),  // yellow
	vec4(0.0, 1.0, 0.0, 1.0),  // green
	vec4(0.0, 0.0, 1.0, 1.0),  // blue
	vec4(1.0, 0.0, 1.0, 1.0),  // magenta
	vec4(1.0, 1.0, 1.0, 1.0),  // white
	vec4(0.0, 1.0, 1.0, 1.0),  // cyan

	vec4(0.0, 0.0, 0.0, 1.0),  // black
	vec4(1.0, 0.0, 0.0, 1.0),  // red
	vec4(1.0, 1.0, 0.0, 1.0),  // yellow
	vec4(0.0, 1.0, 0.0, 1.0),  // green
	vec4(0.0, 0.0, 1.0, 1.0),  // blue
	vec4(1.0, 0.0, 1.0, 1.0),  // magenta
	vec4(1.0, 1.0, 1.0, 1.0),  // white
	vec4(0.0, 1.0, 1.0, 1.0)
};

// Viewing transformation parameters
GLfloat radius = 1.0;
GLfloat theta = 0.0;
GLfloat phi = 0.0;
GLfloat tarPointX = 0.0;
GLfloat tarPointY = 0.0;
GLfloat tarPointZ = 0.0;
GLboolean cullFlag = false;
GLboolean depthFlag = false;
GLboolean polyFlag = false;
GLboolean projFlag = false;
GLboolean flag = false;
const GLfloat  dr = radians(5.0); // 'deltaRadians'

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  model_view2;
GLuint cubeProgram;
GLuint cuboidProgram;
// Projection transformation parameters
GLfloat  fovy = 90.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat width = 800.0;
GLfloat height = 800.0;
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.25, zFar = 6.0;
GLfloat scaleX;

GLuint  projection; // projection matrix uniform shader variable location
GLuint  projection2;
mat4 p;

GLfloat angleM = 0.0, axis[3], trans[3];
GLboolean trackingMouse = false;
GLboolean redrawContinue = false;
GLboolean trackBallMove = false;
float lastPos[3] = { 0.0, 0.0, 0.0 };
int curX, curY, startX, startY;

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices

int Index = 0;

void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}

int Index2 = 0;
void
quad2(int a, int b, int c, int d)
{
	colors2[Index2] = vertex_colors[a]; points2[Index2] = vertices[a]; Index2++;
	colors2[Index2] = vertex_colors[b]; points2[Index2] = vertices[b]; Index2++;
	colors2[Index2] = vertex_colors[c]; points2[Index2] = vertices[c]; Index2++;
	colors2[Index2] = vertex_colors[a]; points2[Index2] = vertices[a]; Index2++;
	colors2[Index2] = vertex_colors[c]; points2[Index2] = vertices[c]; Index2++;
	colors2[Index2] = vertex_colors[d]; points2[Index2] = vertices[d]; Index2++;
}
//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);

}

void colorcuboid()
{
	quad2(9, 8, 11, 10);
	quad2(10, 11, 15, 14);
	quad2(11, 8, 12, 15);
	quad2(14, 13, 9, 10);
	quad2(12, 13, 14, 15);
	quad2(13, 12, 8, 9);
}

//----------------------------------------------------------------------------

//
// init
//
void
init(void)
{
	ShaderInfo  shaders[] =
	{
		{ GL_VERTEX_SHADER, "vshader42.glsl" },
		{ GL_FRAGMENT_SHADER, "fshader42.glsl" },
		{ GL_NONE, NULL }
	};

	// load shaders
	cubeProgram = LoadShaders(shaders);
	glUseProgram(cubeProgram);

	// set up vertex arrays
	vPosition1 = glGetAttribLocation(cubeProgram, "vPosition");
	

	vColor = glGetAttribLocation(cubeProgram, "vColor");
	

	// get uniforms' locations
	model_view = glGetUniformLocation(cubeProgram, "model_view");
	projection = glGetUniformLocation(cubeProgram, "projection");
	colorcube();
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Triangles]);

	glGenBuffers(NumBuffers, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glEnableVertexAttribArray(vPosition1);
	glVertexAttribPointer(vPosition1, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	// misc global OGL initialializes
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearBufferfv(GL_COLOR, 0, black);

	assert((glErr = glGetError()) == GL_NO_ERROR);


}

void cuboidInit(void)
{
	ShaderInfo  shaders[] =
	{
		{ GL_VERTEX_SHADER, "vshader42C.glsl" },
		{ GL_FRAGMENT_SHADER, "fshader42C.glsl" },
		{ GL_NONE, NULL }
	};

	// load shaders
	cuboidProgram = LoadShaders(shaders);
	glUseProgram(cuboidProgram);

	// set up vertex arrays
	vPosition2 = glGetAttribLocation(cuboidProgram, "vPosition");
	

	vColor2 = glGetAttribLocation(cuboidProgram, "vColor");
	
	// get uniforms' locations
	model_view2 = glGetUniformLocation(cuboidProgram, "model_view");
	projection2 = glGetUniformLocation(cuboidProgram, "projection");
	colorcuboid();
	GLuint vao;
	glGenBuffers(1, &vao);
	glGenVertexArrays(1, VAOs2);
	glBindVertexArray(VAOs2[0]);


	glBindBuffer(GL_ARRAY_BUFFER, vao);

	glBufferData(GL_ARRAY_BUFFER, sizeof(points2) + sizeof(colors2),
		NULL, GL_STATIC_DRAW);


	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points2), points2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points2), sizeof(colors2), colors2);
	glEnableVertexAttribArray(vPosition2);
	glVertexAttribPointer(vPosition2, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vColor2);
	glVertexAttribPointer(vColor2, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points2)));

	// misc global OGL initialializes
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	
	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearBufferfv(GL_COLOR, 0, black);

	assert((glErr = glGetError()) == GL_NO_ERROR);

}

//----------------------------------------------------------------------------
//
// display
//*
void drawCuboid();
void
display(void)
{
	// clear framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(cubeProgram);
	glBindVertexArray(VAOs[0]);
	// setup matrices
	vec3 eye(radius*sin(theta)*cos(phi),
		radius*sin(theta)*sin(phi),
		radius*cos(theta));
	vec3  at(tarPointX, tarPointY, tarPointZ);
	vec3  up(0.0, 1.0, 0.0);

	mat4  mv;

	for (int i = -2; i <= 2; i++) {
		for (int j = 0; j > -10; j--)
		{
			mv = vmath::lookat(eye, at, up);
			if (trackBallMove)
			{
				mat4 mv1;
				mv1 = rotate(angleM, axis[0], axis[1], axis[2]);
				mv = mv* mv1;
			}
			GLfloat trX = (float)i * 1.5F;
			GLfloat trY = (float)j * 1.5F;

			mat4 mv2;
			mv2 = translate(trX, 0.0F, trY);

			mv = mv * mv2;

			glUniformMatrix4fv(model_view, 1, GL_FALSE, mv);
			aspect = width / height;
			if (!projFlag)
				p = perspective(fovy, aspect, zNear, zFar);
			else
				p = ortho(-1.0, 1.0, -1.0, 1.0, -zNear, -zFar);
			glUniformMatrix4fv(projection, 1, GL_FALSE, p);

			glDrawArrays(GL_TRIANGLES, 0, NumVertices);
		}
	}
	drawCuboid();
}

void drawCuboid()
{
	glUseProgram(cuboidProgram);
	glBindVertexArray(VAOs2[0]);
	vec3 eye(radius*sin(theta)*cos(phi),
		radius*sin(theta)*sin(phi),
		radius*cos(theta));
	vec3  at(tarPointX, tarPointY, tarPointZ);
	vec3  up(0.0, 1.0, 0.0);

	mat4  mv;
	mv = vmath::lookat(eye, at, up);
	for (int i = -2; i <= 2; i++) {
		for (int j = 0; j > -10; j--)
		{
			mv = vmath::lookat(eye, at, up);
			if (trackBallMove)
			{
				mat4 mv1;
				mv1 = rotate(angleM, axis[0], axis[1], axis[2]);
				mv = mv* mv1;
			}
			GLfloat trX = (float)i * 1.5F;
			GLfloat trY = (float)j * 3.2F;
			scaleX = -1.0f * (float)(j - 1)* 0.2f;
			mat4 mv2;
			mv2 = scale(scaleX,1.0f,scaleX);
			mv2 = mv2*translate(trX, 0.0F, trY);

			mv = mv * mv2;

			glUniformMatrix4fv(model_view2, 1, GL_FALSE, mv);
			aspect = width / height;
			if (!projFlag)
				p = perspective(fovy, aspect, zNear, zFar);
			else
				p = ortho(-1.0, 1.0, -1.0, 1.0, -zNear, -zFar);
			glUniformMatrix4fv(projection2, 1, GL_FALSE, p);

			glDrawArrays(GL_TRIANGLES, 0, NumVertices2);
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		theta += dr;
		if (theta > 2 * M_PI) theta = 0.0;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		//startMotion(0, 0);
		theta -= dr;
		if (theta < 0.0) theta = 2 * M_PI;
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		phi += dr;
		if (phi > 2 * M_PI) phi = 0.0;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		phi -= dr;
		if (phi < 0.0) phi = 2 * M_PI;
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		radius -= dr;
		if (radius < 0.0) radius = 2 * M_PI;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		radius += dr;
		if (radius > 2 * M_PI) radius = 0.0;
	}
	else if (key == GLFW_KEY_J && action == GLFW_PRESS)
	{
		tarPointX -= dr;
		//if (tarPoint < 0.0) radius = 2 * M_PI;
	}
	else if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		tarPointX += dr;
		//if (radius > 2 * M_PI) radius = 0.0;
	}
	else if (key == GLFW_KEY_K && action == GLFW_PRESS)
	{
		tarPointZ -= dr;
		//if (tarPoint < 0.0) radius = 2 * M_PI;
	}
	else if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		tarPointZ += dr;
		//if (radius > 2 * M_PI) radius = 0.0;
	}
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		if (!cullFlag)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
			cullFlag = true;
		}
		else
		{
			glDisable(GL_CULL_FACE);
			cullFlag = false;
		}
	}
	else if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		if (!depthFlag)
		{
			glDisable(GL_DEPTH_TEST);
			depthFlag = true;
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
			depthFlag = false;
		}
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		if (!polyFlag)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			polyFlag = true;
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			polyFlag = false;
		}
	}
	else if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		if (mods == GLFW_MOD_SHIFT)
		{
			zNear -= 0.25;
		}
		else
		{
			zNear += 0.25;
		}
	}
	else if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		if (mods == GLFW_MOD_SHIFT)
		{
			zFar -= 0.25;
		}
		else
		{
			zFar += 0.25;
		}
	}
	else if (key == GLFW_KEY_COMMA && action == GLFW_PRESS)
	{
		if (mods == GLFW_MOD_SHIFT)
		{
			width += 25.0;
		}
		else
			width -= 25.0;
	}
	else if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS)
	{
		if (mods == GLFW_MOD_SHIFT)
		{
			height += 25.0;
		}
		else
			height -= 25.0;
	}
	else if (key == GLFW_KEY_SLASH && action == GLFW_PRESS)
	{
		if (!projFlag)
		{
			projFlag = true;
		}
		else
		{
			projFlag = false;
		}
	}

}
void trackball_ptov(int x, int y, int w, int h, float a[3])
{
	float d, v;
	a[0] = (2.0 * x - w) / w;
	a[1] = (h - 2.0 * y) / h;
	d = sqrt(a[0] * a[0] + a[1] * a[1]);
	a[2] = cos((M_PI / 2.0)*(d < 1.0 ? d : 1.0));
	v = 1.0 / sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] *= v; a[1] *= v; a[2] *= v;
}
void startMotion(int x, int y);
void stopMotion(int x, int y);
void mouseMotion(int x, int y, GLFWwindow* window)
{
	float curPos[3], dx, dy, dz;
	if (flag == true)
	{
		int ypos = 600 - y;
		startMotion(x, ypos);
	}
	else
	{
		stopMotion(x, y);
	}
	trackball_ptov(x, y, 800, 600, curPos);
	if (trackingMouse)
	{
		dx = curPos[0] - lastPos[0];
		dy = curPos[1] - lastPos[1];
		dz = curPos[2] - lastPos[2];
		if (dx || dy || dz)
		{
			angleM = 90.0*sqrt(dx*dx + dy*dy + dz*dz);
			axis[0] = lastPos[1] * curPos[2] - lastPos[2] * curPos[1];
			axis[1] = lastPos[2] * curPos[0] - lastPos[0] * curPos[2];
			axis[2] = lastPos[0] * curPos[1] - lastPos[1] * curPos[0];
			lastPos[0] = curPos[0];
			lastPos[1] = curPos[1];
			lastPos[2] = curPos[2];
		}
	}
	display();
}
void spinCube(GLFWwindow* window)
{
	if (redrawContinue)
		display();
}
void startMotion(int x, int y)
{
	trackingMouse = true;
	redrawContinue = false;
	startX = x;
	startY = y;
	curX = x;
	curY = y;
	trackball_ptov(x, y, 800, 600, lastPos);
	trackBallMove = true;
}
void stopMotion(int x, int y)
{
	trackingMouse = false;

	if (startX != x || startY != y)
		redrawContinue = true;
	else
	{
		angleM = 0.0;
		redrawContinue = false;
		trackBallMove = false;
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		ypos = 600 - ypos;
		flag = true;

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{

		flag = false;
	}

}
//----------------------------------------------------------------------------
//
// main
//

#if defined(_WIN32) && defined(NDEBUG)
/*
If compiling in Windows and with debug compilation _disabled_, then only
create the application window and do not create the secondary console window.
(But without the console window, cout/cerr output is no longer visible.)
*/
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
#else
int
main(int argc, char** argv)
#endif
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(800, 600, "Cube", NULL, NULL);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwMakeContextCurrent(window);

	int api, major, minor, revision, profile;

	api = glfwGetWindowAttrib(window, GLFW_CLIENT_API);
	major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	revision = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	profile = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);

	gl3wInit();

	init();
	cuboidInit();

	while (!glfwWindowShouldClose(window))
	{
		display();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		mouseMotion(xpos, ypos, window);
		spinCube(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
}