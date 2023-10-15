#ifndef TEXT_H
#define TEXT_H

#include <stdio.h>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

int init_text(int *shader);
void render_text(int shader, std::string text, float x, float y, float scale, glm::vec3 color);

#endif
