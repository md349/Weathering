//Shader from GLSL lecture on 20/11/2014

#version 400 core

layout (location = 0) in vec3 inVert;
layout (location = 1) in vec3 inUV;
layout (location = 2) in vec3 inNormal;

uniform mat4 MVP;

out vec3 normal;

out vec2 vertUV;


void main()
{
    normal = inNormal;
    gl_Position = MVP*vec4(inVert, 1.0);
    vertUV=inUV.st;
}
