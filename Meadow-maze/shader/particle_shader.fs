#version 330 core
in vec2 TexCoords;
in vec4 ParticleColor;
out vec4 color;

uniform sampler2D sprite;

void main()
{
	if(texture(sprite, TexCoords).a < 0.5)
		discard;
    color = (texture(sprite, TexCoords) * ParticleColor);
}