#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

uniform mat4 MV;
uniform mat4 P;

out vec3 interpolatedNormal;

out vec3 pos;

void main(){

  pos = Position;

  gl_Position = (P * MV) * vec4(pos, 1.0);

  interpolatedNormal = mat3(MV) * Normal;
}