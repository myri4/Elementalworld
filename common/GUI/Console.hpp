#pragma once

#include "Renderer2D.hpp"

namespace wc {
	class Console {
	public:
		float scale = 0.35f;
		uint32_t line = 0;
		glm::vec2 start;

		void DrawTextLine(const std::string& text, const Font& font, const glm::vec4& color = glm::vec4(1.f)) {
			glm::vec2 pos = { start.x, (line * 10.f + 5.f) * scale * 10.f + start.y };

			for (auto& c : text) {
				Character ch = font.Characters[c];

				float xpos = pos.x + ch.Bearing.x * scale;
				float ypos = pos.y - ch.Bearing.y * scale;

				Renderer2D::DrawQuad({ xpos, ypos }, (glm::vec2)ch.Size * scale, ch.texture, color, 1.f);

				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				pos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
			}

			line++;
		}

		void Reset() {
			line = 0;
		}
	};
}