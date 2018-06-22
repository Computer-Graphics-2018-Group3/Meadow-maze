#version 330 core

//第一层缓冲，编号为0，从C++输入数据，顶点位置
layout(location = 0) in vec3 vertexPosition;

//第二层缓冲，编号为1，从C++输入数据，顶点颜色
layout(location = 1) in vec3 vertexColor;

//第三层缓冲，编号为2，从C++输入数据，纹理坐标
layout(location = 2) in vec2 vertexUV;

//法向量
layout(location = 3) in vec3 vertexNormal;

//输出数据， 给片段着色器
out vec3 fragmentColor;

out vec2 fragmentUV;

out vec3 fragmentPos;

out vec3 fragmentNormal;

//静态数据，MVP
uniform mat4 MVP;
uniform mat4 M;

void main(){	

	//顶点位置，参数
	gl_Position =  MVP * vec4(vertexPosition,1);

	//片段颜色，传递给片段着色器
	fragmentColor = vertexColor;
	
	//纹理坐标，传递给片段着色器
	fragmentUV= vertexUV;
	
	//世界坐标的顶点位置
	fragmentPos= vec3( M * vec4(vertexPosition, 1.0f));
	
	//法向量
	fragmentNormal = vertexNormal;
}