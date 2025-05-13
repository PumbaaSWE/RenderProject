#pragma once
#include "stuff.h"


struct LineVertex {
	vec3_t pos;
	int color;
};

class LineRenderer
{
	VkPipeline pipeline;
	std::vector<LineVertex> verts;
	int buffer_size;

	//we need N nbr of buffers due to FRAMES IN FLIGHT
	//one buffer might be on GPU being rendered and written to by CPU at the same time otherwise 
	//buffer passed to cmdList and passed to GPU, execution continues on CPU...CPU updates buffer "next frame"
	//means we have to have one buffer per frame -.- or delay cpu (gameloop tick) until we done rendering this frame
	//or not because the submit2 command blocks untill command is done? or just 

	void ensure_capacity(int extra);
	void DrawLine(vec3_t from, vec3_t to, uint32_t color);

	void Draw(VkCommandBuffer cmd);
};

