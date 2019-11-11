#include "stdafx.h"

#include <string>
#include <iostream>

#include "ShaderFunctions.h"
#include "vec2.h"
#include "vec3.h"
#include "Scene.h"

#include "windows.h"

#define VOLUME_XY 0
#define LIGHTS 1

// used for mouse-controlled rotation
vec3 g_rotation = vec3(0),light_rotation =vec3(0),rotation_scaled=vec3(0);
vec3 position=vec3(0,0,0);
vec3 eye=vec3(0,0,3);
vec2i g_window_size;
float threshold=0.20,div_coef=0.6,step=255.0;

GLfloat light_pos[] = {0.0f, 0.0f,-10.0f};
GLfloat diffuse[] = {1.0f, 1.0f, 1.0f , 1.0f};
GLfloat specular[] = {1.0f, 1.0f, 1.0f , 1.0f};

//chrome material properties
float shininess=1.0;//76.8;
float bubble_scale=32;
GLfloat diffuse_material[] = {0.40f,0.40f,0.40f,1.0f};
GLfloat specular_material[] = {0.77f,0.77f,0.77f,1.0f};

// GLUT callbacks and similar functions
void init();
void init_callbacks();
void display();
void reshape(int, int);
void keyboard(unsigned char, int, int);
void special_keyboard(int key, int x, int y);
void mouse(int, int, int, int);
void motion(int, int);
bool light=FALSE;
GLuint fbo_front = 0;
GLuint front_tex = 0;
GLuint fbo_back = 0;
GLuint back_tex = 0;
int window_width = 400;
int window_height = 400;
bool framebuffer=TRUE;
int projection=0;
int rotate=VOLUME_XY;
char s[30],buf[30];
int frame,time,timebase=0;
int font=(int)GLUT_BITMAP_8_BY_13;
static vec2i g_last_mousepos,g_last_lightpos;
static int btn;
int pressed=0;

// scene object - the interesting stuff
Scene *g_scene;

int main(int argc, char *argv[])
{
try
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(window_width,window_height);
	glutCreateWindow("Volume Renderer");
	init_callbacks();

	// terminate if OpenGL 2.0 shaders are not supported
	if (!glCreateProgram)
		throw std::runtime_error("the graphics card or driver "
			"does not support the OpenGL Shading Language");

	init();

	glutMainLoop();
}
catch (std::exception &e) {
	std::clog << "exception: " << e.what() << std::endl;
	MessageBoxA(0, e.what(), "Error", MB_OK | MB_ICONERROR);
	return 1;
}
}

void init_callbacks()
{
	// create window

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special_keyboard);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);

	// load OpenGL extensions and functions not part of OpenGL 1.1
	glewInit();
}

void setOrthographicProjection() {

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	// save previous matrix which contains the 
	//settings for the perspective projection
	glPushMatrix();
	// reset matrix
	glLoadIdentity();
	// set a 2D orthographic projection
	gluOrtho2D(0, window_width, 0, window_height);
	// invert the y axis, down is positive
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner
	// to the upper left corner
	glTranslatef(0, -window_height, 0);
	glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection() {
	// set the current matrix to GL_PROJECTION
	glMatrixMode(GL_PROJECTION);
	// restore previous settings
	glPopMatrix();
	// get back to GL_MODELVIEW matrix
	glMatrixMode(GL_MODELVIEW);
}

void renderBitmapString(float x, float y, void *font,char *string)
{
  
  char *c;
  // set position to start drawing fonts
  glRasterPos2f(x, y);
  // loop all the characters in the string
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}


void checkFrameBufferStatus() {
  GLenum status;
  status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  
  std::cerr << "Checking framebuffer object..." << std::endl;
  
  switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      std::cerr << "FBO complete!" << std::endl;
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      std::cerr << "Error! FBO has no images/buffers attached!" << std::endl;
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      std::cerr << "Error! FBO has mismatched image/buffer dimensions!" << std::endl;
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      std::cerr << "Error! FBO's colorbuffer attachments have different types!" << std::endl;
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      std::cerr << "Error! FBO trying to draw to non-attached color buffer!" << std::endl;
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      std::cerr << "Error! FBO trying to read from a non-attached color buffer!" << std::endl;
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      std::cerr << "Error! FBO format is not supported by current graphics card/driver!" << std::endl;
      break;
    default:
      std::cerr << "Error! Unknown error reported from glCheckFramebufferStatusEXT()" << std::endl;
      break;
    }
  
}

GLuint createFloatTexCoordTexture() {
  
	std::cerr << "Creating texcoord texture" << std::endl;
  
  // create and bind texture object
  const GLenum target = GL_TEXTURE_2D;
  GLuint name;
  glGenTextures(1, &name);
  glBindTexture(target, name);
  
  // set texture parameters
  // (clamp to edge in order to get non-power of two support on ATI cards)
  glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glTexImage2D(target,          // target
	       0,               // level
	       GL_RGBA,         // comp
	       window_width,
	       window_height,
	       0,               // border
	       GL_RGBA,         // format
	       GL_FLOAT,        // type
	       0);              // data
  
  std::cerr << "Finished!" << std::endl;
  return name;
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_COLOR_MATERIAL);

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  front_tex = createFloatTexCoordTexture();
  glGenFramebuffersEXT(1, &fbo_front);
  
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_front);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
  			    GL_COLOR_ATTACHMENT0_EXT,
  			    GL_TEXTURE_2D, front_tex, 0);
  
  checkFrameBufferStatus();

  back_tex = createFloatTexCoordTexture();
  glGenFramebuffersEXT(1, &fbo_back);
  
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_back);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
  			    GL_COLOR_ATTACHMENT0_EXT,
  			    GL_TEXTURE_2D, back_tex, 0);
  
  checkFrameBufferStatus();
  
  glBindTexture(GL_TEXTURE_2D, 0);  
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);  

	g_scene = new Scene;
int max;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max);
	std::cerr<<std::endl<<"max:"<<max<<std::endl;

}

void display()
{

	glLoadIdentity();
	glShadeModel(GL_SMOOTH);	
	glEnable(GL_DEPTH_TEST);						// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);	
	//glPushMatrix();

	gluLookAt(eye.x,eye.y,eye.z, 0,0,-3.0, 0,1,0);
	glTranslatef(position.x,position.y,position.z);
	if(rotate==LIGHTS)
	{
		glPushMatrix();
			glRotatef(light_rotation.x*180/3.14f, 1,0,0);
			glRotatef(light_rotation.y*180/3.14f, 0,1,0);
			glRotatef(light_rotation.z*180/3.14f, 0,0,1);
			glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glPopMatrix();
	}
	//if(rotate==VOLUME_XY)
	rotation_scaled.x=g_rotation.x*180/3.14f;
	rotation_scaled.y=g_rotation.y*180/3.14f;
	rotation_scaled.z=g_rotation.z*180/3.14f;

	{
		glRotatef(rotation_scaled.x, 1,0,0);
		glRotatef(rotation_scaled.y, 0,1,0);
		glRotatef(rotation_scaled.z, 0,0,1);
	}

	glClearColor(0.0,0.0,0.0,0);
	//glClearColor(0.0,0.0,0.0,0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_material);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, specular_material);

	// call the main drawing function
	g_scene->draw(threshold,div_coef,framebuffer,fbo_front,fbo_back,
		front_tex,back_tex,light,projection,step,eye,shininess,
		rotation_scaled,pressed,bubble_scale);
pressed=0;

	frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 100) {
		sprintf(s,"FPS:%4.2f",frame*1000.0/(time-timebase));
		timebase = time;		
		frame = 0;
	}
	glUseProgram(0);
	glColor3f(1.0f,1.0f,1.0f);
	setOrthographicProjection();
	glPushMatrix();
	glLoadIdentity();
	renderBitmapString(10,15,(void *)font,s);
//	renderBitmapString(10,35,(void *)font,"Esc - Quit");
	sprintf(buf,"div_coef:%f",div_coef);
	renderBitmapString(10,35,(void *)font,buf);
	sprintf(buf,"threshold:%f",threshold);
	renderBitmapString(10,55,(void *)font,buf);
	glPopMatrix();
	resetPerspectiveProjection();

	glutSwapBuffers();
	glutPostRedisplay();



	if (glGetError())
		std::clog << "OpenGL error" << std::endl;
}

void keyboard(unsigned char k, int, int)
{
	if (k == 27)std::exit(0);
	else if (k == 'f')
	{
		/*glutGameModeString("640x480:32");
		// enter full screen
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) 
		{
			glutEnterGameMode();
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
			init_callbacks();
			init();
		}
		else 
			{
			printf("The select mode is not available\n");
			}*/
		glutFullScreen();
	}

	else if	(k=='v')
	{
		/*glutLeaveGameMode();//glutReshapeWindow(window_width/2,window_height/2);
			glutInitWindowSize(window_width,window_height);
	glutCreateWindow("Volume Renderer");
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
			init_callbacks();
			init();*/
	}
	else if (k=='l')light = !light;
	else if (k=='p')projection=(projection+1)%3;
	else if (k=='b')framebuffer = !framebuffer;
	else if (k == 'd')step+=10;
	else if (k == 'c')step-=10;
	else if (k == 'q')position.z+=0.1;
	else if (k == 'w')position.z-=0.1;
	else if (k == 'h')shininess*=1.1;
	else if (k == 'n')shininess/=1.1;
	else if (k == 'a')threshold+=0.01;
	else if (k == 'z')threshold-=0.01;
	else if (k == 's')div_coef*=1.1;
	else if (k == 'x')div_coef/=1.1;
	else if (k == 'j')bubble_scale*=1.2;
	else if (k == 'm')bubble_scale/=1.2;
}

void special_keyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)position.y-=0.1;
	else if (key == GLUT_KEY_DOWN)position.y+=0.1;
	else if (key == GLUT_KEY_RIGHT)position.x-=0.1;
	else if (key == GLUT_KEY_LEFT)position.x+=0.1;
}

void reshape(int w, int h)
{
  // update our variables
  window_width = w;
  window_height = h;
  
  // update fbo texture
  glBindTexture(GL_TEXTURE_2D, front_tex);
  glTexImage2D(GL_TEXTURE_2D, // target
	       0,             // level
	       GL_RGBA,       // comp
	       w,
	       h,
	       0,             // border
	       GL_RGBA,       // format
	       GL_FLOAT,      // type
	       0);            // data

  glBindTexture(GL_TEXTURE_2D, back_tex);
  glTexImage2D(GL_TEXTURE_2D, // target
	       0,             // level
	       GL_RGBA,       // comp
	       w,
	       h,
	       0,             // border
	       GL_RGBA,       // format
	       GL_FLOAT,      // type
	       0);            // data

	g_window_size = vec2i(w,h);

	glViewport(0,0, w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(30, float(w)/h, 0.1, 10000);
	glMatrixMode(GL_MODELVIEW);

	g_scene->resize(g_window_size);
}

void mouse(int button, int state, int x, int y)
{
	btn=button;
	//if(state=GLUT_UP)glutPostRedisplay();
	if(btn==GLUT_LEFT_BUTTON)g_last_mousepos = vec2i(x,y);
	if(btn==GLUT_RIGHT_BUTTON)g_last_lightpos = vec2i(x,y);
}
void motion(int x, int y)
{
	vec2i pos = vec2i(x,y);
	vec2i delta;
	//g_last_mousepos = pos;
pressed=0;
	
	if(btn==GLUT_LEFT_BUTTON)
	{
		delta = pos - g_last_mousepos;
		pressed=1;
		g_last_mousepos = pos;
		rotate=VOLUME_XY;
		g_rotation.y += 3*float(delta.x) / g_window_size.x;
		g_rotation.x += 3*float(delta.y) / g_window_size.y;
	}
	if(btn==GLUT_RIGHT_BUTTON)
	{
		delta = pos - g_last_lightpos;
		g_last_lightpos = pos;
		rotate=LIGHTS;
		light_rotation.y += 3*float(delta.x) / g_window_size.x;
		light_rotation.x += 3*float(delta.y) / g_window_size.y;
	}
	if(btn==GLUT_MIDDLE_BUTTON)
	{
		delta = pos - g_last_lightpos;
		g_last_lightpos = pos;
		rotate=VOLUME_XY;
		g_rotation.z += 3*float(delta.x) / g_window_size.x;
	}
	//glutPostRedisplay();
}