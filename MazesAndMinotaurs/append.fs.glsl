#version 430 core

// Atomic counter for filled size
layout(binding = 0, offset = 0) uniform atomic_uint fill_counter;

// 2D image to store head pointers 32bit unsigned int will hold the pointer address
layout(binding = 0, r32ui) coherent uniform uimage2D head_pointer;

// Shader storage buffer containing appended fragments
struct list_item
{
	vec4        color;
	float       depth;
	uint        next;
};

layout(binding = 0, std430) buffer list_item_block
{
	list_item   item[];
};

// input from the vertex shader
in VS_OUT
{
	vec4 pos;
	smooth in vec2 map_coord;
} fs_in;

uniform sampler2D textureMap;

layout(location = 0) out vec4 color;

void main(void)
{
	// Make sure the fragment is front facing
	if (gl_FrontFacing) {

		// Get the xy coordinate of this fragment
		ivec2 P = ivec2(gl_FragCoord.xy);

		// Lookup current fragment color from texture map 
		vec4 color = texture(textureMap, fs_in.map_coord);

		// If the color we looked up is not transparent add the color (and depth information)
		// to a linked list of color/depth information
		if (color.a > 0.0) {

			// Count the pixels with a critical section and assign index to this pixel
			// index is essentially a pointer to this particular pixel
			uint index = atomicCounterIncrement(fill_counter);

			// Now go to the head_pointer image buffer at pixel P, grab the current value
			// put it in old_head, and replace it atomically with the new value index
			uint old_head = imageAtomicExchange(head_pointer, P, index);

			// Store the current fragment color/depth in a linked list 
			// of fragment information objects
			item[index].color = texture(textureMap, fs_in.map_coord);
			item[index].depth = gl_FragCoord.z;
			item[index].next = old_head;
		}
	}
}