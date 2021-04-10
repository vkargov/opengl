#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "controls.h"
#include "camera.h"
#include "things.h"
#include "vgl.h"

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
  
  auto things = makeCubeScene(30);

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

      for (auto& thing : things) {
        // Model matrix
        //cout << "New matrix\n";
        glm::mat4 model = glm::translate(identity_matrix, thing.pos);
        //cout << "After translate: " << glm::to_string(tm) << '\n';
        auto angle = glm::radians<float>(thing.speed * dt);
        model = glm::rotate(model, angle, thing.rotation_axis);
        //cout << "After rotate:" << glm::to_string(tm) << '\n';
        model = glm::scale(model, glm::vec3(thing.scale));

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

