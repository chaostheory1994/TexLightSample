#define _CRT_SECURE_NO_WARNINGS
#include "Block.h"
#include <cstdio>
#include <string>
#include <iostream>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

TexList* Block::first = NULL;
GLuint* Block::tex_names = NULL;
GLfloat Block::tex_coords [][2] = {
		{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }
};

Block::Block(int bid, int bheight)
{
	id = bid;
	height = bheight; 
	int i;
	for (i = 0; i < 4; i++){
		emission[i] = 0.0;
		ambient[i] = 0.0;
		diffuse[i] = 0.0;
		spec[i] = 0.0;
	}
	tex = NULL;
}


Block::~Block()
{
}

/* Adds a texture to the texture link list */
void Block::add_texture(int id, int width, int height, char* filename) {
	TexList* curr;
	// Have we added a texture before?
	if (first == NULL){
		first = new TexList;
		curr = first;
	}
	else{
		// traverse to the end.
		curr = first;
		if (curr->id == id) {
			printf("ERR: Texture id already in use.\n");
			return;
		}
		while (curr->next != NULL) {
			curr = curr->next;
			if (curr->id == id) {
				printf("ERR: Texture id already in use.\n");
				return;
			}
		}
		curr->next = new TexList;
		curr = curr->next;
	}

	curr->id = id;
	curr->width = width;
	curr->height = height;
	curr->filename = filename;
	curr->next = NULL;
}

/* deletes all the textures. For cleanup purposes. */
void Block::delete_textures(){

}

void Block::print_text(int id){
	// First find the texture associeated with the id.
	TexList* curr = first;
	if (curr == NULL){
		printf("There are no textures registered yet");
		return;
	}
	while (curr->id != id){
		curr = curr->next;
		if (curr == NULL){
			printf("There is no texture of id: %d\n", id);
			return;
		}
	}

	printf("t %d %d %d ", curr->id, curr->width, curr->height);
	std::cout << curr->filename << std::endl;
}

void Block::set_ambient(float f1, float f2, float f3, float f4){
	ambient[0] = f1;
	ambient[1] = f2;
	ambient[2] = f3;
	ambient[3] = f4;
}

void Block::set_diffusion(float f1, float f2, float f3, float f4){
	diffuse[0] = f1;
	diffuse[1] = f2;
	diffuse[2] = f3;
	diffuse[3] = f4;
}

void Block::set_emission(float f1, float f2, float f3, float f4){
	emission[0] = f1;
	emission[1] = f2;
	emission[2] = f3;
	emission[3] = f4;
}

void Block::set_specular(float f1, float f2, float f3, float f4){
	spec[0] = f1;
	spec[1] = f2;
	spec[2] = f3;
	spec[3] = f4;
}

void Block::set_shininess(int sh){
	shininess = sh;
}

void Block::print(){
	int tex_id;
	if (tex == NULL) tex_id = -1;
	else tex_id = tex->id;
	printf("b %d %d [%d] %d (%3.1f %3.1f %3.1f %3.1f) (%3.1f %3.1f %3.1f %3.1f) (%3.1f %3.1f %3.1f %3.1f) (%3.1f %3.1f %3.1f %3.1f)\n",
		id, height, tex_id, shininess,
		emission[0], emission[1], emission[2], emission[3],
		ambient[0], ambient[1], ambient[2], ambient[3],
		diffuse[0], diffuse[1], diffuse[2], diffuse[3],
		spec[0], spec[1], spec[2], spec[3]);
}

int Block::set_texture(int id){
	// We need to traverse through to find the texture with that id.
	TexList* curr = first;
	while (curr != NULL){
		if (curr->id == id){
			tex = curr;
			return 0;
		}
		else curr = curr->next;
	}
	return -1;
}

void Block::draw(){
	// Start off by drawing the ground.
	glColor3f(0.5, 0.5, 0.5);
	
	glBegin(GL_POLYGON);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 10);
		glVertex3f(10, 0, 10);
		glVertex3f(10, 0, 0);
	glEnd();

	if (!height) return;
	// Set material
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

	glColor3f(ambient[0], ambient[1], ambient[2]);
	// Now to draw the building.

	if (tex != NULL){
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, tex_names[tex->id]);
	}
	glBegin(GL_POLYGON);
		glTexCoord2fv(tex_coords[0]);
		glVertex3f(1, 0, 1);
		glTexCoord2fv(tex_coords[1]);
		glVertex3f(1, 0, 9);
		glTexCoord2fv(tex_coords[2]);
		glVertex3f(1, height, 9);
		glTexCoord2fv(tex_coords[3]);
		glVertex3f(1, height, 1);
	glEnd();

	glBegin(GL_POLYGON);
		glTexCoord2fv(tex_coords[0]);
		glVertex3f(9, 0, 1);
		glTexCoord2fv(tex_coords[1]);
		glVertex3f(9, 0, 9);
		glTexCoord2fv(tex_coords[2]);
		glVertex3f(9, height, 9);
		glTexCoord2fv(tex_coords[3]);
		glVertex3f(9, height, 1);
	glEnd();

	glBegin(GL_POLYGON);
		glTexCoord2fv(tex_coords[0]);
		glVertex3f(1, 0, 1);
		glTexCoord2fv(tex_coords[1]);
		glVertex3f(9, 0, 1);
		glTexCoord2fv(tex_coords[2]);
		glVertex3f(9, height, 1);
		glTexCoord2fv(tex_coords[3]);
		glVertex3f(1, height, 1);
	glEnd();

	glBegin(GL_POLYGON);
		glTexCoord2fv(tex_coords[0]);
		glVertex3f(1, 0, 9);
		glTexCoord2fv(tex_coords[1]);
		glVertex3f(9, 0, 9);
		glTexCoord2fv(tex_coords[2]);
		glVertex3f(9, height, 9);
		glTexCoord2fv(tex_coords[3]);
		glVertex3f(1, height, 9);
	glEnd();

	if(tex != NULL) glDisable(GL_TEXTURE_2D); 
	// Do not draw on roof.
	glBegin(GL_POLYGON);
		glVertex3f(1, height, 1);
		glVertex3f(1, height, 9);
		glVertex3f(9, height, 9);
		glVertex3f(9, height, 1);
	glEnd();
}

void Block::bmp2rgb(GLubyte img [], int size) {
	int i;
	GLubyte temp;

	for (i = 0; i<size; i += 3) {
		temp = img[i + 2];
		img[i + 2] = img[i + 1];
		img[i + 1] = temp;

	}
}

void Block::load_bmp(const char *fname, GLubyte img [], int size, GLuint *ptname) {
	FILE *fp;

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		fprintf(stderr, "unable to open texture file %s\n", fname);
		exit(1);
	}

	fseek(fp, 8, SEEK_SET);
	fread(img, size*size * 3, 1, fp);
	bmp2rgb(img, size*size * 3);
	fclose(fp);

	if (ptname) {
		// initialize the texture
		glGenTextures(1, ptname);
		glBindTexture(GL_TEXTURE_2D, *ptname);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			size, size,
			0, GL_RGB, GL_UNSIGNED_BYTE, img);
	}
}

int Block::getLargestID(){
	int ret = 0;
	TexList* curr = first;
	while (curr != NULL){
		if (curr->id > ret) ret = curr->id;
		curr = curr->next;
	}
	return ret;
}

void Block::confirm_textures(){
	TexList* curr = first;
	const char* temp;
	tex_names = new GLuint[Block::getLargestID() + 1];
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	while (curr != NULL){
		curr->name = new GLubyte[curr->width * curr->height * 3];
		temp = curr->filename.c_str();
		load_bmp(temp, curr->name, curr->width, &tex_names[curr->id]);
		curr = curr->next;
	}
}