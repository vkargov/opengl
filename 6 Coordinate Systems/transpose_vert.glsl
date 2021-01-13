#version 330 core
layout (location = 0) in vec3 aPos;        // the position variable has attribute position 0
layout (location = 1) in vec2 in_tex_coords; // the text coordinates have attribute position 1
  
out vec2 tex_coords; // output texture coordinates

uniform mat4 tm;
uniform float time;

void main()
{
    gl_Position = vec4(tm * vec4(aPos.x, aPos.y, aPos.z, 1));
    tex_coords = in_tex_coords;
}
