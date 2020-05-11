#include "graphics/memory/primitive_buffer.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "utils/hash.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	BufferLayout::BufferLayout(GPUBuffer *b, const BufferAttributes &formats, usz bufferOffset) :
		formats(formats), buffer(b), bufferOffset(bufferOffset),
		elements(u32(b->size() / formats.getStride())) {}

	PrimitiveBuffer::PrimitiveBuffer(
		Graphics &g, const String &name, const Info &inf
	):
		GPUObject(g, name, GPUObjectType::PRIMITIVE_BUFFER), info(inf)
	{
		usz i{}, elements{};

		for (auto &it : info.vertexLayout) {

			if (!it.buffer) {

				it.buffer = new GPUBuffer(g, NAME(name + " vbo " + std::to_string(i)),
					GPUBuffer::Info(
						it.initData,
						GPUBufferType::VERTEX,
						info.usage
					)
				);

				it.initData.clear();

				if(!it.size())
					oic::System::log()->fatal("Invalid primitive buffer size");

				if (!elements)
					elements = it.elements;

				else if(elements != it.elements)
					oic::System::log()->fatal("Invalid primitive buffer size");

			} else if(it.buffer->getInfo().type != GPUBufferType::VERTEX)
				oic::System::log()->fatal("Invalid predefined vertex buffer");

			else {

				if (!elements)
					elements = it.elements;

				if (elements != it.elements || !elements)
					oic::System::log()->fatal("Invalid predefined vertex buffer size");
			}

			if (it.buffer->size() != it.size())
				oic::System::log()->fatal("Invalid primitive buffer size");

			it.buffer->addRef();

			++i;
		}

		if(info.vertexLayout.size() == 0)
			oic::System::log()->fatal("Primitive buffer requires at least one vertex buffer");

		if (hasIndices()) {

			if (!info.indexLayout.buffer) {

				info.indexLayout.buffer = new GPUBuffer(
					g, NAME(name + " ibo"),
					GPUBuffer::Info(
						info.indexLayout.initData,
						GPUBufferType::INDEX,
						info.usage
					)
				);

				info.indexLayout.initData.clear();

			} else if (info.indexLayout.buffer->getInfo().type != GPUBufferType::INDEX)
				oic::System::log()->fatal("Invalid predefined index buffer");

			else
				info.indexLayout.buffer->addRef();

			if(info.indexLayout.formats.size() != 1)
				oic::System::log()->fatal("Index buffer requires one format");

			switch (info.indexLayout.formats[0].format) {
				
				case GPUFormat::R32u:
				case GPUFormat::R32i:
				case GPUFormat::R16u:
				case GPUFormat::R16i:
					break;

				default:
					oic::System::log()->fatal("Index buffer requires a 32 or 16 bit (unsigned) int format");
			}
		}
	}

	PrimitiveBuffer::~PrimitiveBuffer() {

		for (auto &elem : info.vertexLayout)
			elem.buffer->loseRef();

		if(GPUBuffer *buf = info.indexLayout.buffer)
			buf->loseRef();
	}

	bool PrimitiveBuffer::matchLayout(const List<BufferAttributes> &layout) const {

		for (usz i = 0, j = layout.size(); i < j; ++i)
			if (layout[i] != info.vertexLayout[i].formats)
				return false;

		return true;
	}
}