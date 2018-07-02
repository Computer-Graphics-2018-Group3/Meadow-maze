#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;

const vec3 objectColor = vec3(1.0f, 0.25f, 0.25f);
const vec3 lightColor = vec3(1.0f, 0.98f, 0.8f);

const float ambientStrength = 0.2f;
const float diffuseStrength = 0.6f;
const float specularStrength = 0.1f;
const float shininess = 2.0f;

void main() {
	// 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuseStrength * diff * lightColor;

	// 镜面反射
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0f);
}