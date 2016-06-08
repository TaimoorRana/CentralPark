#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 offset;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

void main()
{
    Normal = normal;
    FragPos = vec3(model * vec4(position + offset,1.0f));
    TexCoord = texCoord;
    gl_Position = projection * view * model * vec4(position + offset, 1.0f);
}