#version 330 core

in vec2 tex_coords;

out vec4 FragColor;

uniform sampler2D pony;
uniform float time;
  
void main()
{
    FragColor = texture(pony, tex_coords);
}