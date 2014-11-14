#ifndef __BLOCK_H__
#define __BLOCK_H__
#include <cstdio>
#include <string>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
// texture struct
struct TexList {
	int id;
	int width;
	int height;
	std::string filename;
	GLubyte* name;
	TexList* next;
};

class Block
{
public:
	Block(int, int);
	~Block();
	static void add_texture(int, int, int, char*);
	static void delete_textures();
	static void print_text(int);
	void set_shininess(int);
	void set_emission(float, float, float, float);
	void set_ambient(float, float, float, float);
	void set_diffusion(float, float, float, float);
	void set_specular(float, float, float, float);
	void print();
	int set_texture(int);
	void draw();
	static void confirm_textures();
private:
	static void bmp2rgb(GLubyte img [], int size);
	static void load_bmp(const char *fname, GLubyte img [], int size, GLuint *ptname);
	static GLuint* tex_names;
	static int getLargestID();
	static TexList* first;
	static GLfloat tex_coords [][2];
	int id;
	int height;
	TexList* tex;
	int shininess;
	float emission[4];
	float ambient[4];
	float diffuse[4];
	float spec[4];
};


#endif