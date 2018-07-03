#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform vec3 lightPos;
uniform vec3 viewPos;

const vec3 lightColor = vec3(1.0f, 0.98f, 0.8f);

const float ambientStrength = 0.2f;
const float diffuseStrength = 0.6f;
const float specularStrength = 0.1f;
const float shininess = 2.0f;

void main() {
    // ambient
    vec3 ambient = ambientStrength * texture(texture_diffuse1, fs_in.TexCoords).rgb;
    
    // diffuse 
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * texture(texture_diffuse1, fs_in.TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * texture(texture_specular1, fs_in.TexCoords).rgb;  

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0f);
}