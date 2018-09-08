#version 150

uniform mat4	ciModelViewProjection;
uniform mat4 ciModelMatrix; 

in vec4			ciPosition;
in vec4			ciColor;

out  vec4   vColor;

//out highp vec3	NormalWorldSpace;

void main( void )
{
//	NormalWorldSpace = vec3( ciPosition );
	vColor=ciColor;
	//gl_Position = ciModelViewProjection * ciPosition;
	gl_Position	= ciModelMatrix*ciPosition;

}
