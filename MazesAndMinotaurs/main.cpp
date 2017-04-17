/*
 * ---------------- www.spacesimulator.net --------------
 *   ---- Space simulators and 3d engine tutorials ----
 *
 * Original Author: Damiano Vitulli
 * Porting to OpenGL3.3: Movania Muhammad Mobeen
 * Shaders Functions: Movania Muhammad Mobeen
 *
 * This program is released under the BSD licence
 * By using this program you agree to licence terms on spacesimulator.net copyright page
 *
 */


#include <iostream>
#include<cmath>
#include<iomanip>
#include<vector>

#include <gl/glew.h>
#include <gl/glut.h>
#include "GLSLShader.h"
#include "GraphicsObject.h"
#include "3dsloadertypes.h"
#include "3dsloader.h"
#include "TGALoader.h"
#include "texture.h"
#include"SkyBox.h"

#include "Angel.h"
#include <cassert>


#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

using namespace std; 
// Global Projection Matrices
mat4 projection, modelview, translate;


//my skybox object
SkyBox my_skybox;

int screen_width=1024;
int screen_height=1024;


//controls to move
GLfloat zoom = 0.0;
vec4 view(0.0, 0.0, -2.0, 0.0);

GLfloat fovy = 60.0;
GLfloat aspect = 1.0;
GLfloat zNear = 0.01;
GLfloat zFar = 1000.0;

GLfloat dir = 1.0;
GLfloat theta[] = { 0.0,0.0,0.0 };
GLint axis = 1;

point4  eye(0.0, 0.0, 1.0, 1.0);
point4  at(0.0, 0.0, -1.0, 1.0);
vec4    up(0.0, 1.0, 0.0, 0.0);

GLuint vboVerticesID, vboTexCoordID, vboIndicesID, vaoID;

// Buffer ids for the fragment linked list shader 
GLuint fragment_buffer, atomic_counter_buffer, head_pointer_image;

// Vao id for starting the process
GLuint dummy_vao;

GLsizei stride = sizeof(GLfloat)*3;
GLSLShader shader;

int filling=1;

//Now the object is generic, the cube has annoyed us a little bit, or not?
obj_type object;

// Absolute rotation values (0-359 degrees) and rotiation increments for each frame
float rotation_x=0, rotation_x_increment=0.1f;
float rotation_y=0, rotation_y_increment=0.05f;
float rotation_z=0, rotation_z_increment=0.03f;
 
mat4 P;	//projection matrix;
void display(void)
{
	static float angle = 0.0;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  /*clear the window */

	projection = Perspective(fovy, aspect, zNear, zFar);
	modelview = Translate(0.0, 0.0, 1.0)*LookAt(eye, at, up);

	// tell the skybox to draw its vertex
	my_skybox.draw(theta);

	// swap the buffers
	glutSwapBuffers();

	glutPostRedisplay();
}

void arrowKey(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) //look up--doesn't allow you to look past straight up/down (no sommersaults)
	{
		at.y += .5;
	}
	if (key == GLUT_KEY_DOWN) //look down
	{
		at.y -= .5;
	}
	if (key == GLUT_KEY_RIGHT) //turn right
	{
		view = RotateY(-5) * view;//rotate eye -5 degrees
		at = eye + view;
	}
	if (key == GLUT_KEY_LEFT) //turn left
	{
		view = RotateY(5) * view;//rotate eye 5 degrees
		at = eye + view;
	}
	glutPostRedisplay();
}

void mouse(int btn, int state, int x, int y)
{
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)	axis = 1;
	if (btn == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) axis = 0;
	if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) axis = 2;
	glutPostRedisplay();
}

void mouse_move(int x, int y)
{
	zoom = (10.0 / 500.0) * y + 2.0;  // compute zoom factor 
}

void myReshape(int w, int h)
{
	glViewport(0, 0, w, h);
	aspect = GLfloat(w) / h;
}

void key(unsigned char key, int x, int y) //MAKE ZOOM IN AND OUT WORK
{
	if (key == 'w')//move forward (zoom)
	{
		eye = eye + 0.25*view;
		at = at + 0.25*view;
	}

	if (key == 's')//move backward (zoom)
	{
		eye = eye - 0.25*view;
		at = at - 0.25*view;
	}

	if (key == 'q') exit(0);
	glutPostRedisplay();
}

void runtime_error_check()
{
	GLenum err = glGetError();

	if (err) {
		char buf[20];
		sprintf(buf, "Error = %x", err);
		MessageBoxA(NULL, buf, "Error in shader", MB_OK);
	}
}

void InitShaders(void)
{
	shader.LoadFromFile(GL_VERTEX_SHADER, "shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shader.frag");
	shader.CreateAndLinkProgram();
	shader.Use();
	shader.AddAttribute("vVertex");
	shader.AddAttribute("vUV");
	shader.AddUniform("MVP");
	shader.AddUniform("textureMap");
	glUniform1i(shader("textureMap"), 0);
	shader.UnUse();

	GL_CHECK_ERRORS
}

void InitVAO() {
	GL_CHECK_ERRORS
		//Create vao and vbo stuff
		glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboTexCoordID);
	glGenBuffers(1, &vboIndicesID);

	GL_CHECK_ERRORS

		glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * object.vertices_qty, &object.vertex[0], GL_STATIC_DRAW);
	GL_CHECK_ERRORS
		glEnableVertexAttribArray(shader["vVertex"]);
	glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE, stride, 0);
	GL_CHECK_ERRORS
		glBindBuffer(GL_ARRAY_BUFFER, vboTexCoordID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * object.vertices_qty, &object.mapcoord[0], GL_STATIC_DRAW);
	GL_CHECK_ERRORS
		glEnableVertexAttribArray(shader["vUV"]);
	glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
	GL_CHECK_ERRORS
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 3 * object.polygons_qty, &object.polygon[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	GL_CHECK_ERRORS
}
void InitGL() {
	glGetError();
	GL_CHECK_ERRORS
	glClearColor(0.0f,0.0f,0.2f,0.0f);	
	GL_CHECK_ERRORS

	glEnable(GL_DEPTH_TEST); // We enable the depth test (also called z buffer)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)	
	GL_CHECK_ERRORS

	my_skybox.init_data();	        // Setup the data for the this object
	my_skybox.init_VAO();           // Initialize the vertex array object for this object
	my_skybox.init_VBO();			// Initialize the data buffers for this object
	my_skybox.init_shader();		// Initialize the shader objects and textures for skybox
	my_skybox.init_texture_map();	// Initialize the texture map for this object

	Load3DS (&object,"maze-oversized with granite texture.3ds");
	InitShaders();	
	InitVAO();
	
	
	// Load the TGA texture and store the openGL id for the texture in the object
    object.id_texture = MyLoadTGA("granite.tga"); 
	 // If the last function returns -1 it means the file was not found so we exit from the program
    if (object.id_texture==-1)
    {
        cerr<<"Image file: texture1.bmp not found"<<endl;
        exit (EXIT_FAILURE);
    }

	GL_CHECK_ERRORS
}

void OnRender() {
	GL_CHECK_ERRORS
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable alpha blending for transparent 
	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// IMPORTANT - alph blending won't work properly for back faces if GL_DEPTH_TEST is emabled!
	glEnable(GL_DEPTH_TEST);

	// Translate Center of Object to origin
	mat4 T = Translate(vec3(-object.center.x, -object.center.y, -object.center.z));

	// Scale the object to the standard viewing volume 
	// NOTE!!!!: I scaled the x differntly from the y to get a thinnner flower...
	mat4 S = Scale(1.0 / (object.max_x - object.min_x), 1.0 / (object.max_x - object.min_x),1.0 / (object.max_x - object.min_x));
	mat4 Rx = RotateX(-90.0);
	mat4 Ry = RotateY(rotation_y);
	mat4 Rz = RotateZ(rotation_z);

	mat4 T_locate = Translate(vec3(0.0, 0.0, -1.5));
	mat4 MVP = P*T_locate*Ry*Rx*S*T;   // Multiplcation backwards because matrices transposed.

	glBindVertexArray(vaoID);
	shader.Use();
	glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, MVP);
	glDrawElements(GL_TRIANGLES, object.polygons_qty * 3, GL_UNSIGNED_SHORT, 0);
	shader.UnUse();
	glBindVertexArray(0);

	glutSwapBuffers();
}
void OnResize(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	//setup the projection matrix
	P = Perspective(50.0f, (GLfloat)w/h, 0.1f, 50.0f);
}
void OnShutdown() { 
	glDeleteTextures(1, &object.id_texture);
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboTexCoordID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);
}
void OnKey(unsigned char key, int x, int y)
{        
    switch (key)
	{  
        case ' ':
            rotation_x_increment=0;
            rotation_y_increment=0;
            rotation_z_increment=0;
        break;
        case 'r': case 'R':
            if (filling==0)
            {
                glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)
                filling=1;
            }   
            else 
            {
                glPolygonMode (GL_FRONT_AND_BACK, GL_LINE); // Polygon rasterization mode (polygon outlined)
                filling=0;
            }
        break;
    }
}
void OnSpecialKey(int key, int x, int y)
{        
    switch (key)
    {
        case GLUT_KEY_UP:
            rotation_x_increment = rotation_x_increment +0.005f;
        break;
        case GLUT_KEY_DOWN:
            rotation_x_increment = rotation_x_increment -0.005f;
        break;
        case GLUT_KEY_LEFT:
            rotation_y_increment = rotation_y_increment +0.005f;
        break;
        case GLUT_KEY_RIGHT:
            rotation_y_increment = rotation_y_increment -0.005f;
        break;
    }
}
void OnIdle() {
	rotation_x = rotation_x + rotation_x_increment;
    rotation_y = rotation_y + rotation_y_increment;
    rotation_z = rotation_z + rotation_z_increment;

    if (rotation_x > 359) rotation_x = 0;
    if (rotation_y > 359) rotation_y = 0;
    if (rotation_z > 359) rotation_z = 0;
	
	glutPostRedisplay();
}

void main(int argc, char** argv) {
	atexit(OnShutdown);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("Mazes and Minotaurs");
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Error: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Driver supports OpenGL 3.3\nDetails:"<<endl;
		}
	}
	cout<<"Using GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"Vendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"Renderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"Version: "<<glGetString (GL_VERSION)<<endl;
	cout<<"GLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;
	InitGL();
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKey);
	glutSpecialFunc(OnSpecialKey);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_move);   // Called when mouse moves with a mouse button pressed
	glutKeyboardFunc(key);
	glutIdleFunc(OnIdle);

	cout << "*****************************************************" << endl;
	cout << "*   a moves forward" << endl;
	cout << "*   s moves backward" << endl;
	cout << "*   left arrow key rotates view left" << endl;
	cout << "*   right arrow key rotates view right" << endl;
	cout << "*****************************************************" << endl;


	glutMainLoop();
}