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
#include <vector>
#include <math.h>

#include "collisions.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
int createShader(const char *filename, int shadertype);

// settings
const unsigned int WINDOWWIDTH = 1600;
const unsigned int WINDOWHEIGHT = 900;

// camera
glm::vec3 player_pos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 light_pos  = glm::vec3(0.0f, 0.0f, 0.0f);

bool first_mouse = true;
float yaw   = 0.0f;
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

const char *objectcolorC = "objectcolor";
const char *viewposC = "viewPos";
const char *shininessC = "material.shininess";
const char *colorC = "material.color";
const char *lightposC = "light.position";
const char *directionC = "light.direction";
const char *largecutoffC = "light.largecutoff";
const char *smallcutoffC = "light.smallcutoff";
const char *ambientC = "light.ambient";
const char *diffuseC = "light.diffuse";
const char *specularC = "light.specular";


int num_walls;
float *wall_vertices;

int trailmax = 1800;

glm::vec3 getNormalFromIndex(int ix) {
    return glm::vec3(
        wall_vertices[ix * 6 * 6 + 3],
        wall_vertices[ix * 6 * 6 + 4],
        wall_vertices[ix * 6 * 6 + 5]
    );
}

glm::vec3 getVec3FromIndices(int ix, int jx) {
    return glm::vec3(
        wall_vertices[ix * 6 * 6 + jx * 6],
        wall_vertices[ix * 6 * 6 + jx * 6 + 1],
        wall_vertices[ix * 6 * 6 + jx * 6 + 2]
    );
}

glm::vec3 checkIntersection(glm::vec3 movement) {

    glm::vec3 vec_start, vec_stop, p1, p2, p3, res;
    int gotX = 0, gotZ = 0;
    vec_start = player_pos;
    vec_stop = vec_start + movement * 20.0f;
    res.x = movement.x;
    res.y = movement.y;
    res.z = movement.z;

    for (int ix = 0; ix < num_walls; ix++) {
        glm::vec3 norm = getNormalFromIndex(ix);
        if (gotX && gotZ) {
            break;
        }
        if (!gotX) {
            if ((movement.x > 0 && norm.x < 0) || (movement.x < 0 && norm.x > 0)) {
                p1 = getVec3FromIndices(ix, 1);
                p2 = getVec3FromIndices(ix, 0);
                p3 = getVec3FromIndices(ix, 2);
                if (getIntersection(vec_start, vec_stop, p1, p2, p3)) {
                    gotX = 1;
                    res.x = -movement.x;
                }
            }
        }
        if (!gotZ) {
            if ((movement.z > 0 && norm.z < 0) || (movement.z < 0 && norm.z > 0)) {
                p1 = getVec3FromIndices(ix, 1);
                p2 = getVec3FromIndices(ix, 0);
                p3 = getVec3FromIndices(ix, 2);
                if (getIntersection(vec_start, vec_stop, p1, p2, p3)) {
                    gotZ = 1;
                    res.z = -movement.z;
                }
            }
        }
    }

    return res;

}

int main()
{

    int success;

    FILE *fp = fopen("maze.txt", "r");
    float fakeyaw;
    if (fscanf(fp, "%f %f %f %f\n", &player_pos.x, &player_pos.y, &player_pos.z, &fakeyaw) == EOF) {
        printf("1st fscanf failed.\n");
        return -1;
    }
    if (fscanf(fp, "%d\n", &num_walls) == EOF) {
        printf("2nd fscanf failed.\n");
        return -1;
    }
    // num surfaces * 6 (vertices per point) * 6 (floats per point)
    int num_wall_vertices = num_walls * 6 * 6;
    wall_vertices = (float *)calloc(sizeof(float), num_wall_vertices);
    // read walls
    for (int i = 0; i < num_walls; i++) {
        for (int j = 0; j < 6; j++) {
            int vix = i*6*6 + j*6;
            if (fscanf(
                fp,
                "%f %f %f %f %f %f\n",
                &wall_vertices[vix],
                &wall_vertices[vix+1],
                &wall_vertices[vix+2],
                &wall_vertices[vix+3],
                &wall_vertices[vix+4],
                &wall_vertices[vix+5]
            ) == EOF) {
                printf("3rd fscanf failed.\n");
                return -1;
            }
        }
        if (fscanf(fp, "\n") == EOF) {
            printf("4th fscanf failed.\n");
            return -1;
        }
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOWWIDTH, WINDOWHEIGHT, "Ghostland!", NULL, NULL);
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

    int vertex_shader = createShader("vertexshader.txt", GL_VERTEX_SHADER);
    if (vertex_shader < 0) {
        return -1;
    }
    int fragment_shader = createShader("fragmentshader.txt", GL_FRAGMENT_SHADER);
    if (fragment_shader < 0) {
        return -1;
    }

    int wall_program = glCreateProgram();
    glAttachShader(wall_program, vertex_shader);
    glAttachShader(wall_program, fragment_shader);
    glLinkProgram(wall_program);
    glGetProgramiv(wall_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(wall_program, 1024, NULL, infoLog);
        printf("Shader wall_program link error: %s\n", infoLog);
        glfwTerminate();
        return -1;
    }

    int floor_program = glCreateProgram();
    glAttachShader(floor_program, vertex_shader);
    glAttachShader(floor_program, fragment_shader);
    glLinkProgram(floor_program);
    glGetProgramiv(floor_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(floor_program, 1024, NULL, infoLog);
        printf("Shader floor_program link error: %s\n", infoLog);
        glfwTerminate();
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // read floor
    float *floor_vertices = (float *)calloc(sizeof(float), 6*6);
    for (int j = 0; j < 6; j++) {
        int vix = j*6;
        if (fscanf(
            fp,
            "%f %f %f %f %f %f\n",
            &floor_vertices[vix],
            &floor_vertices[vix+1],
            &floor_vertices[vix+2],
            &floor_vertices[vix+3],
            &floor_vertices[vix+4],
            &floor_vertices[vix+5]
        ) == EOF) {
            printf("floor fscanf failed.\n");
            return -1;
        }
    }
    fclose(fp);

    // do stuff for walls
    unsigned int wallsVBO, wallsVAO;
    glGenVertexArrays(1, &wallsVAO);
    glGenBuffers(1, &wallsVBO);

    glBindVertexArray(wallsVAO);

    glBindBuffer(GL_ARRAY_BUFFER, wallsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_wall_vertices, wall_vertices, GL_STATIC_DRAW);

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

    // do stuff for trail
    float trail_vertices[] = {
        0.25f, 0.0f, 0.0f,
        -0.25f, 0.0f, 0.5f,
        -0.25f, 0.0f, -0.5f,

        -0.25f, 0.0f, 0.1f,
        -0.25f, 0.0f, -0.1f,
        -0.75f, 0.0f, 0.1f,

        -0.75f, 0.0f, 0.1f,
        -0.75f, 0.0f, -0.1f,
        -0.25f, 0.0f, -0.1f,
    };

    unsigned int trailVBO, trailVAO;
    glGenVertexArrays(1, &trailVAO);
    glGenBuffers(1, &trailVBO);

    glBindVertexArray(trailVAO);

    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 3 * 3, trail_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    int trail_shader = createShader("trailshader.txt", GL_VERTEX_SHADER);
    if (trail_shader < 0) {
        return -1;
    }
    int trailfrag_shader = createShader("trailfragshader.txt", GL_FRAGMENT_SHADER);
    if (trailfrag_shader < 0) {
        return -1;
    }

    int trail_program = glCreateProgram();
    glAttachShader(trail_program, trail_shader);
    glAttachShader(trail_program, trailfrag_shader);
    glLinkProgram(trail_program);
    glGetProgramiv(trail_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(wall_program, 1024, NULL, infoLog);
        printf("Shader trail_program link error: %s\n", infoLog);
        glfwTerminate();
        return -1;
    }

    glm::vec3 *trail_positions = (glm::vec3 *)calloc(sizeof(glm::vec3), trailmax);
    float *trail_angles = (float *)calloc(sizeof(float), trailmax);
    int trail_ix = 0;
    int trail_sz = 0;

    int time_sec = 0, num_frames = 0;
    glm::vec3 wall_color = glm::vec3(0.61f, 0.6f, 0.59f);
    float wall_shininess = 6.5;
    glm::vec3 floor_color = glm::vec3(0.11f, 0.1f, 0.09f);
    float floor_shininess = 3.25;
    glm::vec3 ambient = glm::vec3(0.007f, 0.006f, 0.005f);
    glm::vec3 diffuse = glm::vec3(0.61f, 0.60f, 0.59f);
    glm::vec3 specular = glm::vec3(0.11f, 0.10f, 0.09f);

    float largecutoff = glm::cos(glm::radians(30.0f));
    float smallcutoff = glm::cos(glm::radians(7.5f));

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        int current_frameI = (int)current_frame;
        if (current_frameI != time_sec) {
            printf("FPS: %d\n", num_frames);
            time_sec = current_frameI;
            num_frames = 0;
            trail_positions[trail_ix].x = player_pos.x;
            trail_positions[trail_ix].y = 6.5;
            trail_positions[trail_ix].z = player_pos.z;
            trail_angles[trail_ix] = yaw;
            printf("Recorded trail %d: (%f, %f, %f) %f\n", trail_ix, trail_positions[trail_ix].x, trail_positions[trail_ix].y, trail_positions[trail_ix].z, yaw);
            trail_ix = (trail_ix + 1) % trailmax;
            if (trail_sz < trailmax) {
                trail_sz++;
            }
        } else {
            num_frames++;
        }
        timed = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(wall_program);

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOWWIDTH / (float)WINDOWHEIGHT, 0.1f, 256.0f);
        glUniformMatrix4fv(glGetUniformLocation(wall_program, projectionC), 1, GL_FALSE, &projection[0][0]);

        glm::mat4 view = glm::lookAt(player_pos, player_pos + camera_front, camera_up);
        glUniformMatrix4fv(glGetUniformLocation(wall_program, viewC), 1, GL_FALSE, &view[0][0]);

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(wall_program, modelC), 1, GL_FALSE, &model[0][0]);

        // light position
        light_pos.x = player_pos.x + 0.5 * cos(glm::radians(yaw + 45.0f)) * cos(glm::radians(pitch));
        light_pos.y = player_pos.y + 0.5 * sin(glm::radians(pitch));
        light_pos.z = player_pos.z + 0.5 * sin(glm::radians(yaw + 45.0f)) * cos(glm::radians(pitch));

        // fragment requirements
        glUniform3fv(glGetUniformLocation(wall_program, viewposC), 1, &player_pos[0]);
        glUniform1f(glGetUniformLocation(wall_program, shininessC), wall_shininess);
        glUniform3fv(glGetUniformLocation(wall_program, colorC), 1, &wall_color[0]);
        glUniform3fv(glGetUniformLocation(wall_program, lightposC), 1, &light_pos[0]);
        glUniform3fv(glGetUniformLocation(wall_program, directionC), 1, &camera_front[0]);
        glUniform1f(glGetUniformLocation(wall_program, largecutoffC), largecutoff);
        glUniform1f(glGetUniformLocation(wall_program, smallcutoffC), smallcutoff);
        glUniform3fv(glGetUniformLocation(wall_program, ambientC), 1, &ambient[0]);
        glUniform3fv(glGetUniformLocation(wall_program, diffuseC), 1, &diffuse[0]);
        glUniform3fv(glGetUniformLocation(wall_program, specularC), 1, &specular[0]);

        glBindVertexArray(wallsVAO);
        glDrawArrays(GL_TRIANGLES, 0, num_wall_vertices);

        glUseProgram(floor_program);

        glUniformMatrix4fv(glGetUniformLocation(floor_program, projectionC), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floor_program, viewC), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floor_program, modelC), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(glGetUniformLocation(floor_program, viewposC), 1, &player_pos[0]);

        // fragment requirements
        glUniform3fv(glGetUniformLocation(floor_program, viewposC), 1, &player_pos[0]);
        glUniform1f(glGetUniformLocation(floor_program, shininessC), floor_shininess);
        glUniform3fv(glGetUniformLocation(floor_program, colorC), 1, &floor_color[0]);
        glUniform3fv(glGetUniformLocation(floor_program, lightposC), 1, &light_pos[0]);
        glUniform3fv(glGetUniformLocation(floor_program, directionC), 1, &camera_front[0]);
        glUniform1f(glGetUniformLocation(floor_program, largecutoffC), largecutoff);
        glUniform1f(glGetUniformLocation(floor_program, smallcutoffC), smallcutoff);
        glUniform3fv(glGetUniformLocation(floor_program, ambientC), 1, &ambient[0]);
        glUniform3fv(glGetUniformLocation(floor_program, diffuseC), 1, &diffuse[0]);
        glUniform3fv(glGetUniformLocation(floor_program, specularC), 1, &specular[0]);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6 * 6 * 1);

        glUseProgram(trail_program);

        glUniformMatrix4fv(glGetUniformLocation(trail_program, projectionC), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(trail_program, viewC), 1, GL_FALSE, &view[0][0]);
        glm::vec3 trail_color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(trail_program, objectcolorC), 1, &trail_color[0]);

        for (int i = 0; i < trail_sz; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, trail_positions[i]);
            model = glm::rotate(model, glm::radians(trail_angles[i]), glm::vec3(0.0f, -1.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(trail_program, modelC), 1, GL_FALSE, &model[0][0]);
            glBindVertexArray(trailVAO);

            glDrawArrays(GL_TRIANGLES, 0, 3 * 3 * 3);
        }

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
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        movement = glm::vec3(-camera_front.x, 0.0, -camera_front.z);
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm::vec3 right = glm::cross(camera_front, camera_up);
        movement = glm::vec3(-right.x, 0.0, -right.z);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm::vec3 right = glm::cross(camera_front, camera_up);
        movement = glm::vec3(right.x, 0.0, right.z);
    } else {
        return;
    }
    movement = glm::normalize(movement) * camera_speed_adjusted;
    movement = checkIntersection(movement);
    player_pos += movement;
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

int createShader(const char *filename, int shadertype) {
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
