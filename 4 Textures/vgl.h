/* vova gl, functions generalizing repetitive actions */

#include <iostream>

GLuint vglLoadTexture(const char* path, const char* sampler_name, GLuint shader_program, GLuint texture_unit, GLenum format) {
  // prepare our texture
  GLuint texture;  
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
  if (!data) {
    std::cerr << "vglLoadTexture oof: cannot load texture " << path << '\n';
    throw std::invalid_argument{path};
  }
  //std::cout << "Image loaded with width = " << width << " and height = " << height << '\n';
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  GLint texture_unit_location;
  texture_unit_location = glGetUniformLocation(shader_program, sampler_name);
  if (texture_unit_location < 0) {
    if (texture_unit_location == -1) {
      std::cerr << "vglLoadTexture oof: couldn't find find " << sampler_name << ".\n";
    }
    std::cerr << "glGetUniformLocation oof\n";
    return 1;
  }
  glUniform1i(texture_unit_location, texture_unit);

  return texture;
}
