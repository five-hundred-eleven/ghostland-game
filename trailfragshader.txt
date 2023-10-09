#version 460 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 objectcolor;

void main()
{
    FragColor = vec4(objectcolor, 1.0);
}
