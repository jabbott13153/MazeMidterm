#version 430 core

// Send down coordinates for the entire screen so the vs 
// can clear each associated pixel.

void main(void)
{
	const vec4 vertices[] = vec4[](vec4(-1.0, -1.0, 0.5, 1.0),
		vec4(1.0, -1.0, 0.5, 1.0),
		vec4(-1.0, 1.0, 0.5, 1.0),
		vec4(1.0, 1.0, 0.5, 1.0));

	gl_Position = vertices[gl_VertexID];
}
