#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

//	Author:			Chelesa Li

// title of these windows:

const char *WINDOWTITLE = { "Project 6: Geometric Modeling -- Chelsea Li" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:

const int ESCAPE = { 0x1b };

// initial window size:

const int INIT_WINDOW_SIZE = { 600 };

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:

const float MINSCALE = { 0.05f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// non-constant global variables:

int		ActiveButton;			// current button that is down
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
float	Time;					// timer in the range [0.,1.)
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
bool	Frozen;					// checking if the animation should freeze

// function prototypes:

void	Animate( );
void	Display( );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

unsigned char *	BmpToTexture( char *, int *, int * );
void			HsvRgb( float[3], float [3] );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);

bool pointsOn = true;
bool lineOn = true;
int NUMPOINTS = 10;
int NUMCURVES = 5;

struct Point
{
        float x0, y0, z0;       // initial coordinates
        float x,  y,  z;        // animated coordinates
};

struct Curve
{
        float r, g, b;
        Point p0, p1, p2, p3;
};

Curve Curves[5];		// if you are creating a pattern of curves
Curve Stem;				// if you are not

// Rotating a Point an Angle about the X Axis Around a Center

void
RotateX( Point *p, float deg, float xc, float yc, float zc )
{
        float rad = deg * (M_PI/180.f);         // radians
        float x = p->x0 - xc;
        float y = p->y0 - yc;
        float z = p->z0 - zc;

        float xp = x;
        float yp = y*cos(rad) - z*sin(rad);
        float zp = y*sin(rad) + z*cos(rad);

        p->x = xp + xc;
        p->y = yp + yc;
        p->z = zp + zc;
}

// Rotating a Point an Angle about the Y Axis Around a Center

void
RotateY( Point *p, float deg, float xc, float yc, float zc )
{
        float rad = deg * (M_PI/180.f);         // radians
        float x = p->x0 - xc;
        float y = p->y0 - yc;
        float z = p->z0 - zc;

        float xp =  x*cos(rad) + z*sin(rad);
        float yp =  y;
        float zp = -x*sin(rad) + z*cos(rad);

        p->x = xp + xc;
        p->y = yp + yc;
        p->z = zp + zc;
}

// Rotating a Point an Angle about the Z Axis Around a Center

void
RotateZ( Point *p, float deg, float xc, float yc, float zc )
{
        float rad = deg * (M_PI/180.f);         // radians
        float x = p->x0 - xc;
        float y = p->y0 - yc;
        float z = p->z0 - zc;

        float xp = x*cos(rad) - y*sin(rad);
        float yp = x*sin(rad) + y*cos(rad);
        float zp = z;

        p->x = xp + xc;
        p->y = yp + yc;
        p->z = zp + zc;
}

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// init all the global variables used by Display( ):

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	const int MS_IN_THE_ANIMATION_CYCLE = 10000;	// milliseconds in the animation loop
	int ms = glutGet(GLUT_ELAPSED_TIME);			// milliseconds since the program started
	ms %= MS_IN_THE_ANIMATION_CYCLE;				// milliseconds in the range 0 to MS_IN_THE_ANIMATION_CYCLE-1
	Time = ((float)ms  /  (float)MS_IN_THE_ANIMATION_CYCLE);        // [ 0., 1. )

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );

	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );

	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );

	// drawing the stationary curve which will be the body of the dragonfly

	glPushMatrix();
	// setting up the points
	struct Point pt0 = { 1.5, 0., 0. };
	struct Point pt1 = { 0.25, 0.35, 0.2 };
	struct Point pt2 = { 0.5, 0.35, 0.2 };
	struct Point pt3 = { 0., 0., 0. };

	RotateX(&pt0, 180.0, 0.0, 0.0, 0.0);
	RotateY(&pt1, 180.0, 0.0, 0.0, 0.0);
	RotateZ(&pt2, 180.0, 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	// draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 10. );
	glColor3f( 1., 1., 1. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// drawing the animated curves which will be the wings of the dragonfly

	// drawing the first wing

	glPushMatrix();	
	// setting up the points
	pt0 = { 0., 0., (float) (sin(Time)*10) };
	pt1 = { 0.25, 1., (float) (1.+ (sin(Time)*10)) };
	pt2 = { 0.5, 1., (float) (1. + (sin(Time)*10)) };
	pt3 = { 0., 0., (float) (sin(Time)*10) };

	RotateX(&pt1, 100.0 + (sin(Time)*100), 0.0, 0.0, 0.0);
	RotateX(&pt2, 100.0 + (sin(Time)*100), 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	// draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 3. );
	glColor3f( 1., 0., 0. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// drawing the second wing

	glPushMatrix();	
	// setting up the points
	pt0 = { 0., 0., (float) (sin(Time)*10) };
	pt1 = { -0.25, 1., (float) (1. + (sin(Time)*10)) };
	pt2 = { -0.5, 1., (float) (1. + (sin(Time)*10)) };
	pt3 = { 0., 0., (float) (sin(Time)*10) };

	RotateX(&pt1, 100.0 + (sin(Time)*100), 0.0, 0.0, 0.0);
	RotateX(&pt2, 100.0 + (sin(Time)*100), 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	// draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 3. );
	glColor3f( 0., 1., 0. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// drawing the third wing

	glPushMatrix();	
	// setting up the points
	pt0 = { 0., 0., (float) (sin(Time)*10) };
	pt1 = { 0, 1., (float) (1. + (sin(Time)*10)) };
	pt2 = { 0.25, 1., (float) (1. + (sin(Time)*10)) };
	pt3 = { 0., 0., (float) (sin(Time)*10) };

	RotateX(&pt1, 100.0 + (sin(Time)*100), 0.0, 0.0, 0.0);
	RotateX(&pt2, 100.0 + (sin(Time)*100), 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	// draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 3. );
	glColor3f( 0., 0., 1. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// drawing the fourth wing

	glPushMatrix();	
	// setting up the points
	pt0 = { 0., 0., (float) (sin(Time)*10) };
	pt1 = { 0.25, -1., (float) (-1. - (sin(Time)*10)) };
	pt2 = { 0.5, -1., (float) (-1. - (sin(Time)*10)) };
	pt3 = { 0., 0., (float) (sin(Time)*10) };

	RotateX(&pt1, -130.0 - (sin(Time)*100), 0.0, 0.0, 0.0);
	RotateX(&pt2, -130.0 - (sin(Time)*100), 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	// draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 3. );
	glColor3f( 1., 1., 0. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// drawing the fifth curve

	glPushMatrix();	
	// setting up the points
	pt0 = { 0., 0., (float) (sin(Time)*10) };
	pt1 = { -0.25, -1., (float) (-1. - (sin(Time)*10)) };
	pt2 = { -0.5, -1., (float) (-1. - (sin(Time)*10)) };
	pt3 = { 0., 0., (float) (sin(Time)*10) };

	RotateX(&pt1, -130.0 - (sin(Time)*100), 0.0, 0.0, 0.0);
	RotateX(&pt2, -130.0 - (sin(Time)*100), 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	// draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 3. );
	glColor3f( 0., 1., 1. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// drawing the sixth curve

	glPushMatrix();	
	// setting up the points
	pt0 = { 0., 0., (float) (sin(Time)*10) };
	pt1 = { 0, -1., (float) (-1. - (sin(Time)*10)) };
	pt2 = { 0.25, -1., (float) (-1. - (sin(Time)*10)) };
	pt3 = { 0., 0., (float) (sin(Time)*10) };

	RotateX(&pt1, -130.0 - (sin(Time)*100), 0.0, 0.0, 0.0);
	RotateX(&pt2, -130.0 - (sin(Time)*100), 0.0, 0.0, 0.0);

	// draw the points for the curve

	if(pointsOn)
	{
		glPointSize( 5.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_POINTS );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd( );
	}

	//draw the lines for the curve

	if(lineOn)
	{
		glLineWidth( 2.0 );
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINE_LOOP );
			glVertex3f( pt0.x, pt0.y, pt0.z );
			glVertex3f( pt1.x, pt1.y, pt1.z );
			glVertex3f( pt2.x, pt2.y, pt2.z );
			glVertex3f( pt3.x, pt3.y, pt3.z );
		glEnd();
	}

	// Drawing a Bézier Curve

	glLineWidth( 3. );
	glColor3f( 1., 0., 1. );
	glBegin( GL_LINE_STRIP );
	for( int it = 0; it <= NUMPOINTS; it++ )
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt*omt*omt*pt0.x + 3.f*t*omt*omt*pt1.x + 3.f*t*t*omt*pt2.x + t*t*t*pt3.x;
		float y = omt*omt*omt*pt0.y + 3.f*t*omt*omt*pt1.y + 3.f*t*t*omt*pt2.y + t*t*t*pt3.y;
		float z = omt*omt*omt*pt0.z + 3.f*t*omt*omt*pt1.z + 3.f*t*t*omt*pt2.z + t*t*t*pt3.z;
		glVertex3f( x, y, z );
	}
	glEnd( );
	glLineWidth( 1. );
	glPopMatrix();

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}

// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:
	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:
	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	// main menu

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{	
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
}

// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'p': // turn on/off the points for the curves
			pointsOn = !pointsOn;
			break;
		case 'l': // turn on/off the lines for the curves
			lineOn = !lineOn;
			break;
		case 'f': // freeze everything
			Frozen = ! Frozen;
			if( Frozen )
				glutIdleFunc( NULL );
			else
				glutIdleFunc( Animate );
			break;
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	fprintf( stderr, "MouseMotion: %d, %d\n", x, y );

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	Scale  = 1.0;
	Xrot = Yrot = 0.;
	pointsOn = true;
	lineOn = true;
	Frozen = false;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since the window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// read a BMP file into a Texture:

#define VERBOSE		false
#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB			0
#define BI_RLE8			1
#define BI_RLE4			2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;

// read a BMP file into a Texture:

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE *fp;
#ifdef _WIN32
        errno_t err = fopen_s( &fp, filename, "rb" );
        if( err != 0 )
        {
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
        }
#else
	FILE *fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort( fp );

	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if( VERBOSE ) fprintf( stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
			FileHeader.bfType, FileHeader.bfType&0xff, (FileHeader.bfType>>8)&0xff );
	if( FileHeader.bfType != BMP_MAGIC_NUMBER )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}

	FileHeader.bfSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize );

	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );

	FileHeader.bfOffBytes = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfOffBytes = %d\n", FileHeader.bfOffBytes );

	InfoHeader.biSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSize = %d\n", InfoHeader.biSize );
	InfoHeader.biWidth = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biWidth = %d\n", InfoHeader.biWidth );
	InfoHeader.biHeight = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biHeight = %d\n", InfoHeader.biHeight );

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biPlanes = %d\n", InfoHeader.biPlanes );

	InfoHeader.biBitCount = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount );

	InfoHeader.biCompression = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression );

	InfoHeader.biSizeImage = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage );

	InfoHeader.biXPixelsPerMeter = ReadInt( fp );
	InfoHeader.biYPixelsPerMeter = ReadInt( fp );

	InfoHeader.biClrUsed = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed );

	InfoHeader.biClrImportant = ReadInt( fp );

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char *texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\n" );
		return NULL;
	}

	// extra padding bytes:

	int requiredRowSizeInBytes = 4 * ( ( InfoHeader.biBitCount*InfoHeader.biWidth + 31 ) / 32 );
	if( VERBOSE )	fprintf( stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes );

	int myRowSizeInBytes = ( InfoHeader.biBitCount*InfoHeader.biWidth + 7 ) / 8;
	if( VERBOSE )	fprintf( stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes );

	int oldNumExtra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;
	if( VERBOSE )	fprintf( stderr, "Old NumExtra padding = %d\n", oldNumExtra );

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if( VERBOSE )	fprintf( stderr, "New NumExtra padding = %d\n", numExtra );

	// this function does not support compression:

	if( InfoHeader.biCompression != 0 )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}
	
	// we can handle 24 bits of direct color:
	if( InfoHeader.biBitCount == 24 )
	{
		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if( InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256 )
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32 *colorTable = new struct rgba32[ InfoHeader.biClrUsed ];

		rewind( fp );
		fseek( fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET );
		for( int c = 0; c < InfoHeader.biClrUsed; c++ )
		{
			colorTable[c].r = fgetc( fp );
			colorTable[c].g = fgetc( fp );
			colorTable[c].b = fgetc( fp );
			colorTable[c].a = fgetc( fp );
			if( VERBOSE )	fprintf( stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a );
		}

		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				int index = fgetc( fp );
				*(tp+0) = colorTable[index].r;	// r
				*(tp+1) = colorTable[index].g;	// g
				*(tp+2) = colorTable[index].b;	// b
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}

		delete[ ] colorTable;
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	const unsigned char b2 = fgetc( fp );
	const unsigned char b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short
ReadShort( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}
