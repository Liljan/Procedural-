#version 330 core

layout(location = 0) in vec3 Position;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 pos;

void main(){

  pos = Position;

  gl_Position = (P * V * M) * vec4(pos, 1.0);
}