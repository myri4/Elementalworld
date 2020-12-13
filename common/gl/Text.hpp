#ifndef TEXT_HPP
#define TEXT_HPP
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <gl/Shaders.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace gl {
        class Character {
        public:
            Character() {}
            Character(const uint32_t& TextureID, const glm::ivec2& Size, const glm::ivec2& Bearing, const uint32_t& Advance) : TextureID(TextureID), Size(Size), Bearing(Bearing), Advance(Advance) {}
            uint32_t TextureID = 0;     // ID handle of the glyph texture
            glm::ivec2   Size = glm::ivec2(0);      // Size of glyph
            glm::ivec2   Bearing = glm::ivec2(0);   // Offset from baseline to left/top of glyph
            uint32_t Advance = 0;       // Horizontal offset to advance to next glyph
        };

    class Text {
    public:
        Text() {}
        void Create(const char* fontFileLoc, const char* Shader, const int& glyphs = 128) {
            shader.Create(Shader);

            // FreeType
            // --------
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            if (FT_Init_FreeType(&ft)) WC_ERROR("Could not init freetype library!");
            

            // find path to font
            if (fontFileLoc == "") WC_ERROR("Could not find font file location!");
            

            // load font as face
            FT_Face face;
            if (FT_New_Face(ft, fontFileLoc, 0, &face)) { WC_ERROR("Failed to load font!"); }
            else {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, 48);

                // disable byte-alignment restriction
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // load first 128 characters of ASCII set
                for (unsigned char c = 0; c < glyphs; c++)
                {
                    // Load character glyph 
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                        WC_ERROR("Failed to load glyph!");
                    }
                    // generate texture
                    uint32_t texture;
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
                    // set texture options
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    // now store character for later use
                    Character character = {
                        texture,
                        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                        static_cast<unsigned int>(face->glyph->advance.x)
                    };
                    Characters.insert(std::pair<char, Character>(c, character));
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            // destroy FreeType once we're finished
            FT_Done_Face(face);
            FT_Done_FreeType(ft);


            // configure VAO/VBO for texture quads
            // -----------------------------------
            TextVA.Create();
            TextVA.Bind();
            TextVB.Create(nullptr, sizeof(float) * 6 * 4, GL_DYNAMIC_DRAW);
            TextVA.VertexAttribPointer(0, 4, 4 * sizeof(float), 0);
        }
        void Create(const char* fontFileLoc, const gl::Shader& Shader, const int& glyphs = 128) {
            this->shader = Shader;

            // FreeType
            // --------
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            if (FT_Init_FreeType(&ft)) WC_ERROR("Could not init freetype library!");


            // find path to font
            if (fontFileLoc == "") WC_ERROR("Could not find font file location!");


            // load font as face
            FT_Face face;
            if (FT_New_Face(ft, fontFileLoc, 0, &face)) { WC_ERROR("Failed to load font!"); }
            else {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, 48);

                // disable byte-alignment restriction
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // load first 128 characters of ASCII set
                for (uint8_t c = 0; c < glyphs; c++)
                {
                    // Load character glyph 
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                        WC_ERROR("Failed to load glyph!");
                    }
                    // generate texture
                    uint32_t texture;
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
                    // set texture options
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    // now store character for later use
                    Character character = {
                        texture,
                        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                        static_cast<uint32_t>(face->glyph->advance.x)
                    };
                    //Characters.insert(std::pair<char, Character>(c, character));
                    Characters[c] = character;
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            // destroy FreeType once we're finished
            FT_Done_Face(face);
            FT_Done_FreeType(ft);


            // configure VAO/VBO for texture quads
            // -----------------------------------
            TextVA.Create();
            TextVA.Bind();
            TextVB.Create(nullptr, sizeof(float) * 6 * 4, GL_DYNAMIC_DRAW);
            TextVA.VertexAttribPointer(0, 4, 4 * sizeof(float), 0);
        }

        void Draw(const std::string& text, const glm::vec2& windowSize, glm::vec2 pos = { 0,0 }, float scale = 0.4f, glm::vec3 color = glm::vec3(0.5, 0.8f, 0.2f)) {
            // activate corresponding render state	
            glm::mat4 projection = glm::ortho(0.0f, windowSize.x, 0.0f, windowSize.y);
            shader.use();
            shader.setMat4("projection", projection);
            shader.setVec3("textColor", color);
            TextVA.Bind();

            // iterate through all characters
            for (auto& c : text){
                Character ch = Characters[c];

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
                    { xpos + w, ypos,       1.0f, 1.0f },
                };
                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                // update content of VBO memory
                TextVB.Update(0, sizeof(vertices), vertices);

                // render quad
                glDrawArrays(GL_TRIANGLES, 0, 6);
                // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                pos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            }
        }
    private:
        gl::Shader shader;
        gl::VertexBuffer TextVB;
        gl::VertexArray TextVA;

        std::unordered_map<char, Character> Characters;
    };
}
#endif