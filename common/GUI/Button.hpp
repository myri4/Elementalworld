#ifndef BUTTON_HPP
#define BUTTON_HPP
#include <gl/Texture.hpp>
#include <gl/Vertex.hpp>
#include <gl/Shaders.hpp>
#include <gl/IndexBuffer.hpp>

namespace wc {
	class Button {
	private:
		gl::Texture guiTex;
		gl::VertexBuffer quadVBO;
		gl::VertexArray quadVAO;
		gl::Shader quadShader;
		gl::IndexBuffer quadEBO;
	public:
		Button() {

		}
		
		~Button() = default;

		void Create() {

		}

	};
}
#endif