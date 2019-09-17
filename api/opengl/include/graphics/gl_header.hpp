#pragma once

#define GL_FUNC(x, y) extern PFN##y##PROC x;
#include "graphics/gl_functions.hpp"
#undef GL_FUNC

namespace ignis {

	extern HashMap<String, void**> glFunctionNames;

	enum class DepthFormat : u8;
	enum class GPUFormat : u16;
}

extern void glBeginRenderPass(
	ignis::Graphics::Data &data, const Vec4u &renderArea, const Vec2u &size, GLuint framebuffer
);

extern void APIENTRY glDebugMessage(
	GLenum, GLenum, GLuint, GLenum,
	GLsizei, const GLchar*, const void*
);

extern GLenum glDepthFormat(ignis::DepthFormat format);
extern GLenum glColorFormat(ignis::GPUFormat format);