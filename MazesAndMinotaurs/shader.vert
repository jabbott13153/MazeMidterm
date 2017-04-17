#version 330
 
in vec2 vUV;
in vec3 vVertex;
smooth out vec2 vTexCoord;

uniform mat4 MVP;
void main()
{
   vTexCoord = vUV;
   gl_Position = vec4(vVertex,1)*MVP;
}