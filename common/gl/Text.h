#pragma once
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <gl/Shaders.h>
#include <gl/glErrors.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace gl {
    enum class TextStatus {OK, COULD_NOT_INIT_FREETYPE_LIB, FAILED_TO_FIND_FONT, FAILED_TO_LOAD_FONT, FAILED_TO_LOAD_GLYPH};
        class Character {
        public:
            Character() {

            }
            Character(uint32_t TextureID, glm::ivec2 Size, glm::ivec2 Bearing, uint32_t Advance) : TextureID(TextureID), Size(Size), Bearing(Bearing), Advance(Advance) {}
            ~Character() {

            }
            uint32_t TextureID;     // ID handle of the glyph texture
            glm::ivec2   Size;      // Size of glyph
            glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
            uint32_t Advance;       // Horizontal offset to advance to next glyph
        };
    class Font{
    public:
        Font() {
        
        }
        Font(const char* fontFileLoc) {
            Create(fontFileLoc);
        }
        ~Font() {

        }
        TextStatus Create(const char* fontFileLoc, int glyphs = 128) {
            // FreeType
            // --------
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            if (FT_Init_FreeType(&ft)) return TextStatus::COULD_NOT_INIT_FREETYPE_LIB;


            // find path to font
            if (fontFileLoc == "") return TextStatus::FAILED_TO_FIND_FONT;


            // load font as face
            FT_Face face;
            if (FT_New_Face(ft, fontFileLoc, 0, &face)) return TextStatus::FAILED_TO_LOAD_FONT;
            else {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, 48);

                // disable byte-alignment restriction
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // load first 128 characters of ASCII set
                for (unsigned char c = 0; c < glyphs; c++){
                    // Load character glyph 
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
                        return TextStatus::FAILED_TO_LOAD_GLYPH;
                    
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
                    Characters.insert(std::pair<char, Character>(c, 
                    Character(texture,
                        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                        static_cast<unsigned int>(face->glyph->advance.x))));
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            // destroy FreeType once we're finished
            FT_Done_Face(face);
            FT_Done_FreeType(ft);


            // configure VAO/VBO for texture quads
            // -----------------------------------
            TextVB.Create(nullptr, sizeof(float) * 6 * 4, GL_DYNAMIC_DRAW);
            gl::VertexAttribPointer(0, 4, 4 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            return TextStatus::OK;
        }
    protected:
        gl::VertexBuffer TextVB;

        std::map<char, Character> Characters;
    };
    class Text {
    public:
        Text() {

        }
        Text(const char* fontFileLoc, const char* vs, const char* fs, glm::mat4 projection) {
            Create(fontFileLoc, vs, fs, projection);
        }
        ~Text() {

        }
        TextStatus Create(const char* fontFileLoc, const char* vs, const char* fs, glm::mat4 projection,int glyphs = 128) {
            shader.Create(vs, fs);
            shader.use();
            shader.setMat4("projection", projection);

            // FreeType
            // --------
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            if (FT_Init_FreeType(&ft)) return TextStatus::COULD_NOT_INIT_FREETYPE_LIB;
            

            // find path to font
            if (fontFileLoc == "") return TextStatus::FAILED_TO_FIND_FONT;
            

            // load font as face
            FT_Face face;
            if (FT_New_Face(ft, fontFileLoc, 0, &face)) return TextStatus::FAILED_TO_LOAD_FONT;
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
                        return TextStatus::FAILED_TO_LOAD_GLYPH;
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
            TextVB.Create(nullptr, sizeof(float) * 6 * 4, GL_DYNAMIC_DRAW);
            gl::VertexAttribPointer(0, 4, 4 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            return TextStatus::OK;
        }
        void Draw(std::string text, glm::vec2 pos = { 0,0 }, float scale = 1.0f, glm::vec3 color = { 0,0,0 }, int activeTexture = 0) {
            // activate corresponding render state	
            shader.use();
            shader.setVec3("textColor", color);
            glActiveTexture(GL_TEXTURE0 + activeTexture);
            TextVB.Bind();

            // iterate through all characters
            //for (std::string::const_iterator c = text.begin(); c != text.end(); c++) 
            for(auto c : text){
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
                    { xpos + w, ypos,       1.0f, 1.0f }
                };
                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                // update content of VBO memory
                TextVB.Update(0, sizeof(vertices), vertices);

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
        gl::VertexBuffer TextVB;

        std::map<char, Character> Characters;
    };
}