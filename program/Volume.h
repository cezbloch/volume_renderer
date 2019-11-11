#pragma once

#include "vec3.h"

#include <vector>
#include <string>

class multidata
{
public:
	unsigned char x;
	unsigned char y;
	unsigned char z;
//multidata::multidata(x,y)
};

class tex_type
{
public:
vec3usi gradients;
unsigned short int size;
};

class grad_float
{
public:
vec3si gradients;
grad_float::grad_float(vec3si g)
	{
	gradients=g;
	}
};

class Volume {
	Volume(const Volume &);
	Volume &operator=(const Volume &);
public:
	//typedef grad_float value_type;
	//typedef std::vector<value_type> Data;
	//typedef unsigned char mini_type;
	typedef tex_type mini_type;

	vec3i size;
	std::vector<multidata> data;
	std::vector<vec3si> gradients;
	std::vector<mini_type> mini_data;
	GLuint texture,gradientTexture,miniTexture;

	multidata &operator()(int x, int y, int z)
		{ return data[x+y*size.x+z*(size.x*size.y)]; }
	const multidata &operator()(int x, int y, int z) const
		{ return data[x+y*size.x+z*(size.x*size.y)]; }

	Volume(vec3i size_);
	Volume::Volume(vec3i size_, std::string filename, std::string gradname,std::string mininame,vec3i mini_size);
	~Volume();

void Volume::updateTexture(GLuint tex,GLint internalFormat,vec3i curr_size,
						   GLenum format,GLenum type ,const GLvoid *pixels);
	void updateDataTex();
	void updateGradTex();
	void updateMiniTex(vec3i m_size);
	//void updateMiniTex();
	//void write_gradient(vec3 point,vec3 value){gradients[point.x+point.y*size.x+point.z*(size.x*size.y)]=value;}
	//vec3 read_gradient(vec3i point){return gradients[point.x+point.y*size.x+point.z*(size.x*size.y)];}
	//value_type read_data(int x, int y, int z){ return data[x+y*size.x+z*(size.x*size.y)]; }
};
