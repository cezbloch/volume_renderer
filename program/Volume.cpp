#include "stdafx.h"

#include "Volume.h"

#include <sstream>
#include <fstream>

namespace {

std::string leaf(const std::string &path)
{
	std::string::size_type p = path.find_last_of("/\\");
	std::string l(path, p != path.npos ? p+1 : 0);
	return l;
}

} // anonymous namespace



Volume::Volume(vec3i size_)

	: data(size_.x*size_.y*size_.z),
	  gradients(size_.x*size_.y*size_.z),
	  size(size_),
	  texture(0)
{
}

Volume::~Volume()
{
	glDeleteTextures(1, &texture);
}

Volume::Volume(vec3i size_, std::string filename, std::string gradname,std::string mininame,vec3i mini_size)
	: data(size_.x*size_.y*size_.z),
	  gradients(size_.x*size_.y*size_.z,vec3si(0,0,0)),
	  mini_data(mini_size.x*mini_size.y*mini_size.z),
	  size(size_),
	  miniTexture(0),
	  gradientTexture(0),
	  texture(0)
{
	std::ifstream fs(filename.c_str(), std::ios::in | std::ios::binary);
	if (!fs.is_open())
		throw std::runtime_error("could not open " + filename);

	fs.read((char*)(&data[0]), unsigned(size_.x*size_.y*size_.z*sizeof(multidata)));
	if (fs.gcount() != sizeof(multidata)*size_.x*size_.y*size_.z)
		throw std::runtime_error("unexpected end of file in " + filename);

	std::ifstream gs(gradname.c_str(), std::ios::in | std::ios::binary);
	if (!gs.is_open())
		throw std::runtime_error("could not open " + gradname);

	gs.read((char*)(&gradients[0]), unsigned(size_.x*size_.y*size_.z*sizeof(vec3si)));
	if (gs.gcount() != sizeof(vec3si)*size_.x*size_.y*size_.z)
		throw std::runtime_error("unexpected end of file in " + gradname);

	//gs.close();

	std::ifstream ms(mininame.c_str(), std::ios::in | std::ios::binary);
	if (!ms.is_open())
		throw std::runtime_error("could not open " + mininame);

	ms.read((char*)(&mini_data[0]), unsigned(mini_size.x*mini_size.y*mini_size.z*sizeof(mini_type)));
	if (ms.gcount() != mini_size.x*mini_size.y*mini_size.z*sizeof(mini_type))
		throw std::runtime_error("unexpected end of file in " + mininame);




	std::cout<<"updating texture mini..";
	//updateTexture(miniTexture,GL_LUMINANCE,mini_size,GL_LUMINANCE,GL_UNSIGNED_BYTE,&mini_data[0]);
	updateMiniTex(mini_size);
	std::cout<<"done"<<std::endl;


	std::cout<<"updating texture grad..";
	updateGradTex();
	std::cout<<"done"<<std::endl;

	std::cout<<"updating texture multi..";
	updateDataTex();
	//updateTexture(texture,3,size,GL_RGB,GL_UNSIGNED_BYTE,&data[0]);
	std::cout<<"done"<<std::endl;

}

void Volume::updateTexture(GLuint tex,GLint internalFormat,vec3i curr_size,
						   GLenum format,GLenum type ,const GLvoid *pixels)
{
	const GLenum target = GL_TEXTURE_3D;

	if (!tex)
		glGenTextures(1, &tex);
	glBindTexture(target, tex);

	// set texture parameters
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_NEAREST);

	//glTexImage3D(target, 0, GL_LUMINANCE8, size.x, size.y, size.z, 0,GL_LUMINANCE, GL_UNSIGNED_BYTE, &data[0]);
	//glTexImage3D(target, 0, 4, size.x, size.y, size.z, 0,GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	glTexImage3D(target, 0, internalFormat, curr_size.x, curr_size.y, curr_size.z, 0,format, type, pixels);

	if (glGetError())
		throw std::runtime_error("error creating volume texture");
}


void Volume::updateMiniTex(vec3i m_size)
{
	const GLenum target = GL_TEXTURE_3D;

	if (!miniTexture)
		glGenTextures(1, &miniTexture);
	glBindTexture(target, miniTexture);

	// set texture parameters
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri( target, GL_GENERATE_MIPMAP, GL_TRUE );
	glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage3D(target, 0, 4, m_size.x, m_size.y, m_size.z, 0,GL_RGBA, GL_UNSIGNED_SHORT, &mini_data[0]);

	if (glGetError())
		throw std::runtime_error("error creating volume texture");
}

void Volume::updateDataTex()
{
	const GLenum target = GL_TEXTURE_3D;

	if (!texture)
		glGenTextures(1, &texture);
	glBindTexture(target, texture);

	// set texture parameters
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_NEAREST);

	//glTexImage3D(target, 0, GL_LUMINANCE8, size.x, size.y, size.z, 0,GL_LUMINANCE, GL_UNSIGNED_BYTE, &data[0]);
	//glTexImage3D(target, 0, 4, size.x, size.y, size.z, 0,GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	glTexImage3D(target, 0, 3, size.x, size.y, size.z, 0,GL_RGB, GL_UNSIGNED_BYTE, &data[0]);

	if (glGetError())
		throw std::runtime_error("error creating volume texture");
}

void Volume::updateGradTex()
{
	const GLenum target = GL_TEXTURE_3D;

	if (!gradientTexture)
		glGenTextures(1, &gradientTexture);
	glBindTexture(target, gradientTexture);

	// set texture parameters
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_NEAREST);

	//glTexImage3D(target, 0, GL_LUMINANCE8, size.x, size.y, size.z, 0,GL_LUMINANCE, GL_UNSIGNED_BYTE, &data[0]);
	//glTexImage3D(target, 0, 4, size.x, size.y, size.z, 0,GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	glTexImage3D(target, 0, 3, size.x, size.y, size.z, 0,GL_RGB, GL_SHORT, &gradients[0]);

	if (glGetError())
		throw std::runtime_error("error creating volume texture");
}