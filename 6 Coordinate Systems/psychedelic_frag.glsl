#version 330 core

in vec2 tex_coords;

out vec4 FragColor;

uniform sampler2D pony;
uniform float time;
uniform vec2 center = vec2(1, 2);
  
void main()
{
    float dist = distance(center, vec2(gl_FragCoord.x, gl_FragCoord.y));
    float color = abs(sin(dist*0.1-float(time)*0.1));
    FragColor = vec4(0, color, 0, 1.0f);
}