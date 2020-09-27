#pragma once

#include <gl/Text.h>

class Renderer2D {
public:
    Renderer2D() {

    }
    void Init() {
        shader.Create("shaderpacks/default/text.vs", "shaderpacks/default/text.fs");
    }
    void ShutDown() {

    }
	void DrawQuad() {

	}
	void Drawtext(gl::Font& font, std::string text, glm::vec2 pos = { 0,0 }, float scale = 1.0f, glm::vec3 color = { 0,0,0 }, int activeTexture = 0) {
        glDisable(GL_DEPTH_TEST);
        // activate corresponding render state	
        shader.use();
        shader.setVec3("textColor", color);
        glActiveTexture(GL_TEXTURE0 + activeTexture);
        font.TextVB.Bind();

        // iterate through all characters
        //for (std::string::const_iterator c = text.begin(); c != text.end(); c++) 
        for (auto c : text) {
            gl::Character ch = font.Characters[c];

            float xpos = pos.x + ch.Bearing.x * scale;
            float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;
            // update VBO for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos,       0.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f }
            };
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            // update content of VBO memory
            font.TextVB.Update(0, sizeof(vertices), vertices);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            pos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
	}
private:
    gl::Shader shader;
};