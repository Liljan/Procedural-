#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

uniform mat4 MV;
uniform mat4 P;

uniform float radius;
uniform float elevationModifier;

out vec3 interpolatedNormal;

out vec3 pos;

void main(){
  float height = elevationModifier;

  pos = Position + radius * Normal;
  pos += height * Normal;

  gl_Position = (P * MV) * vec4(pos, 1.0);

  interpolatedNormal = mat3(MV) * Normal;
}