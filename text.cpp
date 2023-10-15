#include <glm/glm.hpp>
#include <map>
#include <glad/glad.h>

#include "text.h"
#include "shader.h"

FT_Library ft;
FT_Face face;

const char *textcolorC = "textColor";

unsigned int textVAO, textVBO;

struct character_t {
    unsigned int texture_id;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

std::map<char, character_t> glyph_to_character;

int init_text(int *shader) {

    if (FT_Init_FreeType(&ft)) {
        printf("Could not init freetype library.\n");
        return -1;
    }

    if (FT_New_Face(ft, "fonts/LiberationSerif-Regular.ttf", 0, &face)) {
        printf("Could not font face.\n");
        return -1;
    }
    
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            printf("Failed to load glyph: %c\n", c);
            return -1;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        character_t character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x),
        };

        glyph_to_character.insert(std::pair<char, character_t>(c, character));

    }

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // configure textVAO/textVBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    *shader = create_shader_program("textshader.glsl", "textfragshader.glsl");
    if (*shader < 0) {
        printf("Got error compiling text shader!\n");
        return -1;
    }

    return 0;

}

void render_text(int shader, std::string text, float x, float y, float scale, glm::vec3 color) {

    set_uniform(shader, textcolorC, color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        character_t ch = glyph_to_character[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[24] = {
            xpos,       ypos + h,   0.0f, 0.0f,
            xpos,       ypos,       0.0f, 1.0f,
            xpos + w,   ypos,       1.0f, 1.0f,
            xpos,       ypos + h,   0.0f, 0.0f,
            xpos + w,   ypos,       1.0f, 1.0f,
            xpos + w,   ypos + h,   1.0f, 0.0f,
        };
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * (2 + 2) * 6, vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, (2 + 2) * 6);

        x += (ch.advance >> 6) * scale;

    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}
