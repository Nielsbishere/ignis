#pragma once 
#include "types/types.hpp"
#include "graphics/graphics.hpp"

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN

	#include <Windows.h>
	#include <gl/GL.h>
	#include "graphics/wglext.h"

	#undef ERROR
	#undef far
	#undef near
	#undef min
	#undef max
#endif

#include "glext.h"

namespace ignis {

	class Surface;
	class Swapchain;
	class PrimitiveBuffer;
	class Pipeline;
	class Descriptors;

	enum class CullMode : u8;
	enum class WindMode : u8;
	enum class FillMode : u8;

	//Graphics data

	struct BoundRange {
		GLuint handle{};
		usz offset{}, size{};
	};

	struct Graphics::Data {

		Surface *currentSurface{};
		Swapchain *swapchain{};
		PrimitiveBuffer *primitiveBuffer{};
		Pipeline *pipeline{};
		Descriptors *descriptors{};

		f32 depth{};
		u32 stencil{};

		CullMode cullMode{};
		WindMode windMode{};
		FillMode fillMode{};

		HashMap<GLenum, GLuint> bound;
		HashMap<u64, BoundRange> boundByBase;	//GLenum lower 32-bit, Base upper 32-bit

		Vec4u viewport{}, scissor{};

		Vec4f clearColor{};

		u32 maxSamples;

		u32 major, minor;
		bool isES{}, scissorEnable{};

		static inline constexpr u64 getVersion(u32 major, u32 minor) {
			return (u64(major) << 32) | minor;
		}

		//Detect if it is the current version or higher
		inline const bool version(u32 maj, u32 min) const {
			return getVersion(major, minor) >= getVersion(maj, min);
		}
		
		inline void bind(void (*bindFunc)(GLenum, GLuint), GLenum where, GLuint what) {
			
			auto it = bound.find(where);

			if (it == bound.end() || it->second != what) {
				bindFunc(where, what);
				bound[where] = what;
			}
		}
	};

}

#include "gl_header.hpp"