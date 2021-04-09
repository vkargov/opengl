/* vova gl, functions generalizing repetitive actions */

#ifndef VGL_H
#define VGL_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class CameraState {
public:
  void calcPos() {
    dir.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    dir.y = sin(glm::radians(pitch));
    dir.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    //dir *= 0.9;
    std::cout << dir.x << ", " << dir.y << ", " << dir.z << '\n';
  }

  struct KeyboardState {
    int state = GLFW_RELEASE;
    double timestamp = 0.;
  } keys[GLFW_KEY_LAST+1];

  // Projection matrix. Can be affected by the window size callback.
  glm::mat4 projection;
  // FOV. Can be changed with keys.
  double fov = 90;
  double aspect_ratio;
  double zNear = 0.01;

  // Camera position and direction
  glm::vec3 pos = glm::vec3(0, 0, 1.5); // aka eye
  glm::vec3 dir = glm::vec3(0, 0, -1);
  glm::vec3 up = glm::vec3(0, 1, 0);

  // last camera position
  double lastX = 0, lastY = 0;
  bool firstMouseCall = true;
  
  double yaw = 90, pitch = 0;
};

extern GLfloat vglCubeVertices[];
extern size_t vglCubeVerticesSize;

GLuint vglLoadTexture(const char* path, const char* sampler_name, GLuint shader_program, GLuint texture_unit, GLenum format);
GLuint vglBuildShaderFromFile(const char* vertex_file_name, const char* fragment_file_name);
GLenum vglCheckError();

#endif