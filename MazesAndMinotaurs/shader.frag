#version 330
smooth in vec2 vTexCoord;
out vec4 vFragColor;

uniform sampler2D textureMap;

void main(void)
{
   vFragColor = 0.5*vec4(0.7, 0.3, 0.8, 1.0) + texture(textureMap, vTexCoord);
}