// passthrough fragment shader
#version 410 core

uniform vec3			iResolution; // viewport resolution (in pixels)
uniform float			uAspectRatio;

uniform samplerCube		uTex0;

in vec2		texCoord;
out vec4	fragColor;

void main() {
 // vec2 texCoord = fragCoord.xy / iResolution.xy; 

 //	vec2 uv = texCoord.xy/ iResolution.xy - vec2( 0.5 );
 //   uv.x *= uAspectRatio;

	vec2 uv = ((texCoord * 2.0) - vec2(1.0));
	//uv.x *= uAspectRatio;
//	uv.y*=1.5;
    vec2 thetaphi = uv * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398); 
    vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));

	fragColor = texture(uTex0, rayDirection);

	//fragColor	= texture( uTex0, TexCoord.st );//vec4( gColor, 1.0 );
}