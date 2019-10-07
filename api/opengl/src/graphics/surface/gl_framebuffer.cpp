#include "utils/hash.hpp"
#include "utils/math.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/surface/gl_framebuffer.hpp"
#include "graphics/format.hpp"

namespace ignis {

	Framebuffer::Framebuffer(Graphics &g, const String &name, const Info &info): 
		Surface(g, name, info) {
		
		data = new Data();
		this->info.samples = oic::Math::min(g.getData()->maxSamples, info.samples);
	}

	Framebuffer::~Framebuffer() {
		onResize(Vec2u());
		destroy(data);
	}

	void Framebuffer::onResize(const Vec2u &siz) {

		Vec2u size{ u32(siz[0] * info.viewportScale), u32(siz[1] * info.viewportScale) };

		if (info.size == size || !info.isDynamic)
			return;

		info.size = size;

		if (data->index)
			glDeleteFramebuffers(1, &data->index);

		if (data->depth) {

			if (info.keepDepth)
				glDeleteTextures(1, &data->depth);
			else
				glDeleteRenderbuffers(1, &data->depth);
		}

		for (GLuint &renderTexture : data->renderTextures)
			if (renderTexture)
				glDeleteTextures(1, &renderTexture);

		if (!size[0]) {
			data->index = data->depth = 0;
			return;
		}

		glGenFramebuffers(1, &data->index);
		glBindFramebuffer(GL_FRAMEBUFFER, data->index);
		glObjectLabel(
			GL_FRAMEBUFFER, data->index, GLsizei(getName().size()), getName().c_str()
		);

		if (info.depthFormat != DepthFormat::NONE) {

			if (info.keepDepth) {

				glGenTextures(1, &data->depth);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, data->depth);

				String hashed = NAME(getName() + " depth texture");
				glObjectLabel(
					GL_TEXTURE, data->depth, GLsizei(hashed.size()), hashed.c_str()
				);

				glTexImage2DMultisample(
					GL_TEXTURE_2D_MULTISAMPLE, GLsizei(info.samples),
					glDepthFormat(info.depthFormat), size[0], size[1], GL_FALSE
				);

				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, data->depth, 0);

			} else {

				glGenRenderbuffers(1, &data->depth);
				glBindRenderBuffer(GL_RENDERBUFFER, data->depth);

				String hashed = NAME(getName() + " depth buffer");
				glObjectLabel(
					GL_RENDERBUFFER, data->depth, GLsizei(hashed.size()), hashed.c_str()
				);

				glRenderbufferStorageMultisample(
					GL_RENDERBUFFER, GLsizei(info.samples),
					glDepthFormat(info.depthFormat), size[0], size[1]
				);

				glFramebufferRenderbuffer(
					GL_FRAMEBUFFER,
					FormatHelper::hasStencil(info.depthFormat) ?
					GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT,
					GL_RENDERBUFFER,
					data->depth
				);
			}
		}

		data->renderTextures.resize(info.colorFormats.size());
		List<GLenum> drawBuffers(info.colorFormats.size());

		for (usz i = 0, j = info.colorFormats.size(); i < j; ++i) {

			glGenTextures(1, data->renderTextures.data() + i);
			GLuint tex = data->renderTextures[i];

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
			glTexImage2DMultisample(
				GL_TEXTURE_2D_MULTISAMPLE, GLsizei(info.samples),
				glColorFormat(info.colorFormats[i]), size[0], size[1], GL_FALSE
			);

			String hashed = NAME(getName() + " buffer " + std::to_string(i));
			glObjectLabel(GL_TEXTURE, tex, GLsizei(hashed.size()), hashed.c_str());

			glFramebufferTexture(GL_FRAMEBUFFER, drawBuffers[i] = GL_COLOR_ATTACHMENT0 + GLenum(i), tex, 0);
		}

		glDrawBuffers(GLsizei(drawBuffers.size()), drawBuffers.data());
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE)
			oic::System::log()->fatal("Couldn't create framebuffer");

	}

	void Framebuffer::begin(const Vec4u &area) {
		glBeginRenderPass(*getGraphics().getData(), area, info.size, data->index);
	}

	void Framebuffer::end() {}

}