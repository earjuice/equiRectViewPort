#version 150

uniform samplerCube uCubeMapTex;

in vec3	vPosition;

out vec4 	oColor;

void main( void )
{
	oColor = texture( uCubeMapTex, vPosition );
}