#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

class CameraState {
public:
  void calcPos();

  struct KeyboardState {
    int state = GLFW_RELEASE;
    double timestamp = 0.;
  } keys[GLFW_KEY_LAST+1];

  // Projection matrix. Can be affected by the window size callback.
  glm::mat4 projection;
  // FOV. Can be changed with keys.
  double default_fov = 90;
  double fov = default_fov;
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

void updateProjectionMatrix_ (CameraState *cam);

#endif