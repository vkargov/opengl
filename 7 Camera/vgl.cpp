#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <string>

#include "vgl.h"

// Optimal reprezentation of a cube for
// GL_TRIANGLE_STRIP rendering
// Note to self: it takes way more time to
// figure this out than it seems
// But hey look, only 14 vertices
// ------------------------------------------------------------------
GLfloat vglCubeVertices[] = {
  // positions          // texture coordinates
  -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,  1.0f,  0.0f,
   1.0f, -1.0f, -1.0f,  0.0f,  1.0f,
   1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
   1.0f,  1.0f,  1.0f,  1.0f,  0.0f,
  -1.0f, -1.0f,  1.0f,  0.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
  -1.0f,  1.0f, -1.0f,  1.0f,  0.0f,
   1.0f, -1.0f, -1.0f,  0.0f,  1.0f,
   1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,  1.0f,  0.0f,
  -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f
};
size_t vglCubeVerticesSize = sizeof(vglCubeVertices);

GLuint vglLoadTexture(const char* path, const char* sampler_name, GLuint shader_program, GLuint texture_unit, GLenum format) {
  // prepare our texture
  GLuint texture;  
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
  if (!data) {
    std::cerr << "vglLoadTexture oof: cannot load texture " << path << '\n';
    throw std::invalid_argument{path};
  }
  //std::cout << "Image loaded with width = " << width << " and height = " << height << '\n';
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  GLint texture_unit_location;
  texture_unit_location = glGetUniformLocation(shader_program, sampler_name);
  if (texture_unit_location < 0) {
    if (texture_unit_location == -1) {
      std::cerr << "vglLoadTexture oof: couldn't find find " << sampler_name << ".\n";
    }
    std::cerr << "glGetUniformLocation oof\n";
    return 1;
  }
  glUniform1i(texture_unit_location, texture_unit);

  return texture;
}

GLuint vglBuildShaderFromFile(const char* vertex_file_name, const char* fragment_file_name) {
  // TODO idiomatic reading of files into a string is a whole can of worms...
  // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  std::ifstream vertex_shader_file(vertex_file_name);
  std::string vertex_shader_src {std::istreambuf_iterator<char>{vertex_shader_file},
                                 std::istreambuf_iterator<char>{}};
  std::ifstream fragment_shader_file(fragment_file_name);
  std::string fragment_shader_src {std::istreambuf_iterator<char>{fragment_shader_file},
                                   std::istreambuf_iterator<char>{}};

  GLint success;
  char infoLog[512];

  // vertex shader
  int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  const char * vertex_shader_arr = vertex_shader_src.c_str();
  glShaderSource(vertex_shader, 1, &vertex_shader_arr, NULL);
  glCompileShader(vertex_shader);
  // check for shader compile errors

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
      std::cout << infoLog << '\n';
      std::string log = std::to_string(sizeof(infoLog)); 
      throw std::runtime_error(std::string{"Vertex shader compilation failed:\n"} + log + infoLog + '\n');
    }

  // fragment shader
  int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char * fragment_shader_arr = fragment_shader_src.c_str();
  glShaderSource(fragment_shader, 1, &fragment_shader_arr, NULL);
  glShaderSource(fragment_shader, 1, &fragment_shader_arr, NULL);
  glCompileShader(fragment_shader);
  // check for shader compile errors
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
      std::cout << infoLog << '\n';
      throw std::runtime_error(std::string{"Fragment shader compilation failed:\n"} + infoLog + '\n');
    }

  // Compose the shader program
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  // check for linking errors
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    throw std::runtime_error{std::string{"Fragment shader compilation failed:\n"} + infoLog + '\n'};
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

GLenum vglCheckError() {
  // Error messages taken from:
  // https://www.khronos.org/opengl/wiki/OpenGL_Error
  auto error = glGetError();
  std::string msg;
  switch (error) {
  case GL_NO_ERROR:
    return error;
  case GL_INVALID_ENUM:
    msg = "GL_INVALID_ENUM: \
Given when an enumeration parameter is not a legal enumeration for that function. \
* glEnable: are you using the correct value? You can't just OR multiple values!\n";
    break;
  case GL_INVALID_VALUE:
    msg = "GL_INVALID_VALUE";
    break;  
  case GL_INVALID_OPERATION:
    msg = "GL_INVALID_OPERATION: \
The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.\n\
Things to check are:\n\
* Are you sure the type of the uniform you're setting matches?\n";
    break;  
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    msg = "GL_INVALID_FRAMEBUFFER_OPERATION";
    break;  
  default:
    msg = "Unknown";
  }

  std::cout << "Error: " << msg << '\n';

  return error;
}