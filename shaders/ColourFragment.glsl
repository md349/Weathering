//Shader from GLSL lecture on 20/11/2014

#version 400 core
layout (location = 0) out vec4 fragColour;

uniform vec3 lightPos;

in vec3 normal;

void main()
{
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos);
    fragColour = vec4(0.0);
    float I = dot(L,N);
    fragColour.rgb = vec3(I);
}
