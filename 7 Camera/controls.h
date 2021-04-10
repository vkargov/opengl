#ifndef CONTROLS_H
#define CONTROLS_H

#include <GLFW/glfw3.h>

#include "camera.h"
#include "vgl.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback (GLFWwindow* window, double xPos, double yPos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void handleKeys(CameraState& cam);

#endif