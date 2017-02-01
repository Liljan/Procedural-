#version 330 core

// SMART NOISE STUFF

//
// GLSL textureless classic 3D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author:  Stefan Gustavson (stefan.gustavson@liu.se)
// Version: 2011-10-11
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/ashima/webgl-noise
//

vec3 mod289(vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec3 P)
{
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod289(Pi0);
  Pi1 = mod289(Pi1);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 * (1.0 / 7.0);
  vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 * (1.0 / 7.0);
  vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

// Classic Perlin noise, periodic variant
float pnoise(vec3 P, vec3 rep)
{
  vec3 Pi0 = mod(floor(P), rep); // Integer part, modulo period
  vec3 Pi1 = mod(Pi0 + vec3(1.0), rep); // Integer part + 1, mod period
  Pi0 = mod289(Pi0);
  Pi1 = mod289(Pi1);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 * (1.0 / 7.0);
  vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 * (1.0 / 7.0);
  vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

// END OF SMART NOISE FUNCTIONS


// MY NOT SO GLORIOUS CODE BEGINS

in vec3 interpolatedNormal;
in float height;

in vec3 camPos;
in vec3 pos;

uniform float light_intensity;
uniform vec3 light_pos;
uniform float shininess;

uniform vec3 color_deep; // water color of the planet
uniform vec3 color_beach; // beach color of the planet
uniform vec3 color_grass; // grass color of the planet
uniform vec3 color_rock; // rock color of the planet
uniform vec3 color_snow; // snow color of the planet

uniform int seed;

uniform float frag_frequency;
uniform int octaves;

out vec4 color;

// height ranges

vec2 deep_range = vec2(0.0,0.005);
vec2 beach_range = vec2(0.005,0.2);
vec2 grass_range = vec2(0.2,0.625);
vec2 rock_range = vec2(0.5,0.75);
vec2 snow_range = vec2(-0.7,1.0);

void main() {

  //vec3 mixLava = mix(color_low,color_glow,cnoise( vec3(height) * 0.8));
  //vec3 mixMtn = mix(color_high,color_med, cnoise( 2.0 * vec3(10.0*height) ) );

  vec3 watermix;
  vec3 groundmix;
  vec3 mountainmix;

  float noise = cnoise(frag_frequency*vec3(pos + seed));

  // 1th to (n-1):th octave
  for(float o = 1.0; o < octaves; o++)
  {
    noise += 1.0 / (pow(2,o)) * cnoise((o+1.0)*frag_frequency*vec3(pos + seed));
  }

  /*
  watermix = mix(color_water_1, color_water_2, noise);
  groundmix = mix(color_ground_1,color_ground_2, noise);
  mountainmix = mix(color_mountain_1,color_mountain_2, noise);
  */

  vec3 diffusecolor;



  //diffusecolor = watermix;

  // height: from 0 to 1
  float int_dir = 0.02;
  /*float deep = smoothstep(deep_range.x - 0.1, deep_range.x, height) - smoothstep(deep_range.y - 0.1, deep_range.y, height);
  float beach = smoothstep(beach_range.x - int_dir, beach_range.x, height) - smoothstep(beach_range.y - int_dir, beach_range.y, height);
  float grass = smoothstep(grass_range.x - int_dir, grass_range.x, height) - smoothstep(grass_range.y - int_dir, grass_range.y, height);
  float rock = smoothstep(rock_range.x - int_dir, rock_range.x, height) - smoothstep(rock_range.y - int_dir, rock_range.y, height);
  float snow = smoothstep(snow_range.x - int_dir, snow_range.x, height) - smoothstep(snow_range.y - int_dir, snow_range.y, height);*/

  /*diffusecolor = mix(color_deep,color_beach,beach);
  diffusecolor = mix(diffusecolor, color_grass, grass);
  diffusecolor = mix(diffusecolor, color_rock, rock);
  diffusecolor = mix(diffusecolor, color_snow, snow);*/


  //float beach = smoothstep(0.0,0.1,height) - smoothstep(0.11,0.25,height);
  float beach = smoothstep(0.0,0.2,height);
  float grass = smoothstep(0.15,0.3,height);
  float rock = smoothstep(0.25,0.8,height);
  float snow = smoothstep(0.7, 0.85, height);
  //float snow = mix()

  diffusecolor = mix(color_deep, color_beach, beach);
  diffusecolor = mix(diffusecolor, color_grass, grass);
  diffusecolor = mix(diffusecolor, color_rock, rock);
  diffusecolor = mix(diffusecolor, color_snow, snow);


 /* float low = smoothstep(deep_range.x - 0.1, deep_range.x, height) - smoothstep(deep_range.y - 0.1, deep_range.y, height);
  float high = smoothstep(snow_range.x - int_dir, snow_range.x, height) - smoothstep(snow_range.y - int_dir, snow_range.y, height);

  diffusecolor = mix(color_deep,color_snow,low);
*/
/*
  if(height < 0.005)
    diffusecolor = watermix;
  else if(height < 0.1)
    diffusecolor = groundmix;
  else if(height < 0.375)
    diffusecolor = color_ground_1;
  else if(height < 0.625)
    diffusecolor = color_ground_2;
  else if(height < 0.75)
    diffusecolor = color_mountain_1;
  else
    diffusecolor = color_mountain_2;
    
  //diffusecolor = vec3(0.1,0.2,0.4);

  vec3 kd = vec3(0.7,0.7,0.7);
  vec3 ka = vec3(0.1,0.1,0.1);
  vec3 ks = vec3(0.2,0.2,0.2);

  vec3 normal = normalize(interpolatedNormal);
  vec3 viewDir = normalize(camPos);

  vec3 s = normalize(vec3(light_pos) - pos);
  vec3 r = reflect(-s,normal);

  vec3 ambient = ka * light_intensity; // * intensity
  vec3 diffuse = kd * max(dot(s,normal), 0.0) * light_intensity; // * intensity
  vec3 specular = ks * pow( max( dot(r,viewDir),0.0 ) , shininess);

  vec3 diffuselighting = diffusecolor * (ka + kd);

  if(height < 0.1)
    color = vec4(diffuselighting + specular, 1.0);
  else
    color = vec4(diffuselighting, 1.0);
    */

    color = vec4(diffusecolor,1.0);
}