#include "LineRenderer.h"

void LineRenderer::ensure_capacity(int extra)
{
	if (buffer_size < verts.size() + extra) {
		//we need to grow the buffer
		//1 double the size and create new
		//2 delete old ones--- if we are currently render from one does that go bad? somehow defere destruction when that buffer is not used?
	}
}

void LineRenderer::DrawLine(vec3_t from, vec3_t to, uint32_t color)
{
	ensure_capacity(2);
	verts.emplace_back(from, color);
	verts.emplace_back(to, color);

}

void LineRenderer::Draw(VkCommandBuffer cmd)
{
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);


}
