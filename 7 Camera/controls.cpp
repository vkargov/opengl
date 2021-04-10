#include <iostream>

#include <GLFW/glfw3.h>

#include "camera.h"
#include "vgl.h"

using namespace std;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  //cout << "Key event!\n" << flush;

  CameraState *cam = static_cast<CameraState *>(glfwGetWindowUserPointer(window));

  // Not interested in repeats, they only mess things up with jerky motion
  // If a key has been pressed, we *know* it's still pressed if it hasn't been released
  // (big brain moment)
  if (action == GLFW_REPEAT)
    return;

  if (cam->keys[key].state != action) {
    cam->keys[key].state = action;
    cam->keys[key].timestamp = glfwGetTime();
  }

  cout << "Action = " << action << '\n';
}

void mouse_callback (GLFWwindow* window, double xPos, double yPos) {
  auto cam = static_cast<CameraState *>(glfwGetWindowUserPointer(window));

  if (cam->firstMouseCall) {
    cam->lastX = xPos;
    cam->lastY = yPos;
    cam->firstMouseCall = false;
    return;
  }

  constexpr double sensitivity = 0.1;

  double xOffset = xPos - cam->lastX;
  double yOffset = yPos - cam->lastY;
  cam->lastX = xPos;
  cam->lastY = yPos;

  cam->yaw += xOffset * sensitivity;
  cam->pitch -= yOffset * sensitivity;

  // TODO figure out why there are artifacts at 90
  constexpr float pitchThreshold = 89;
  if (cam->pitch > pitchThreshold)
    cam->pitch = pitchThreshold;
  else if (cam->pitch < -pitchThreshold)
    cam->pitch = -pitchThreshold;

  cout << cam->pitch << '\n';

  cam->calcPos();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  auto cam = static_cast<CameraState *>(glfwGetWindowUserPointer(window));

  cout << "cb" << '\n';
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS)
      // "Aiming down sights" style zoom
      cam->fov = cam->default_fov * 0.5;
    else
      cam->fov = cam->default_fov;

    updateProjectionMatrix_(cam);
  }
}

void handleKeys(CameraState& cam) {

  bool update_projection_matrix = false;

  auto d0 = glfwGetTime();

  constexpr float speed = 0.05;
  for (int key = 0; key < GLFW_KEY_LAST; key++) {
    if (cam.keys[key].state == GLFW_RELEASE)
      continue;

    float dt = d0 - cam.keys[key].timestamp;
    float dts = speed * dt;

    switch (key) {
      case GLFW_KEY_W:
      case GLFW_KEY_UP:
        cam.pos += cam.dir * dts;
        cout << "up\n";
        break;
      case GLFW_KEY_S:
      case GLFW_KEY_DOWN:
        cam.pos -= cam.dir * dts;
        cout << "down\n";
        break;
      case GLFW_KEY_A:
      case GLFW_KEY_LEFT:
        cam.pos -= glm::normalize(glm::cross(cam.dir, cam.up)) * dts;
        break;
      case GLFW_KEY_D:
      case GLFW_KEY_RIGHT:
        cam.pos += glm::normalize(glm::cross(cam.dir, cam.up)) * dts;
        break;
      case GLFW_KEY_SPACE:
        cam.pos += cam.up * dts;
        break;
      case GLFW_KEY_LEFT_CONTROL:
      case GLFW_KEY_RIGHT_CONTROL:
        cam.pos -= cam.up * dts;
        break;
      case GLFW_KEY_LEFT_BRACKET:
        cam.default_fov -= 1 * dt;
        update_projection_matrix = true;
        break;
      case GLFW_KEY_RIGHT_BRACKET:
        cam.default_fov += 1 * dt;
        update_projection_matrix = true;
        break;
      case GLFW_KEY_J:
        cam.zNear -= 0.01 * dt;
        update_projection_matrix = true;
        break;
      case GLFW_KEY_K:
        cam.zNear += 0.01 * dt;
        update_projection_matrix = true;
        break;
    }

    if (update_projection_matrix) {
      //cout << "FOV = " << cam.fov << ", zNear = " << cam.zNear << endl;
      updateProjectionMatrix_(&cam);
    }
  }
}