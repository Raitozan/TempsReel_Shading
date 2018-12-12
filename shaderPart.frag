#version 450

in vec3 colorIn;
out vec4 color;

void main()
{
    color = vec4(colorIn, 1);
}