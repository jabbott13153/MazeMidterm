#version 430 core

// 2D image to store head pointers
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

layout(location = 0) out vec4 color;
//vec4 color;
const uint max_fragments = 16;

void main(void)
{
	uint frag_count = 1;
	float depth_accum = 0.0;

	// Get the current x,y coordinate of the fragment
	ivec2 P = ivec2(gl_FragCoord.xy);

	// Look up the index of the start of the linked list of color/depth 
	// information for this particular point on the frame
	uint index = imageLoad(head_pointer, P).x;

	// Get the color/depth items one at a time from the linked list
	// as long as we don't hit the end of the linked list or exceed
	// max_fragments
	while (index != 0xFFFFFFFF && frag_count < max_fragments)
	{
		// Get the object from the linked list
		list_item this_item = item[index];

		// accumulate the colors from each item in linked list
		color += this_item.color;

		// accumulate the depth from each item in linked list
		depth_accum += this_item.depth;
		
		// What's the next item in the linked list?
		index = this_item.next;

		// Count this item
		frag_count++;
	}

	// Let's use the depth accumulator to create a green enhancement color
	vec4 green_enhance = normalize( vec4(0.0, depth_accum, 0.0, 1.0) );

	// Take 5% of the green_enhancement and add it to the accumulated color 
	// then ship color out to the frame buffer
	color = color + 0.05 * green_enhance;
}
