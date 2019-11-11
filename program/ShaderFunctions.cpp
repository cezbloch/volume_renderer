#include "stdafx.h"

#include "ShaderFunctions.h"

#include <fstream>
#include <stdexcept>
#include <vector>

std::string loadFile(const std::string filename)
{
	std::string contents;
	char buf[1024];

	std::ifstream fs(filename.c_str());
	if (!fs.is_open())
		throw std::runtime_error("could not open file: " + filename);

	do {
		fs.read(buf, sizeof buf);
		contents.append(buf, buf + fs.gcount());
	} while (fs);

	return contents;
}

namespace {

template<class Getiv, class GetInfoLog>
std::string getInfoLog(Getiv getiv, GetInfoLog getInfoLog, GLuint id)
{
	// get size of info log
	GLint loglen = 0;
	getiv(id, GL_INFO_LOG_LENGTH, &loglen);

	// get info log
	std::vector<char> buf(loglen);
	getInfoLog(id, loglen, 0, &buf[0]);

	// copy to string and return
	std::string logstr(buf.begin(), buf.end());
	return logstr;
}

} // anonymous namespace

GLuint loadProgram(const std::string &vertexFile, const std::string &fragmentFile)
{
	GLuint vshader = loadShader(vertexFile, GL_VERTEX_SHADER);
	GLuint fshader = loadShader(fragmentFile, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	if (!program)
		throw std::runtime_error("could not create program");

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	glLinkProgram(program);

	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		// get error, clean up, report error
		std::string logstr = getInfoLog(glGetProgramiv, glGetProgramInfoLog, program);
		glDeleteProgram(program);
		throw std::runtime_error("linking failed for shader \""
			+ vertexFile + "\", \"" + fragmentFile + "\":\n" + logstr);
	}

	return program;
}

GLuint loadShader(const std::string &file, GLuint type)
{
	GLuint shader = glCreateShader(type);
	if (!shader)
		throw std::runtime_error("could not create shader");

	std::string src = loadFile(file);
	const char *s = src.c_str();
	glShaderSource(shader, 1, &s, 0);
	glCompileShader(shader);

	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		// get error, clean up, report error
		std::string logstr = getInfoLog(glGetShaderiv, glGetShaderInfoLog, shader);
		glDeleteShader(shader);
		throw std::runtime_error("compilation failed for shader \"" + file + "\":\n" + logstr);
	}

	return shader;
}
