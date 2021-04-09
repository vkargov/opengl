#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "vgl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void tryOutGlm() {
  glm::mat4 proj = glm::perspective(glm::radians(40.0f), 1.0f, 0.1f, 100.0f);
  cout << "Perspective matrix:\n" << glm::to_string(proj) << '\n';
}

struct Pony {
  glm::vec3 pos;
  glm::vec3 rotation_axis;
  double speed;
  double scale;
};

void updateProjectionMatrix_ (CameraState *cam) {
  cam->projection = glm::perspective(glm::radians(cam->fov), cam->aspect_ratio, cam->zNear, 100.);
}

void updateProjectionMatrix (GLFWwindow* window) {
  auto cam = static_cast<CameraState *>(glfwGetWindowUserPointer(window));
  updateProjectionMatrix_(cam);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
  auto camera_state = static_cast<CameraState *>(glfwGetWindowUserPointer(window));
  camera_state->aspect_ratio = (double) width / height;
  updateProjectionMatrix(window);
}

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

void handleKeys(CameraState& cam) {

  bool update_projection_matrix = false;

  auto d0 = glfwGetTime();

  constexpr float speed = 0.05;
  for (int key = 0; key < GLFW_KEY_LAST; key++) {
    if (cam.keys[key].state == GLFW_RELEASE)
      continue;

    float dt = d0 - cam.keys[key].timestamp;

    switch (key) {
      case GLFW_KEY_W:
      case GLFW_KEY_UP:
        cam.pos += cam.dir * speed * dt;
        cout << "up\n";
        break;
      case GLFW_KEY_S:
      case GLFW_KEY_DOWN:
        cam.pos -= cam.dir * speed * dt;
        cout << "down\n";
        break;
      case GLFW_KEY_A:
        cam.pos -= glm::normalize(glm::cross(cam.dir, cam.up)) * speed * dt;
        break;
      case GLFW_KEY_D:
        cam.pos += glm::normalize(glm::cross(cam.dir, cam.up)) * speed * dt;
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

int main()
{
  // glfw: initialize and configure
  // ------------------------------
  if (!glfwInit()) {
    cout << "Cannot initialize GLFW!";
    return -1;
  };
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Enable vsync
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
    {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  CameraState cam{};
  glfwSetWindowUserPointer(window, &cam);

  // update the projection matrix whenever the window changes
  glfwSetWindowSizeCallback(window, window_size_callback);

  glfwSetKeyCallback(window, keyCallback);

  // Capture the mouse cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // Setup the mouse callback
  glfwSetCursorPosCallback(window, mouse_callback);  
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
    }

  auto ponyShader = vglBuildShaderFromFile("transpose_vert.glsl", "texture_frag.glsl");
  auto bgShader = vglBuildShaderFromFile("transpose_vert.glsl", "psychedelic_frag.glsl");


  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vglCubeVerticesSize, vglCubeVertices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // texture coordinate
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0+3);
  glEnableVertexAttribArray(1);

  // needs to be called before setting up uniforms by vglLoadTexture
  glUseProgram(ponyShader);

  GLuint pony_texture;
  pony_texture = vglLoadTexture("../resources/container.jpg", "pony", ponyShader, 0, GL_RGB);

  /* Variable 'time' is used for the fade effect. */
  auto pony_time_loc = glGetUniformLocation(ponyShader, "time");
  // Actually the uniform can be -1 if it's not used.
  // This is not considered an error.
  // See https://community.khronos.org/t/keep-unused-shader-variables-for-debugging/61280/5
  // if (time < 0) { q  
  //   std::cerr << "time oof\n";
  //   return 1;
  // }
  auto t1 = std::chrono::high_resolution_clock::now();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pony_texture);

  // Enable transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Experiement with GLM									  
  tryOutGlm();
  
  // Build pones
  vector<Pony> ponies(30);
  mt19937 gen(random_device{}());
  uniform_real_distribution<> distr_vec(-1, 1);
  uniform_real_distribution<> distr_speed(0, 1);

  for (int i = 0; i < ponies.size(); i++) {
    auto&& pony = ponies[i];
    pony.pos.x = distr_vec(gen);
    pony.pos.y = distr_vec(gen);
    pony.pos.z = distr_vec(gen);
    glm::vec3 axis;
    axis.x = distr_vec(gen);
    axis.y = distr_vec(gen);
    axis.z = distr_vec(gen);
    pony.rotation_axis = glm::normalize(axis);
    pony.speed = distr_speed(gen);
    pony.scale = distr_speed(gen) * 0.5;

    // Collision check. Re-roll all the parameters if the collision check fails.
    for (int j = 0; j < i; j++) {
      auto&& pony2 = ponies[j];
      auto dx = (pony.pos.x - pony2.pos.x);
      auto dy = (pony.pos.y - pony2.pos.y);
      auto dz = (pony.pos.z - pony2.pos.z);
      auto mindist = pony.scale + pony2.scale;
      if (2 * mindist * mindist > dx*dx + dy*dy + dz*dz)
        i--;
    }
    // cout << "Generated the followig pony:\n";
    // cout << "Position: " << pony.pos.x << ", " << pony.pos.y << ", " << pony.pos.z << '\n';
    // cout << "Rotation axis: " << pony.rotation_axis.x << ", " << pony.rotation_axis.y << ", " << pony.rotation_axis.z << '\n';
    // cout << "Speed: " << pony.speed << '\n';
    // cout << "Scale: " << pony.speed << '\n';
  }

  auto pony_tm_uniform_location = glGetUniformLocation(ponyShader, "tm");
  auto background_tm_uniform_location = glGetUniformLocation(bgShader, "tm");

  glm::mat4 identity_matrix = glm::mat4(1.0f);

  // Set up the perspective projection
  window_size_callback(window, SCR_WIDTH, SCR_HEIGHT);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
    {
      // input
      // -----
      processInput(window);

      handleKeys(cam);

      int state;

      // render
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // No need to use glUseProgram every frame because there's only one shader
      // https://stackoverflow.com/questions/64089592/why-is-gluseprogram-called-every-frame-with-gluniform
      // glUseProgram(shaderProgram);
      
      auto t2 = std::chrono::high_resolution_clock::now();
      auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 10.;

      // Render background
      glUseProgram(bgShader);

      auto background_time_loc = glGetUniformLocation(bgShader, "time");
      glUniformMatrix4fv(background_tm_uniform_location, 1, GL_FALSE, glm::value_ptr(identity_matrix));
      //cout << background_time_loc << ", " << '\n';
      glUniform1f(background_time_loc, dt);
      //cout << "error status: " << glGetError() << '\n';
      //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      // Render ponies
      glUseProgram(ponyShader);

      glUniform1f(pony_time_loc, (GLfloat)dt);

      for (auto& pony : ponies) {
        // Model matrix
        //cout << "New matrix\n";
        glm::mat4 model = glm::translate(identity_matrix, pony.pos);
        //cout << "After translate: " << glm::to_string(tm) << '\n';
        auto angle = glm::radians<float>(pony.speed * dt);
        model = glm::rotate(model, angle, pony.rotation_axis);
        //cout << "After rotate:" << glm::to_string(tm) << '\n';
        model = glm::scale(model, glm::vec3(pony.scale));

        float radius = 1.5;
        // rotate clockwise
        float cameraX = -sin(glfwGetTime()) * radius;
        float cameraZ = -cos(glfwGetTime()) * radius;
        glm::vec3 eye = glm::vec3(cameraX, 0, cameraZ);
        glm::vec3 center = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
        glm::mat4 camera = glm::lookAt(cam.pos, cam.pos + cam.dir, cam.up);
        glm::mat4 tm = cam.projection * camera * model;

        glUniformMatrix4fv(pony_tm_uniform_location, 1, GL_FALSE, glm::value_ptr(tm));

        // Draw pone
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14); // 14 vertices represent 1 cube
      }

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

