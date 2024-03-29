#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset;
uniform vec4 color;

void main()
{
    float scale = 0.07f;
    TexCoords = texCoords;
    ParticleColor = color;
    gl_Position = projection * view * vec4((position * scale) + offset, 1.0);
}