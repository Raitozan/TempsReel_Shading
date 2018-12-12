#version 450

in vec3 positionVertTri;
in vec3 normalVertTris;

uniform mat4 transfo;
uniform mat4 view;
uniform mat4 proj;


void main()
{
    gl_Position = proj * view * transfo * vec4(positionVertTri, 1.0);
}