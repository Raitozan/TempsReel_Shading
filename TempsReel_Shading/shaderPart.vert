#version 450

in vec3 colorParticule;
in vec3 positionParticule;
out vec3 colorIn;

void main()
{
    gl_Position = vec4(positionParticule, 1.0);
    colorIn = colorParticule;
}