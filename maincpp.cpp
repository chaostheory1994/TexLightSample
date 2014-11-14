//#define USE_ORIGINAL
#ifndef USE_ORIGINAL
#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "Block.h"

#define min(a,b) ((a) < (b)? a:b)
#define FALSE 0 
#define TRUE  1

#define COUNTERCLOCKWISE  1
#define CLOCKWISE -1

#define DELTA_TIME 100
#define DELTA_DEG  5

int   light1_theta = 0;
float zoom = 1.0;

GLfloat vertices [][3] = {
		{ -1, 1, 1 }, { 1, 1, 1 }, { 1, 1, -1 }, { -1, 1, -1 },
		{ -1, -1, 1 }, { 1, -1, 1 }, { 1, -1, -1 }, { -1, -1, -1 }
};
/* The Light source struct. Everything you would need to know about a light setup in a linked list. */
struct LightList {
	float pos[4];
	float lookat[4];
	float angle = 0;
	float amb[4];
	float diff[4];
	float spec[4];
	LightList* next;
};
/* This is the flashlight the player can carry. */
/* pos is the position in world coordinates */
/* look at is the position on the unit circle around the position at the direciton it is facing. 
 * Meaning the limits on the x, z coordinate is [-1, 1]. Acts like an offset rather than a world point. */
struct FlashLight {
	float pos[3];
	float angle;
};

void glut_setup(void);
void gl_setup(void);
int my_setup(int, char**);
void lighting_setup();
void my_display(void);
void my_reshape(int w, int h);
void my_keyboard(unsigned char key, int x, int y);
LightList* create_light();
void delete_light();
void print_light(LightList*);
void setup_flashlight();
void forward();
void backward();
void left();
void right();

Block** world;
bool isFirstPerson;
FlashLight fl;
LightList* first;

int main(int argc, char **argv) {

	/* General initialization for GLUT and OpenGL
	Must be called first */
	glutInit(&argc, argv);

	/* we define these setup procedures */
	glut_setup();
	gl_setup();
	if (my_setup(argc, argv)) return -1;

	/* go into the main event loop */
	glutMainLoop();

	return(0);
}

/* This function sets up the windowing related glut calls */
void glut_setup(void) {

	/* specify display mode -- here we ask for a double buffer and RGB coloring */
	/* NEW: tell display we care about depth now */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	/* make a 400x400 window with the title of "GLUT Skeleton" placed at the top left corner */
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("LightTexSample");

	/*initialize callback functions */
	glutDisplayFunc(my_display);
	glutReshapeFunc(my_reshape);
	glutKeyboardFunc(my_keyboard);
	return;
}

/* This function sets up the initial states of OpenGL related enivornment */
void gl_setup(void) {

	/* specifies a background color: black in this case */
	glClearColor(0, 0, 0, 0);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1.0, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* NEW: lighting stuff */
	glShadeModel(GL_SMOOTH);
	return;
}

int my_setup(int argc, char** argv) {
	/* We will first start by parsing the file in the argument. */
	FILE* map;
	char* fn = "hw4.spec.txt";
	char curr;
	char* buffer = new char[50];
	int curr_i;
	int temp;
	float curr_f[4];
	int i;
	LightList* llptr;
	isFirstPerson = false;
	bool world_iscreated[16];
	// To make sure that we create4 all blocks including ones not mentioned in the file
	// I need to keep track of which i've created so after i can finish the rest with height = 0;
	for (i = 0; i < 16; i++) world_iscreated[i] = false;
	// Make sure first is null.
	first = NULL;
	// Was an arguement provided with a filename?
	if (argc > 1) fn = argv[1];
	// Open the file.
	if ((map = fopen(fn, "r")) == NULL){
		printf("File could not be found.\n Exiting Program.\n");
		return -1;
	}
	// Lets initiailize the block arrays.
	world = new Block*[16];
	// At this point, the file is open and found.
	// Now we need to read it.
	while ((curr = fgetc(map)) != EOF){
		// There are four options for the first letter. b, l, t, #.
		if (curr == 'b'){
			// Looks like a block is being defined.
			// The first thing we have to read in is the id.
			fscanf(map, "%d", &curr_i);
			// And next the height;
			fscanf(map, "%d", &temp);
			// Now we setup the block object with those two parameters.
			world[curr_i] = new Block(curr_i, temp);
			world_iscreated[curr_i] = true;
			// At this point we need to setup the rest of the object. 
			// The next thing that MIGHT be there is the texture id. lets see.
			fscanf(map, " %c", &curr);
			if (curr == '['){
				// There is a texture id next.
				fscanf(map, "%d", &temp);
				if (world[curr_i]->set_texture(temp)) printf("WARNING: Texture %d is not defined for block %d", temp, curr_i);
				// Now to catch up to the else lets read the next int for shininess!
				fscanf(map, "%*c %d", &temp);
			}
			else{
				// This means there wasnt a bracket next and as such there wasnt a texture id and we actually read in the shininess.
				// Lets fix this.
				// Also we only read the largest digit.
				// first adjust ascii to int.
				temp = curr - 48;
				do{
					fscanf(map, "%c", &curr);
					if (curr != ' '){
						temp *= 10;
						temp += (curr - 48);
					}
					else break;
				} while (curr != ' ');
			}
			// Now lets set the shininess.
			world[curr_i]->set_shininess(temp);

			// Finally is there the 4 optional light intensities.
			fscanf(map, " %c", &curr);
			if (curr == '('){
				// This means the 4 intensities are there.
				// Lets read them.
				// First up is the emission
				fscanf(map, "%f", &curr_f[0]);
				fscanf(map, "%f", &curr_f[1]);
				fscanf(map, "%f", &curr_f[2]);
				fscanf(map, "%f %*c", &curr_f[3]);
				world[curr_i]->set_emission(curr_f[0], curr_f[1], curr_f[2], curr_f[3]);

				// Similar stuff for the other 3.
				// ambient
				fscanf(map, " %*c %f", &curr_f[0]);
				fscanf(map, "%f", &curr_f[1]);
				fscanf(map, "%f", &curr_f[2]);
				fscanf(map, "%f %*c", &curr_f[3]);
				world[curr_i]->set_ambient(curr_f[0], curr_f[1], curr_f[2], curr_f[3]);

				// diffuse
				fscanf(map, " %*c %f", &curr_f[0]);
				fscanf(map, "%f", &curr_f[1]);
				fscanf(map, "%f", &curr_f[2]);
				fscanf(map, "%f %*c", &curr_f[3]);
				world[curr_i]->set_diffusion(curr_f[0], curr_f[1], curr_f[2], curr_f[3]);
				
				//specular
				fscanf(map, " %*c %f", &curr_f[0]);
				fscanf(map, "%f", &curr_f[1]);
				fscanf(map, "%f", &curr_f[2]);
				fscanf(map, "%f %*c", &curr_f[3]);
				world[curr_i]->set_specular(curr_f[0], curr_f[1], curr_f[2], curr_f[3]);
			}
			// All done with reading in a block.
			world[curr_i]->print();
		}
		else if (curr == 'l'){
			// A light is being created!
			// Lets first create a light.
			llptr = create_light();
			// Were we able to create the light?
			if (llptr == NULL){
				printf("Was Not Able to create a Light");
				break;
			}
			// Lets read the ambient light.
			fscanf(map, " %*c");
			for (i = 0; i < 4; i++) fscanf(map, "%f", &(llptr->amb[i]));
			fscanf(map, " %*c");

			// diffuse
			fscanf(map, " %*c");
			for (i = 0; i < 4; i++) fscanf(map, "%f", &(llptr->diff[i]));
			fscanf(map, " %*c");

			// specular
			fscanf(map, " %*c");
			for (i = 0; i < 4; i++) fscanf(map, "%f", &(llptr->spec[i]));
			fscanf(map, " %*c");

			// Read Position
			fscanf(map, " %*c");
			for (i = 0; i < 3; i++) fscanf(map, "%d", &(llptr->pos[i]));
			llptr->pos[3] = 1.0f;
			fscanf(map, " %*c");

			// Now for the option lookat
			fscanf(map, " %c", &curr);
			if (curr == '['){
				// There is lookat stuff
				llptr->pos[3] = 0.0f;
				fscanf(map, " %*c");
				for (i = 0; i < 3; i++) fscanf(map, "%d", &(llptr->lookat[i]));
				fscanf(map, " %*c");
				fscanf(map, "%d %*c", &(llptr->angle));
			}
			else{
				for (i = 0; i < 3; i++) llptr->lookat[i] = 0;
				llptr->angle = 0;
			}
			print_light(llptr);
		}
		else if (curr == 't'){
			// A texture is being added!
			// This is just reading 3 ints and a string.
			fscanf(map, "%d", &curr_i);
			fscanf(map, "%d", &i);
			fscanf(map, "%d", &temp);
			fscanf(map, "%s", buffer);
			Block::add_texture(curr_i, i, temp, buffer);
			Block::print_text(curr_i);
		}
		else if (curr == '#'){
			while (curr != '\n' && curr != EOF) curr = fgetc(map);
		}
	}
	delete[] buffer;
	fclose(map);
	Block::confirm_textures();
	// Time to setup all the rest of the blocks.
	for (i = 0; i < 16; i++){
		if (!world_iscreated[i]) world[i] = new Block(i, 0);
	}
	lighting_setup();
	//setup_flashlight();
	return 0;
}

void setup_flashlight(){
	/* FlashLight Properties */
	GLfloat flash_amb [] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat flash_diffuse [] = { 1, 1, 1, 1.0 };
	GLfloat flash_specular [] = { 1, 1, 1, 1 };

	glEnable(GL_LIGHT7);
	// Setup flashlight position and lookat initial points.
	fl.pos[0] = 40;
	fl.pos[1] = 2;
	fl.pos[2] = 40;
	fl.angle = -90;

	// OpenGl calls for flash light lighting
	glLightfv(GL_LIGHT7, GL_AMBIENT, flash_amb);
	glLightfv(GL_LIGHT7, GL_DIFFUSE, flash_diffuse);
	glLightfv(GL_LIGHT7, GL_SPECULAR, flash_specular);
}

void my_reshape(int w, int h) {

	/* define viewport -- x, y, (origin is at lower left corner) width, height */
	glViewport(0, 0, min(w, h), min(w, h));
	return;
}

void lighting_setup() {
	int i;
	LightList* curr = first;
	/*
	GLfloat light0_amb [] = { 0.2, 0.2, 0.2, 1 };
	GLfloat light0_diffuse [] = { 1, 0, 0, 1 };
	GLfloat light0_specular [] = { 1, 0, 0, 1 };
	GLfloat light0_pos [] = { 1, 1, 1, 0 };

	GLfloat light1_amb [] = { 0.2, 0.2, 0.2, 1 };
	GLfloat light1_diffuse [] = { 0, 1, 0, 1 };
	GLfloat light1_specular [] = { 0, 1, 0, 1 };

	GLfloat globalAmb [] = { .1, .1, .1, 1 };
	*/

	GLfloat globalAmb [] = { .1, .1, .1, 1 };
	//enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	setup_flashlight();
	// Enable the lights being used.
	i = 0;
	while (curr != NULL){
		// Enable Light
		glEnable(GL_LIGHT0 + i);
		// Set the light's stuff.
		glLightfv(GL_LIGHT0 + i, GL_AMBIENT, curr->amb);
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, curr->diff);
		glLightfv(GL_LIGHT0 + i, GL_SPECULAR, curr->spec);
		curr = curr->next;
		i++;
	}

	// reflective propoerites -- global ambiant light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

	
}

void my_keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case 'o':
		break;
	case 'O':
		break;
	case 'l':
		glutPostRedisplay();
		break;
	case 'L':
		glutPostRedisplay();
		break;
	case 'v':
		isFirstPerson = !isFirstPerson;
		glutPostRedisplay();
		break;
	case 'e':
		forward();
		glutPostRedisplay();
		break;
	case 'c':
		backward();
		glutPostRedisplay();
		break;
	case 'f':
		right();
		glutPostRedisplay();
		break;
	case 's':
		left();
		glutPostRedisplay();
		break;
	case 'q':
	case 'Q':
		exit(0);
	default: break;
	}
	return;
}

void my_display(void) {
	int i, j;
	float angle;
	LightList* curr = first;

	GLfloat center_pos[] = { 0, 0, 0, 1 };
	GLfloat lookat_pos [] = { 20.0, 2.0, 20.0 };
	GLUquadric* flashlight_shell;

	/* clear the buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glLightfv(GL_LIGHT7, GL_POSITION, center_pos);
	glPushMatrix();
	
	gluLookAt(20.0, 20.0, 100.0,  // x,y,z coord of t1he camera 
			20.0, 0.0, 20.0,  // x,y,z coord of what we are looking at
			0.0, 1.0, 0.0); // the direction of up 
	
	glLightfv(GL_LIGHT7, GL_POSITION, center_pos);

	

	if (isFirstPerson) {
		glPopMatrix();
		glPushMatrix();
		gluLookAt(fl.pos[0], fl.pos[1], fl.pos[2],
			fl.pos[0] + cos((fl.angle * M_PI) / 180), fl.pos[1], fl.pos[2] + sin((fl.angle * M_PI) / 180),
			0, 1, 0);
	}
	

	// Draw the city.
	glPushMatrix();
	for (i = 0; i < 4; i++){
		glPushMatrix();
		for (j = 0; j < 4; j++){
			world[i * 4 + j]->draw();
			glTranslatef(10.0, 0.0, 0.0);
		}
		glPopMatrix();
		glTranslatef(0.0, 0.0, 10.0);
	}
	glPopMatrix();

	// Draw Lights.
	i = 0;
	//while (curr != NULL){
	//	glPushMatrix();
	//	glRotatef(curr->angle, 0, 1, 0);
	//	glLightfv(GL_LIGHT0 + i, GL_POSITION, curr->pos);
	//	if(curr->pos[3] == 0.0f) glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, curr->lookat);
	//	glPopMatrix();
	//	curr = curr->next;
	//	i++;
	//}

	// Draw Flashlight
	glPushMatrix();
	//glRotatef(90, 1, 0, 0);
	glRotatef(fl.angle, 0, 1, 0);
	glTranslatef(fl.pos[0], fl.pos[1], fl.pos[2]);
	glBegin(GL_POLYGON);
	flashlight_shell = gluNewQuadric();
	gluCylinder(flashlight_shell, 0.5, 0.5, 2, 30, 30);
	glEnd();
	glPopMatrix();
	// Setup actual light
	glPushMatrix();
	/*glTranslatef(-20, -20, -100);
	glTranslatef(fl.pos[0], fl.pos[1], fl.pos[2]);*/
	
	//glLightfv(GL_LIGHT7, GL_SPOT_DIRECTION, lookat_pos);
	glPopMatrix();

	glPopMatrix();
	/* buffer is ready */
	glutSwapBuffers();

	return;
}

/* Simply adds a light to the light linked list */
LightList* create_light(){
	LightList* curr = first;
	int i;
	// Is there current a starting node?
	if (curr == NULL){
		// Then create the new node here.
		curr = new LightList;
		curr->next = NULL;
		first = curr;
	}
	else{
		// Other lights have been create.
		// We need to traverse to the last node
		i = 1;
		while (curr->next != NULL){
			curr = curr->next;
			i++;
		}
		// Now do we have 8 lights already?
		if (i == 7) return NULL;
		// Lets create the new node then
		curr->next = new LightList;
		curr->next->next = NULL;
		curr = curr->next;
		for (i = 0; i < 3; i++) curr->lookat[i] = 0.0;
		glEnable(GL_LIGHT0 + i);
		return curr;
	}
}

/* Debug message to print the current state of the the specified light. */
void print_light(LightList* llptr){
	printf("l (%3.1f %3.1f %3.1f %3.1f) (%3.1f %3.1f %3.1f %3.1f) (%3.1f %3.1f %3.1f %3.1f) (%d %d %d) [(%d %d %d) %d]\n",
		llptr->amb[0], llptr->amb[1], llptr->amb[2], llptr->amb[3],
		llptr->diff[0], llptr->diff[1], llptr->diff[2], llptr->diff[3],
		llptr->spec[0], llptr->spec[1], llptr->spec[2], llptr->spec[3],
		llptr->pos[0], llptr->pos[1], llptr->pos[2],
		llptr->lookat[0], llptr->lookat[1], llptr->lookat[2], llptr->angle
		);
}

// Movement functions.
void forward(){
	fl.pos[0] += cos((fl.angle * M_PI) / 180);
	fl.pos[2] += sin((fl.angle * M_PI) / 180);
}

void backward(){
	fl.pos[0] -= cos((fl.angle * M_PI) / 180);
	fl.pos[2] -= sin((fl.angle * M_PI) / 180);
}
 
void left(){
	fl.angle -= 10;
	if (fl.angle < -180) fl.angle += 360;
}

void right(){
	fl.angle += 10;
	if (fl.angle > 180) fl.angle -= 360;
}

#endif
