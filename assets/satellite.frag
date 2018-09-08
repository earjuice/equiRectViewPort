#version 150


in vec3	vPosition;
in vec4 gColor;

out vec4 	oColor;

void main( void )
{
	oColor = gColor;
}