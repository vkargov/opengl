/* vova gl, functions generalizing repetitive actions */

#ifndef VGL_H
#define VGL_H

#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

extern GLfloat vglCubeVertices[];
extern size_t vglCubeVerticesSize;

GLuint vglLoadTexture(const char* path, const char* sampler_name, GLuint shader_program, GLuint texture_unit, GLenum format);
GLuint vglBuildShaderFromFile(const char* vertex_file_name, const char* fragment_file_name);
GLenum vglCheckError();

#endif