#version 330 core

//输入，来自顶点着色器的颜色
in vec3 fragmentColor;

//输入，来自顶点着色器的纹理坐标
in vec2 fragmentUV;

//输入，来自顶点着色器的世界坐标的顶点位置
in vec3 fragmentPos;

//输入，来自顶点着色器的法向量
in vec3 fragmentNormal;

//输出
out vec4 color;

//纹理图案对象
uniform sampler2D myTextureSampler;

struct Light
{
	//光源的色彩
	vec3 lightColor;
	//光源位置
	vec3 lightPos;
};

uniform Light light;

//相机位置
uniform vec3 viewPos;

struct Material
{
    float ambientStrength;
    float diffuse;
    float specularStrength;
    float shininess;
};

uniform Material material;

void main(){
	
	//环境光
	float ambient=material.ambientStrength;
	
	//光源入射方向
	vec3 lightDir = normalize(light.lightPos - fragmentPos);
	//漫反射
	float diffuse= max(dot(normalize(fragmentNormal), lightDir), 0.0) * material.diffuse;
	
	
	//镜面反射计算公式
	float spec = pow(max(dot(normalize(viewPos - fragmentPos), reflect(-lightDir, normalize(fragmentNormal))), 0.0), material.shininess);
	float specular = material.specularStrength * spec;
	
	
	color =(ambient + diffuse + specular) * vec4(light.lightColor,1.0f) * texture(myTextureSampler,fragmentUV) * vec4(fragmentColor, 1.0f);
	//* texture(myTextureSampler,fragmentUV) 
}