#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D ourTexture;

out vec4 color;

void main()
{
    color = texture(ourTexture, TexCoord);
}
