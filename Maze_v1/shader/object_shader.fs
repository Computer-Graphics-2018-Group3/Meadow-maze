#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool isOrthoProj;

const vec3 objectColor = vec3(1.0f, 1.0f, 1.0f);
const vec3 lightColor = vec3(1.0f, 0.98f, 0.8f);

const float ambientStrength = 0.15f;
const float diffuseStrength = 1.0f;
const float specularStrength = 0.5f;
const int shininess = 32;

// 用于柏松取样
const vec2 poissonDisk[16] = vec2[](
	vec2(-0.94201624, -0.39906216), 
	vec2( 0.94558609, -0.76890725), 
	vec2(-0.094184101, -0.92938870), 
	vec2( 0.34495938,  0.29387760), 
	vec2(-0.91588581,  0.45771432), 
	vec2(-0.81544232, -0.87912464), 
	vec2(-0.38277543,  0.27676845), 
	vec2( 0.97484398,  0.75648379), 
	vec2( 0.44323325, -0.97511554), 
	vec2( 0.53742981, -0.47373420), 
	vec2(-0.26496911, -0.41893023), 
	vec2( 0.79197514,  0.19090188), 
	vec2(-0.24188840,  0.99706507), 
	vec2(-0.81409955,  0.91437590), 
	vec2( 0.19984126,  0.78641367), 
	vec2( 0.14383161, -0.14100790) 
);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float ShadowCalculation(vec4 fragPosLightSpace) {
	// 执行透视除法
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// 变换到[0, 1]的范围
	projCoords = projCoords * 0.5f + 0.5f;
	// 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	// 取得当前片元在光源视角下的深度
	float currentDepth = projCoords.z;
	// 检查当前片元是否在阴影中，若在阴影中，返回1.0
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	// 根据平面坡度调整bias的值，防止bias过大导致明显的Peter Panning
    float cosTheta = dot(normal, lightDir);
	float bias = 0.001f * tan(acos(cosTheta));
	bias = clamp(bias, 0.005f, 0.01f);

	// Surrounding Texels Sampling
    float surroundingTexelsShadow = 0.0f; // 周围像素取样阴影值
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0); // 像素大小
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y)*texelSize).r; 
            surroundingTexelsShadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;        
        }    
    }
    surroundingTexelsShadow /= 25.0f; // 共取样25次

    // Poisson Sampling
    float poissonShadow = 0.0f; // 柏松取样阴影值
	for (int i = 0; i < 16; ++i) {
		float pcfDepth = texture(shadowMap, projCoords.xy + poissonDisk[i]/700.0f).r;
		poissonShadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
	}
	poissonShadow /= 16.0f; // 共取样16次

	// 阴影值，取周围像素阴影与柏松阴影的平均值
	float shadow = (surroundingTexelsShadow + poissonShadow) / 2.0f;

    // 视锥外位置shadow值为0
    if (projCoords.z > 1.0f)
        shadow = 0.0f;
        
    return shadow;
}

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

	// 阴影
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
	shadow = min(shadow, 0.75f);

	vec3 result = (ambient + (1.0f-shadow)*(diffuse+specular)) * texture(diffuseTexture, fs_in.TexCoords).rgb * objectColor;
	FragColor = vec4(result, 0.8f);
}