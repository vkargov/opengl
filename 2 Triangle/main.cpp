#include <iostream>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

int main()
{
  GLfloat vertices[] = {
    // triangle
    -0.5f, -0.18f, 0.0f,
    -0.3f, 0.18f, 0.0f,
    -0.1f, -0.18f, 0.0f,
    // rectangle
    0.1f, -0.18f, 0.0f,
    0.1f, 0.18f, 0.0f,
    0.46f, 0.18f, 0.0f,
    0.46f, -0.18f, 0.0f
  };
  GLuint rectangle_indices[] = {3, 4, 5, 5, 6, 3};

  for (int i = 0; i < sizeof(vertices)/sizeof(vertices[0]); i++)
    vertices[i] *= 1.8;
  
  char infoLog[512];

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
    {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
    }    
    
  //glViewport(0, 0, 800, 600);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Define & compile the vertex shader.
  GLuint barsProgram, ripplesProgram;
  {  
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
      {
	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	std::cout << "Vertex shader compilation error:\n" << infoLog << std::endl;
      }    

  // Define & compile the fragment shader.
    const char* barsShaderSource = R"(
#version 330 core
uniform float offset;
out vec4 FragColor;

void main()
{
    float x = abs(sin((gl_FragCoord.x+offset)*0.1));
    float y = abs(sin((gl_FragCoord.y+offset)*0.1));
    FragColor = vec4(x,
                     y,
                     (x+y)*0.5,
                     1.0f);
}
)";
    GLuint barsShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(barsShader, 1, &barsShaderSource, NULL);
    glCompileShader(barsShader);
    glGetShaderiv(barsShader, GL_COMPILE_STATUS, &success);
    if (!success)
      {
	glGetShaderInfoLog(barsShader, 512, NULL, infoLog);
	std::cout << "Fragment shader compilation error:\n" << infoLog << std::endl;
	exit(1);
      }    
    const char* ripplesShaderSource = R"(
#version 330 core
uniform float offset;
out vec4 FragColor;

void main()
{
    float dist = distance(vec2(190, 280), vec2(gl_FragCoord.x, gl_FragCoord.y));
    float color = abs(sin(dist*0.1-offset*0.02));
    //float color = abs(sin((gl_FragCoord.x+offset)*0.1));
    FragColor = vec4(0,
                     color,
                     0,
                     1.0f);
}
)";
    GLuint ripplesShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ripplesShader, 1, &ripplesShaderSource, NULL);
    glCompileShader(ripplesShader);
    glGetShaderiv(ripplesShader, GL_COMPILE_STATUS, &success);
    if (!success)
      {
	glGetShaderInfoLog(ripplesShader, 512, NULL, infoLog);
	std::cout << "Fragment shader compilation error:\n" << infoLog << std::endl;
	exit(1);
      }    

    // Create the bars shader program
    barsProgram = glCreateProgram();
    glAttachShader(barsProgram, vertexShader);
    glAttachShader(barsProgram, barsShader);
    glLinkProgram(barsProgram);
    glGetProgramiv(barsProgram, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(barsProgram, 512, NULL, infoLog);
      std::cout << "Shader linking error:\n" << infoLog << std::endl;
      exit(1);
    }

    // Create the ripples shader program
    ripplesProgram = glCreateProgram();
    glAttachShader(ripplesProgram, vertexShader);
    glAttachShader(ripplesProgram, ripplesShader);
    glLinkProgram(ripplesProgram);
    glGetProgramiv(ripplesProgram, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ripplesProgram, 512, NULL, infoLog);
      std::cout << "Shader linking error:\n" << infoLog << std::endl;
      exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(barsShader);
    glDeleteShader(ripplesShader);

  }
 
  auto t1 = std::chrono::high_resolution_clock::now();

  GLuint VAO/*scene*/, EBO[1]/*rectangle*/, VBO[1]/*triangle*/;

  // Vertex array objet containing our scene
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Setting up our triangle contained in a VBO
  glGenBuffers(1, VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
  glEnableVertexAttribArray(0);

  // Setting up our rectangle contained in a EBO
  glGenBuffers(1, EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangle_indices), rectangle_indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
  glEnableVertexAttribArray(0);

  while(!glfwWindowShouldClose(window))
    {
      processInput(window);

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glBindVertexArray(VAO);

      auto t2 = std::chrono::high_resolution_clock::now();
      GLint dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

      // Draw our triangle from VBO
      //glBindBuffer(GL_ARRAY_BUFFER, VBO)
      GLint ripplesUniform = glGetUniformLocation(ripplesProgram, "offset");
      glUseProgram(ripplesProgram);
      glUniform1f(ripplesUniform, dt/50);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      // Draw our rectangle from EBO
      //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      GLint barsUniform = glGetUniformLocation(barsProgram, "offset");
      glUseProgram(barsProgram);
      glUniform1f(barsUniform, dt/50);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
      glfwSwapBuffers(window);
      glfwPollEvents();    
    }

  glfwTerminate();
    
  return 0;
}
