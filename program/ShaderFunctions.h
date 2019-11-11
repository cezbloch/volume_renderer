#pragma once

#include <string>

std::string loadFile(const std::string filename);
GLuint loadProgram(const std::string &vertexFile, const std::string &fragmentFile);
GLuint loadShader(const std::string &file, GLuint type);
