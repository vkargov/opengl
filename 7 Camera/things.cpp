#include <random>
#include <vector>

#include "things.h"

using namespace std;

vector<Thing> makeCubeScene(int num) {
  vector<Thing> things(30);

  // Build pones
  mt19937 gen(random_device{}());
  uniform_real_distribution<> distr_vec(-1, 1);
  uniform_real_distribution<> distr_speed(0, 1);

  for (int i = 0; i < things.size(); i++) {
    auto &&thing = things[i];
    thing.pos.x = distr_vec(gen);
    thing.pos.y = distr_vec(gen);
    thing.pos.z = distr_vec(gen);
    glm::vec3 axis;
    axis.x = distr_vec(gen);
    axis.y = distr_vec(gen);
    axis.z = distr_vec(gen);
    thing.rotation_axis = glm::normalize(axis);
    thing.speed = distr_speed(gen);
    thing.scale = distr_speed(gen) * 0.5;

    // Collision check. Re-roll all the parameters if the collision check fails.
    for (int j = 0; j < i; j++) {
      auto &&thing2 = things[j];
      auto dx = (thing.pos.x - thing2.pos.x);
      auto dy = (thing.pos.y - thing2.pos.y);
      auto dz = (thing.pos.z - thing2.pos.z);
      auto mindist = thing.scale + thing2.scale;
      if (2 * mindist * mindist > dx * dx + dy * dy + dz * dz)
        i--;
    }
    // cout << "Generated the followig pony:\n";
    // cout << "Position: " << pony.pos.x << ", " << pony.pos.y << ", " <<
    // pony.pos.z << '\n'; cout << "Rotation axis: " << pony.rotation_axis.x <<
    // ", " << pony.rotation_axis.y << ", " << pony.rotation_axis.z << '\n';
    // cout << "Speed: " << pony.speed << '\n';
    // cout << "Scale: " << pony.speed << '\n';
  }

  return things;
}