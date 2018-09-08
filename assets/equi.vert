// passthrough vertex shader
// no vertex transformation

#version 410 core
uniform mat4 ciModelViewProjection;
in vec4			ciPosition;
in vec2			ciTexCoord0;
out highp vec2 texCoord;
//in vec3			ciColor;
//out vec3		vColor;
out float faceNum;
void main() {
	//vColor		= ciColor;
	texCoord=ciTexCoord0;
	gl_Position = ciModelViewProjection * ciPosition;
} 