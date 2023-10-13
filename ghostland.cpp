#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <filesystem>
#include <vector>
#include <math.h>

#include "collisions.h"
#include "player.h"
#include "shader.h"
#include "ghost.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void set_light_front(int xoffset, int yoffset);

// player
Player *player;

// settings
const unsigned int WINDOWWIDTH = 1600;
const unsigned int WINDOWHEIGHT = 900;

// camera
bool mousemove = false;
float xpersist = 0.0f;
float ypersist = 0.0f;

bool first_mouse = true;
float yaw   = 0.0f;
float pitch =  0.0f;
float lastX =  WINDOWWIDTH / 2.0;
float lastY =  WINDOWHEIGHT / 2.0;

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

int main(int argc, char *argv[])
{

    int success;
    glm::vec3 position;

    FILE *fp = fopen("maze.txt", "r");
    float yaw;
    if (fscanf(fp, "%f %f %f %f\n", &position.x, &position.y, &position.z, &yaw) == EOF) {
        printf("1st fscanf failed.\n");
        return -1;
    }
    player = new Player(position, yaw);
    if (fscanf(fp, "%d\n", &num_walls) == EOF) {
        printf("2nd fscanf failed.\n");
        return -1;
    }
    player->num_walls = num_walls;
    // num surfaces * 6 (vertices per point) * 6 (floats per point)
    int num_wall_vertices = num_walls * 6 * 6;
    wall_vertices = (float *)calloc(sizeof(float), num_wall_vertices);
    int xmin_wall, xmax_wall, zmin_wall, zmax_wall;
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
            if (wall_vertices[vix] < xmin_wall) {
                xmin_wall = wall_vertices[vix];
            } else if (wall_vertices[vix] > xmax_wall) {
                xmax_wall = wall_vertices[vix];
            }
            if (wall_vertices[vix + 2] < zmin_wall) {
                zmin_wall = wall_vertices[vix + 2];
            } else if (wall_vertices[vix + 2] > zmax_wall) {
                zmax_wall = wall_vertices[vix + 2];
            }
        }
        if (fscanf(fp, "\n") == EOF) {
            printf("4th fscanf failed.\n");
            return -1;
        }
    }
    player->wall_vertices = wall_vertices;

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
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD.\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int wall_program = create_shader_program("vertexshader.glsl", "fragmentshader.glsl");
    if (wall_program < 0) {
        glfwTerminate();
        return -1;
    }

    // Perhaps there's a better way of doing this, rather than re-compiling the same files?
    int floor_program = create_shader_program("vertexshader.glsl", "fragmentshader.glsl");
    if (wall_program < 0) {
        glfwTerminate();
        return -1;
    }

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

    int trail_program = create_shader_program("trailshader.glsl", "trailfragshader.glsl");
    if (trail_program < 0) {
        glfwTerminate();
        return -1;
    }

    glm::vec3 *trail_positions = (glm::vec3 *)calloc(sizeof(glm::vec3), trailmax);
    float *trail_angles = (float *)calloc(sizeof(float), trailmax);
    int trail_ix = 0;
    int trail_sz = 0;

    // do stuff for ghosts
    // create program
    int ghost_program = create_shader_program("ghostshader.glsl", "ghostfragshader.glsl");
    if (ghost_program < 0) {
        glfwTerminate();
        return -1;
    }
    // ghost vertices
    float ghost_vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };
    unsigned int ghostVBO, ghostVAO;
    glGenVertexArrays(1, &ghostVAO);
    glBindVertexArray(ghostVAO);
    glGenBuffers(1, &ghostVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ghostVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ghost_vertices), ghost_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // ghost texture
    unsigned int ghost_texture;
    glGenTextures(1, &ghost_texture);
    glBindTexture(GL_TEXTURE_2D, ghost_texture);
    // set the texture wrapping parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, numberChannels;
    //stbi_set_flip_vertically_on_load(true);
    unsigned char *ghost_data = stbi_load("ghost_facing_right.png", &width, &height, &numberChannels, 0);
    printf("Num channels: %d\n", numberChannels);
    if (!ghost_data) {
        printf("Failed to load texture!\n");
        glfwTerminate();
        return -1;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ghost_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(ghost_data);

    // TODO move these into config file??
    int time_secI = 0, num_frames = 1;
    float time_sec;
    glm::vec3 wall_color = glm::vec3(0.61f, 0.6f, 0.59f);
    float wall_shininess = 6.5;
    glm::vec3 floor_color = glm::vec3(0.11f, 0.1f, 0.09f);
    float floor_shininess = 3.25;
    glm::vec3 ambient = glm::vec3(0.007f, 0.006f, 0.005f);
    glm::vec3 diffuse = glm::vec3(0.61f, 0.60f, 0.59f);
    glm::vec3 specular = glm::vec3(0.11f, 0.10f, 0.09f);

    float largecutoff = glm::cos(glm::radians(30.0f));
    float smallcutoff = glm::cos(glm::radians(7.5f));

    std::vector<Ghost *> ghosts;
    for (int i = 0; i < 4000; i++) {
        ghosts.push_back(new Ghost(xmin_wall, xmax_wall, zmin_wall, zmax_wall));
    }

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        int current_frameI = (int)current_frame;
        if (current_frameI != time_secI) {
            glm::vec3 player_pos = player->get_position();
            float yaw, pitch;
            yaw = player->get_yaw();
            pitch = player->get_pitch();
            printf("FPS: %d\n", num_frames);
            printf("Player is at: (%f, %f, %f) facing (%f, %f)\n", player_pos.x, player_pos.y, player_pos.z, yaw, pitch);
            time_sec = current_frame;
            time_secI = current_frameI;
            num_frames = 1;
            trail_positions[trail_ix].x = player_pos.x;
            trail_positions[trail_ix].y = 6.5;
            trail_positions[trail_ix].z = player_pos.z;
            trail_angles[trail_ix] = yaw;
            //printf("Recorded trail %d: (%f, %f, %f) %f\n", trail_ix, trail_positions[trail_ix].x, trail_positions[trail_ix].y, trail_positions[trail_ix].z, yaw);
            trail_ix = (trail_ix + 1) % trailmax;
            if (trail_sz < trailmax) {
                trail_sz++;
            }
        } else {
            num_frames++;
        }
        timed = current_frame - last_frame;
        last_frame = current_frame;

        mousemove = false;

        player->process_input(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(wall_program);

        float fov = player->get_fov();
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOWWIDTH / (float)WINDOWHEIGHT, 0.1f, 256.0f);
        set_uniform(wall_program, projectionC, projection);

        glm::vec3 camera_pos, camera_front, camera_up;
        camera_pos = player->get_camera_pos();
        camera_front = player->get_camera_front();
        camera_up = player->get_camera_up();

        glm::mat4 view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
        set_uniform(wall_program, viewC, view);

        glm::mat4 model = glm::mat4(1.0f);
        set_uniform(wall_program, modelC, model);

        // light position
        glm::vec3 light_pos = player->get_light_pos();
        //printf("light_pos: (%f, %f, %f)\n", light_pos.x, light_pos.y, light_pos.z);
        glm::vec3 light_front = player->get_light_front();
        //printf("light_pos: (%f, %f, %f)\n", light_front.x, light_front.y, light_front.z);

        // fragment requirements
        set_uniform(wall_program, viewposC, camera_pos);
        set_uniform(wall_program, shininessC, wall_shininess);
        set_uniform(wall_program, colorC, wall_color);
        set_uniform(wall_program, lightposC, light_pos);
        set_uniform(wall_program, directionC, light_front);
        set_uniform(wall_program, largecutoffC, largecutoff);
        set_uniform(wall_program, smallcutoffC, smallcutoff);
        set_uniform(wall_program, ambientC, ambient);
        set_uniform(wall_program, diffuseC, diffuse);
        set_uniform(wall_program, specularC, specular);

        glBindVertexArray(wallsVAO);
        glDrawArrays(GL_TRIANGLES, 0, num_wall_vertices);

        glUseProgram(floor_program);

        set_uniform(floor_program, projectionC, projection);
        set_uniform(floor_program, viewC, view);
        set_uniform(floor_program, modelC, model);
        set_uniform(floor_program, viewposC, camera_pos);

        // fragment requirements
        set_uniform(floor_program, shininessC, floor_shininess);
        set_uniform(floor_program, colorC, floor_color);
        set_uniform(floor_program, lightposC, light_pos);
        set_uniform(floor_program, directionC, light_front);
        set_uniform(floor_program, largecutoffC, largecutoff);
        set_uniform(floor_program, smallcutoffC, smallcutoff);
        set_uniform(floor_program, ambientC, ambient);
        set_uniform(floor_program, diffuseC, diffuse);
        set_uniform(floor_program, specularC, specular);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6 * 6 * 1);

        glUseProgram(trail_program);

        set_uniform(trail_program, projectionC, projection);
        set_uniform(trail_program, viewC, view);
        glm::vec3 trail_color = glm::vec3(1.0f, 0.0f, 0.0f);
        set_uniform(trail_program, objectcolorC, trail_color);

        for (int i = 0; i < trail_sz; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, trail_positions[i]);
            model = glm::rotate(model, glm::radians(trail_angles[i]), glm::vec3(0.0f, -1.0f, 0.0f));
            set_uniform(trail_program, modelC, model);
            glBindVertexArray(trailVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3 * 3 * 3);
        }

        glUseProgram(ghost_program);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ghost_texture);

        set_uniform(ghost_program, projectionC, projection);
        set_uniform(ghost_program, viewC, view);
        for (int i = 0; i < ghosts.size(); i++) {
            glm::mat4 ghost_model = ghosts[i]->get_model(camera_pos);
            ghosts[i]->apply_movement();
            set_uniform(ghost_program, modelC, ghost_model);
            glBindVertexArray(ghostVAO);
            glDrawArrays(GL_TRIANGLES, 0, (3 + 3 + 2) * 6);
        }

        player->apply_movement();

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    player->mouse_callback(window, xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    player->scroll_callback(window, xoffset, yoffset);
}
