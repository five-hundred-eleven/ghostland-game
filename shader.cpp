#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

int create_shader(const char *filename, int shadertype) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("failed to open vertex shader!\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    unsigned int buffer_sz = ftell(fp);
    rewind(fp);

    char *buffer = (char *)calloc(sizeof(char), buffer_sz + 1);
    if (fread(buffer, buffer_sz, 1, fp) != 1) {
        printf("failed to read vertex shader!\n");
        return -1;
    }
    fclose(fp);

    int shader = glCreateShader(shadertype);
    glShaderSource(shader, 1, &buffer, NULL);
    free(buffer);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, info_log);
        printf("Shader compile error for %s: %s\n", filename, info_log);
        glfwTerminate();
        return -1;
    }
    return shader;
}

int create_shader_program(const char *vertex_filename, const char *fragment_filename) {

    int vertex_shader = create_shader(vertex_filename, GL_VERTEX_SHADER);
    if (vertex_shader < 0) {
        return -1;
    }

    int fragment_shader = create_shader(fragment_filename, GL_FRAGMENT_SHADER);
    if (fragment_shader < 0) {
        return -1;
    }

    int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        printf("Shader wall_program link error: %s\n", infoLog);
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;

}

void set_uniform(int program, const char *key, float value) {
    glUniform1f(glGetUniformLocation(program, key), value);
}

void set_uniform(int program, const char *key, const glm::vec3 &value) {
    glUniform3fv(glGetUniformLocation(program, key), 1, &value[0]);
}
void set_uniform(int program, const char *key, const glm::mat4 &value) {
    glUniformMatrix4fv(glGetUniformLocation(program, key), 1, GL_FALSE, &value[0][0]);
}
