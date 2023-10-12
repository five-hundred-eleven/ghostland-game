#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>

int create_shader(const char *filename, int shadertype);
int create_shader_program(const char *vertex_filename, const char *fragment_filename);

void set_uniform(int program, const char *key, float value);
void set_uniform(int program, const char *key, const glm::vec3 &value);
void set_uniform(int program, const char *key, const glm::mat4 &value);

#endif
