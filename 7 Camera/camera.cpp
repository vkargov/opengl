#include "camera.h"

void updateProjectionMatrix_(CameraState *cam) {
  cam->projection = glm::perspective(glm::radians(cam->fov), cam->aspect_ratio,
                                     cam->zNear, 100.);
}

void CameraState::calcPos() {
  dir.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
  dir.y = sin(glm::radians(pitch));
  dir.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
  // dir *= 0.9;
  std::cout << dir.x << ", " << dir.y << ", " << dir.z << '\n';
}