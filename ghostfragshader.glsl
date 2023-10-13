#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture0;

void main() {
    vec4 outcolor = texture(texture0, TexCoord);
    outcolor.a = outcolor.a * 0.3;
    FragColor = outcolor;
}
