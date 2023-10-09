#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"

#include <iostream>
#include <filesystem>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int WINDOWWIDTH = 1280;
const unsigned int WINDOWHEIGHT = 720;

// camera
glm::vec3 player_pos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f, 0.0f);

bool first_mouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  WINDOWWIDTH / 2.0;
float lastY =  WINDOWHEIGHT / 2.0;
float fov   =  60.0f;

float timed = 0.0f;
float last_frame = 0.0f;

float camera_speed = 10.0f;

const char *projectionC = "projection";
const char *viewC = "view";
const char *modelC = "model";

const char *viewposC = "viewPos";
const char *lightcolorC = "lightColor";
const char *objectcolorC = "objectColor";

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOWWIDTH, WINDOWHEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD.\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    FILE* fp = fopen("vertexshader.txt", "r");
    if (!fp) {
        printf("failed to open vertex shader!\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    unsigned int vertex_sz = ftell(fp);
    rewind(fp);

    char *vertex_buffer = (char *)calloc(sizeof(char), vertex_sz + 1);
    if (fread(vertex_buffer, vertex_sz, 1, fp) != 1) {
        printf("failed to read vertex shader!\n");
        return -1;
    }
    fclose(fp);

    int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_buffer, NULL);
    glCompileShader(vertex_shader);
    int success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(vertex_shader, 1024, NULL, infoLog);
        printf("Vertex shader compile error: %s\n", infoLog);
        glfwTerminate();
        return -1;
    }

    fp = fopen("fragmentshader.txt", "r");
    if (!fp) {
        printf("failed to open fragment shader!\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    unsigned int fragment_sz = ftell(fp);
    rewind(fp);

    char *fragment_buffer = (char *)calloc(sizeof(char), fragment_sz + 1);
    if (fread(fragment_buffer, fragment_sz, 1, fp) != 1) {
        printf("failed to read fragment shader!\n");
        return -1;
    }
    fclose(fp);

    int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_buffer, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(fragment_shader, 1024, NULL, infoLog);
        printf("Fragment shader compile error: %s\n", infoLog);
        glfwTerminate();
        return -1;
    }

    int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        printf("Shader program link error: %s\n", infoLog);
        glfwTerminate();
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    free(vertex_buffer);
    free(fragment_buffer);

    fp = fopen("maze.txt", "r");
    fscanf(fp, "%f %f %f %f\n", &player_pos.x, &player_pos.y, &player_pos.z, &yaw);
    int num_surfaces;
    fscanf(fp, "%d\n", &num_surfaces);
    // num surfaces * 6 (vertices per point) * 6 (floats per point)
    // add 1 for floor
    int num_walls = (num_surfaces) * 6 * 6;
    float *wall_vertices = (float *)calloc(sizeof(float), num_surfaces * 6 * 6);
    // read walls
    for (int i = 0; i < num_surfaces; i++) {
        for (int j = 0; j < 6; j++) {
            int vix = i*6*6 + j*6;
            fscanf(
                fp,
                "%f %f %f %f %f %f\n",
                &wall_vertices[vix],
                &wall_vertices[vix+1],
                &wall_vertices[vix+2],
                &wall_vertices[vix+3],
                &wall_vertices[vix+4],
                &wall_vertices[vix+5]
            );
        }
        fscanf(fp, "\n");
    }
    // read floor
    float *floor_vertices = (float *)calloc(sizeof(float), 6*6);
    for (int j = 0; j < 6; j++) {
        int vix = j*6;
        fscanf(
            fp,
            "%f %f %f %f %f %f\n",
            &floor_vertices[vix],
            &floor_vertices[vix+1],
            &floor_vertices[vix+2],
            &floor_vertices[vix+3],
            &floor_vertices[vix+4],
            &floor_vertices[vix+5]
        );
    }
    fclose(fp);

    // do stuff for walls
    unsigned int wallsVBO, wallsVAO;
    glGenVertexArrays(1, &wallsVAO);
    glGenBuffers(1, &wallsVBO);

    glBindVertexArray(wallsVAO);

    glBindBuffer(GL_ARRAY_BUFFER, wallsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_walls, wall_vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // do stuff for floor
    unsigned int floorVBO, floorVAO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 6 * 1, floor_vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int time_sec = 0, num_frames = 0;
    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        int current_frameI = (int)current_frame;
        if (current_frameI != time_sec) {
            printf("FPS: %d\n", num_frames);
            time_sec = current_frameI;
            num_frames = 0;
        } else {
            num_frames++;
        }
        timed = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOWWIDTH / (float)WINDOWHEIGHT, 0.1f, 256.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, projectionC), 1, GL_FALSE, &projection[0][0]);

        glm::mat4 view = glm::lookAt(player_pos, player_pos + camera_front, camera_up);
        glUniformMatrix4fv(glGetUniformLocation(program, viewC), 1, GL_FALSE, &view[0][0]);

        glm::vec3 wall_color = glm::vec3(0.5f, 0.5f, 0.5f);
        glUniform3fv(glGetUniformLocation(program, objectcolorC), 1, &wall_color[0]);

        glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(program, lightcolorC), 1, &light_color[0]);

        glUniform3fv(glGetUniformLocation(program, viewposC), 1, &player_pos[0]);

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, modelC), 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(wallsVAO);
        glDrawArrays(GL_TRIANGLES, 0, num_walls);

        glm::vec3 floor_color = glm::vec3(0.1f, 0.1f, 0.1f);
        glUniform3fv(glGetUniformLocation(program, objectcolorC), 1, &floor_color[0]);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6 * 6 * 1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &wallsVAO);
    glDeleteBuffers(1, &wallsVBO);

    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float camera_speed_adjusted = static_cast<float>(camera_speed * timed);
    glm::vec3 movement;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        movement = glm::vec3(camera_front.x, 0.0, camera_front.z);
        player_pos += glm::normalize(movement) * camera_speed_adjusted;
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        movement = glm::vec3(-camera_front.x, 0.0, -camera_front.z);
        player_pos += glm::normalize(movement) * camera_speed_adjusted;
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm::vec3 right = glm::cross(camera_front, camera_up);
        movement = glm::vec3(right.x, 0.0, right.z);
        player_pos -= glm::normalize(movement) * camera_speed_adjusted;
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm::vec3 right = glm::cross(camera_front, camera_up);
        movement = glm::vec3(right.x, 0.0, right.z);
        player_pos += glm::normalize(movement) * camera_speed_adjusted;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in)
{
    float xpos = static_cast<float>(xpos_in);
    float ypos = static_cast<float>(ypos_in);

    if (first_mouse)
    {
        lastX = xpos;
        lastY = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}
