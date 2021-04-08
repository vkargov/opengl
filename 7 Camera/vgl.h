/* vova gl, functions generalizing repetitive actions */

#ifndef VGL_H
#define VGL_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct CameraState {
  struct KeyboardState {
    int state;
    double timestamp;
  } keys[GLFW_KEY_LAST+1];

  // Projection matrix. Can be affected by the window size callback.
  glm::mat4 projection;
  // FOV. Can be changed with keys.
  double fov;
  double aspect_ratio;
  double zNear;

  // Camera position and direction
  glm::vec3 pos; // aka eye
  glm::vec3 dir;
  glm::vec3 up;

  CameraState() :
    fov(90),
    zNear(0.1),
    pos(glm::vec3(0, 0, 1.5)),
    dir(glm::vec3(0, 0, -1)),
    up(glm::vec3(0, 1, 0))
    {
      for (auto& key : keys) {
        key.state = GLFW_RELEASE;
        key.timestamp = 0.;
      }
    };
};

extern GLfloat vglCubeVertices[];
extern size_t vglCubeVerticesSize;

GLuint vglLoadTexture(const char* path, const char* sampler_name, GLuint shader_program, GLuint texture_unit, GLenum format);
GLuint vglBuildShaderFromFile(const char* vertex_file_name, const char* fragment_file_name);
GLenum vglCheckError();

#endif