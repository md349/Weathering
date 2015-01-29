//Shader from GLSL lecture on 20/11/2014 and Jon Macey's texture NGL demo

#version 400 core
layout (location = 0) out vec4 fragColour;

uniform sampler2D tex;

uniform vec3 lightPos;

in vec3 normal;

in vec2 vertUV;

void main()
{
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos);
    fragColour = vec4(0.0);
    float I = dot(L,N);
    fragColour.rgb = vec3(I);
    fragColour = texture(tex,vertUV);
}
