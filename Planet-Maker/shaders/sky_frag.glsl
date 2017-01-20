#version 330 core

in vec3 interpolatedNormal;
in vec3 pos;

out vec4 color;

void main() {

  vec3 diffusecolor = vec3(0.1,0.2,0.5);

  color = vec4(diffusecolor, 0.3);
}