#include <Windows.h>
#include <GL/glut.h>
#include <math.h>
#include "hw6.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <climits>
#include <string>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* strtof */
#include <sstream> 

using namespace std;

/******************************************************************
	Notes:
	Image size is 400 by 400 by default.  You may adjust this if
		you want to.
	You can assume the window will NOT be resized.
	Call clearFramebuffer to clear the entire framebuffer.
	Call setFramebuffer to set a pixel.  This should be the only
		routine you use to set the color (other than clearing the
		entire framebuffer).  drawit() will cause the current
		framebuffer to be displayed.
	As is, your scan conversion should probably be called from
		within the display function.  There is a very short sample
		of code there now.
	You may add code to any of the subroutines here,  You probably
		want to leave the drawit, clearFramebuffer, and
		setFramebuffer commands alone, though.
  *****************************************************************/

float framebuffer[ImageH][ImageW][3];
float zbuffer[ImageH][ImageW];

vector<Face> faces;
vector<Pt> pts;

vector<Face> faces2;
vector<Pt> pts2;
bool paintshape = true;
string filename;
float zoom = 1;
int offset = 0;
float mousex;
float mousey;

bool isRotate = true;
bool action = false;

Pt addMedian(Pt a, Pt b)
{
	float newX = (a.x + b.x) / 2;
	float newY = (a.y + b.y) / 2;
	float newZ = (a.z + b.z) / 2;

	for (int i = 0; i < pts2.size(); i++)
	{
		//cout << i << endl;
		if (pts2[i].equals(newX, newY, newZ))
		{
			return pts2[i];
		}
	}

	Pt p(newX, newY, newZ, pts2.size());
	pts2.push_back(p);
	return p;
}

void warp(Pt &p, float &cx, float &cy, float &cz, int &i, int &j)
{
	Pt center;
	cx = 0;
	cy = 0;
	cz = 0;
	int counter = 0;
	if (!pts[p.index].moved)
	{
		for (j = 0; j < pts[p.index].faces_included.size(); j++)
		{
			center = faces[pts[p.index].faces_included[j]].center;
			cx += center.x;
			cy += center.y;
			cz += center.z;
			counter++;
		}
		cx /= counter;
		cy /= counter;
		cz /= counter;
		pts[p.index].move(cx, cy, cz);
		for (j = 0; j < pts[p.index].faces_included.size(); j++) {
			faces[pts[p.index].faces_included[j]].warp(p.index,cx, cy, cz);
		}
	}
}

void average()
{
	Pt one, two, three, four;
	float cx, cy, cz;
	int i, j;
	for (i = 0; i < faces.size(); i++)
	{
		one   = pts[faces[i].point1.index];
		two   = pts[faces[i].point2.index];
		three = pts[faces[i].point3.index];
		four  = pts[faces[i].point4.index];

		warp(pts[faces[i].point1.index], cx, cy, cz, i, j);
		warp(pts[faces[i].point2.index], cx, cy, cz, i, j);
		warp(pts[faces[i].point3.index], cx, cy, cz, i, j);
		warp(pts[faces[i].point4.index],  cx, cy, cz, i, j);
	}
	for (i = 0; i < faces.size(); i++)
	{
		faces[i].normalize();
	}
}

void divideFace(int p, int first, int second, int third)
{
	//adds the new face to the points' lists that make it up
	pts2[p].add(faces2.size());
	pts2[first].add(faces2.size());
	pts2[second].add(faces2.size());
	pts2[third].add(faces2.size());

	//pushes the new face to the face list
	faces2.push_back(Face(pts2[p], pts2[first], pts2[second], pts2[third]));
}

Pt getCorrectPoint(int pts_index)
{
	//checks to see if the point being created is already on the list
	for (int j = 0; j < pts2.size(); j++)
	{
		//checks to see if the past index of the vertex has already been added to the list
		if (pts2[j].past_index == pts_index)
		{
			return pts2[j];
		}
	}

	//the point was not already on the list, add it
	Pt newP = Pt(pts[pts_index].x, pts[pts_index].y, pts[pts_index].z, pts2.size() , pts_index);
	pts2.push_back(newP);
	return newP;
}

void subDivide()
{
	Pt one, two, three, four, zero, ninety, one80, two70, middle;
	for (int i = 0; i < faces.size(); i++)
	{
		one   = getCorrectPoint(faces[i].point1.index);
		two   = getCorrectPoint(faces[i].point2.index);
		three = getCorrectPoint(faces[i].point3.index);
		four  = getCorrectPoint(faces[i].point4.index);

		zero   = addMedian(one,   four);
		ninety = addMedian(one,   two);
		one80  = addMedian(two,   three);
		two70  = addMedian(three, four);
		middle = addMedian(one,   three);

		divideFace(one.index,   ninety.index, middle.index, zero.index);
		divideFace(two.index,   one80.index,  middle.index, ninety.index);
		divideFace(three.index, two70.index,  middle.index, one80.index);
		divideFace(four.index,  zero.index,   middle.index, two70.index);
		
	}
	faces = faces2;
	pts = pts2;

	faces2.clear();
	pts2.clear();
}

void zooms(float z)
{
	for (int i = 0; i < pts.size(); i++)
	{
		pts[i].mult(z);
	}
	for (int i = 0; i < faces.size(); i++)
	{
		faces[i].refresh(pts[faces[i].point1.index], pts[faces[i].point2.index], pts[faces[i].point3.index], pts[faces[i].point4.index]);
	}
}

void mouseClick(int button, int state, int x, int y)
{
	mousex = x;
	mousey = x;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (button == GLUT_DOWN)
		{
			action = true;
		}
		else
		{
			action = false;
		}
		isRotate = true;
	}
	//used to test different fractals
	if (button == GLUT_RIGHT_BUTTON)
	{
		if (button == GLUT_DOWN)
		{
			action = true;
		}
		else
		{
			action = false;
		}
		isRotate = false;
	}
}

void mouseHold(int x, int y)
{
	if (action)
	{
		if (isRotate)
		{
			//rotate up
				glRotatef(10, 1, 0, 1);
			//rotate down
				glRotatef(10, -1, 0, 1);
			//rotate right
				glRotatef(10, 0, 1, 1);
			//rotate left
				glRotatef(10, 0, -1, 1);
		}
		else
		{
			if (y > mousey + 10)
			{
				mousey = y;
				//scale out
				zooms(.1);
				//zoom *= 1 / .75;


				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glutPostRedisplay();
				glFlush();
			}
			if(y < mousey - 10)
			{
				mousey = y;
				//scale in
				zooms(1.1);
				//zoom *= .75;


				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glutPostRedisplay();
				glFlush();
			}
		}
	}
}

void keyboard(unsigned char key, int x, int y)
{
	paintshape = true;

	switch (key)
	{
	case 'z': //scale in
		zooms(2);
		zoom *= .75;
		break;
	case 'x': //scale out
		zooms(.5);
		zoom *= 1 / .75;
		break;

	case 'c': //rotate clockwise along z
		glRotatef(10, 0, 0, 1);
		break;
	case 'v': //rotate counterclockwise along z
		glRotatef(10, 0, 0, -1);
		break;
	case 'f': //rotate right
		glRotatef(10, 0, 1, 0);
		break;
	case 'g': //rotate left
		glRotatef(10, 0, -1, 0);
		break;
	case 'u': //rotate up
		glRotatef(10, 1, 0, 0);
		break;
	case 'd': //rotate down
		glRotatef(10, -1, 0, 0);
		break;

	case 'l': //subdivide
	case 'L': 
		cout << "subdivide start: please wait" << endl;
		subDivide();
		cout << "subdivide over" << endl;
		break;
	case 'a': //average
	case 'A':
		cout << "average start: please wait" << endl;
		average();
		cout << "average over" << endl;
		break;
	case '+':
		cout << "subdividng and then averaging start: please wait" << endl;
		subDivide();
		average();
		cout << "complete" << endl;
		break;
	case '-':
		for (int i = 0; i < faces.size(); i++)
		{
			faces[i].normal.invert();
		}
		break;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT  );
	glutPostRedisplay();
	glFlush();
}

void SpecialKeys(int key, int x, int y)
{
	paintshape = true;

	switch (key)
	{
		case GLUT_KEY_UP:
			glTranslatef(0, -10*zoom, 0);
			break;
		case GLUT_KEY_DOWN:
			glTranslatef(0, 10*zoom, 0);
			break;
		case GLUT_KEY_RIGHT:
			glTranslatef(10 * zoom, 0, 0);
			break;
		case GLUT_KEY_LEFT:
			glTranslatef(-10 * zoom, 0, 0);
			break;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT  );
	glutPostRedisplay();
	glFlush();
}


void Flat(void)
{
	for (int i = 0; i < faces.size(); i++)
	{
		if (!faces[i].deleted)
		{
			GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.f };
			GLfloat blue[] = { 0.0f, 0.0f, 0.5f, 1.f };
			GLfloat greenish_amb[] = { 0.0f, .5f, 0.05f, 1.f };
			GLfloat greenish[] = { 0.0f, 1.0f, 0.1f, 1.f };
			GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glMaterialfv(GL_FRONT, GL_AMBIENT, greenish_amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, greenish);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, greenish);
			glMaterialfv(GL_FRONT, GL_SPECULAR, white);
			GLfloat shininess[] = { 10 };
			glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

			glBegin(GL_QUADS);
				glNormal3d(faces[i].normal.x, -faces[i].normal.y, faces[i].normal.z);
				glVertex3f(pts[faces[i].point1.index].x, -pts[faces[i].point1.index].y, pts[faces[i].point1.index].z);
				glVertex3f(pts[faces[i].point2.index].x, -pts[faces[i].point2.index].y, pts[faces[i].point2.index].z);
				glVertex3f(pts[faces[i].point3.index].x, -pts[faces[i].point3.index].y, pts[faces[i].point3.index].z);
				glVertex3f(pts[faces[i].point4.index].x, -pts[faces[i].point4.index].y, pts[faces[i].point4.index].z);
			glEnd();

		}
	}
	glFlush();

}

void display(void)
{
	if(paintshape) { Flat(); }
	paintshape = false;
}

void readInput(void)
{
	//filename = "cow.obj";
	string input;
	ifstream file(filename);				//input file name
	bool skip;
	int count = 0;
	while (!file.eof())						//runs until the end of the file is reached
	{						//three variables used to fill in the data members of the student
		getline(file, input);						//attempts to fill in the first variable
		if (file.fail())					//if the input has an error it lets the user know that there is an error and breaks out of the while loop 
		{
			//cout<<"\n There was an error in the input, data not read in, will exit \n\"ERROR\"\n\n";
			break;
		}
		stringstream ss(input);

		if (input[0] == 'v')
		{
			string temp, point1, point2, point3;
			ss >> temp >>point1 >> point2 >> point3;
			Pt point(atof(point1.c_str()), atof(point2.c_str()), atof(point3.c_str()), pts.size());
			//cout << input<< endl;
			//cout << "test: "<< atof(point1.c_str()) <<" "<< point1 << endl;
			pts.push_back(point);
		}
		else if (input[0] == 'f')
		{
			string temp, point1, point2, point3, point4;
			ss >> temp >> point1 >> point2 >> point3 >> point4;
			//cout << input << endl;
			//cout << "New Face: " << point1 << " " << point2 << " " << point3 << " " << endl;
			pts[stoi(point1) - 1].add(count);
			pts[stoi(point2) - 1].add(count);
			pts[stoi(point3) - 1].add(count);
			pts[stoi(point4) - 1].add(count);
			Face f(pts[stoi(point1) - 1], pts[stoi(point2) - 1], pts[stoi(point3) - 1], pts[stoi(point4) - 1]);
			faces.push_back(f);
			count++;
		}
	}
	file.close();			//closes the file
}

void init(void) {
	readInput();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2000, 2000, 2000, -2000, -150.0, 150.0);
	glMatrixMode(GL_MODELVIEW);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	//////
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Create light components.
	GLfloat ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat diffuseLight[] = { 0.0f, 0.8f, 0.0, 1.0f };
	GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat position[] = { -9000.0f, -9000.0f, -9000.0f, 1.0f };

	// Assign created components to GL_LIGHT0.
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	//////

	//get correct size
	glScalef(28, 28, 1.0f);
	glScalef(2, 2, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	display();
}


int main(int argc, char* argv[])
{
	//filename = argv[1];
	filename = "monsterfrog.obj";
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Inaki Rosa - Homework 6");
	init();

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseHold);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(SpecialKeys);
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}
