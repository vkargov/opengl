// 3D things
// Cube things, pony things, shiny things, you name it

#ifndef THINGS_H
#define THINGS_H

#include <vector>

#include <glm/glm.hpp>

struct Thing {
  glm::vec3 pos;
  glm::vec3 rotation_axis;
  double speed;
  double scale;
};

std::vector<Thing> makeCubeScene(int num);

#endif