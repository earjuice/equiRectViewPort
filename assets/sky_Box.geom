// instanced geometry shader
// invocated once for each of the 4 viewports
// takes care of setting the viewport and transforming the vertices/primitives

#version 410 core
#define NUMVIEWS 6

layout(triangles, invocations=NUMVIEWS) in;
layout(triangle_strip, max_vertices=3) out;

uniform mat4 uMatrices[NUMVIEWS];

//in vec3 vColor[];
//out vec3 gColor;
in highp vec3	NormalWorldSpace[];
out vec3 vPosition;

void main() {
	for( int i = 0; i < gl_in.length(); ++i ) {
		// transform the position here instead of in the vertex shader
		vPosition			=vec3(gl_in[i].gl_Position);// NormalWorldSpace[i];
		gl_Position			= uMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		// set the current viewport for this vertex
		gl_ViewportIndex	= gl_InvocationID + 1;
		//gColor				= vColor[i];

		EmitVertex();
	}
	EndPrimitive();
} 