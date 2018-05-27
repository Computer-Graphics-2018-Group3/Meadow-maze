//Phong Shading

#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float ambientStrength;
uniform float specularStrength;
uniform float diffStrength;
uniform int Shininess;

uniform int ortho_or_pespective;
uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}
float ShadowCalulation(vec4 FragPosLightSpace, vec3 norm, vec3 lightDir) {
    //perspective case
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    //[-1,1] -> [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) {
        return 0.0;
    }
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    if (ortho_or_pespective == 1) {
        closestDepth = LinearizeDepth(closestDepth);
        currentDepth = LinearizeDepth(currentDepth);
    }
    float bias = max(0.05 * (1.0 - dot(norm, -lightDir)), 0.005);
    //PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            if (ortho_or_pespective == 1) {
                pcfDepth = LinearizeDepth(pcfDepth);
            }
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    return shadow;
    //return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
}

void main()
{
    //
    vec3 objectColor = texture(diffuseTexture, TexCoord).rgb;
    vec3 lightColor = vec3(1.0);
    //ambient
    vec3 ambient = ambientStrength * objectColor;
    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightPos + FragPos);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diffStrength * diff * lightColor;
    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
    vec3 specular = specularStrength * spec * lightColor;

    //calculate shadow
    float shadow = ShadowCalulation(FragPosLightSpace, norm, lightDir);

    //result
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor;
    FragColor = vec4(result, 1.0);
}