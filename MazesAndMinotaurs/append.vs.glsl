#version 430 core
 
in vec2 vUV;
in vec3 vVertex;

// output to the fragment shader
out VS_OUT
{
	vec4 pos;
	smooth out vec2 map_coord;
} vs_out;

uniform mat4 MVP;
void main()
{
   vs_out.map_coord = vUV;
   vs_out.pos = vec4(vVertex,1)*MVP;
   gl_Position = vs_out.pos;
}