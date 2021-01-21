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

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
    }

  auto ponyShader = vglBuildShaderFromFile("transpose_vert.glsl", "texture_frag.glsl");
  auto bgShader = vglBuildShaderFromFile("transpose_vert.glsl", "psychedelic_frag.glsl");

  // Optimal reprezentation of a cube for
  // GL_TRIANGLE_STRIP rendering
  // Note to self: it takes way more time to
  // figure this out than it seems
  // But hey look, only 14 vertices
  // ------------------------------------------------------------------
  GLfloat vertices[] = {
    // positions          // texture coordinates
    -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,  1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  0.0f,  1.0f,
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  0.0f,  1.0f,
     1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f
  };

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // texture coordinate
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0+3);
  glEnableVertexAttribArray(1);

  // needs to be called before setting up uniforms by vglLoadTexture
  glUseProgram(ponyShader);

  GLuint pony_texture;
  pony_texture = vglLoadTexture("container.jpg", "pony", ponyShader, 0, GL_RGB);

  /* Variable 'time' is used for the fade effect. */
  auto pony_time_loc = glGetUniformLocation(ponyShader, "time");
  // Actually the uniform can be -1 if it's not used.
  // This is not considered an error.
  // See https://community.khronos.org/t/keep-unused-shader-variables-for-debugging/61280/5
  // if (time < 0) {
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
  vector<Pony> ponies(10);
  mt19937 gen(random_device{}());
  uniform_real_distribution<> distr_vec(-1, 1);
  uniform_real_distribution<> distr_speed(0, 1);

  for (Pony& pony : ponies) {
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
    // cout << "Generated the followig pony:\n";
    // cout << "Position: " << pony.pos.x << ", " << pony.pos.y << ", " << pony.pos.z << '\n';
    // cout << "Rotation axis: " << pony.rotation_axis.x << ", " << pony.rotation_axis.y << ", " << pony.rotation_axis.z << '\n';
    // cout << "Speed: " << pony.speed << '\n';
    // cout << "Scale: " << pony.speed << '\n';
  }

  auto pony_tm_uniform_location = glGetUniformLocation(ponyShader, "tm");
  auto background_tm_uniform_location = glGetUniformLocation(bgShader, "tm");

  glm::mat4 identity_matrix = glm::mat4(1.0f);

  // Camera matrix
  glm::mat4 camera = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -1.5));

  // Perspective translation
  glm::mat4 asd = glm::perspective(90., (double) SCR_WIDTH / SCR_HEIGHT, 0.1, 100.);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
    {
      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
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

        glm::mat4 tm = asd * camera * model;        

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

