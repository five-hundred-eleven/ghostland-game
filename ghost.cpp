#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

#include "ghost.h"

int counter = 0;

Ghost::Ghost(float xmin, float xmax, float zmin, float zmax) {

    ghost_id = counter;
    counter++;

    this->xmin = xmin;
    this->xmax = xmax;
    this->zmin = zmin;
    this->zmax = zmax;

    pos.x = rand_float(xmin, xmax);
    pos.y = 2.5;
    pos.z = rand_float(zmin, zmax);

    y_offset = rand_float(0.0, 3.14);
    yawr = rand_float(0.0, 6.28);

    first_frame = false;

}

void Ghost::apply_movement() {

    float curr_time = static_cast<float>(glfwGetTime());

    if (!first_frame) {
        first_frame = true;
        prev_move_time = curr_time;
        return;
    }

    float diff_time = curr_time - prev_move_time;
    prev_move_time = curr_time;

    glm::vec3 moved;
    moved.x = pos.x + (-sin(yawr) + cos(yawr)) * diff_time;
    moved.y = 2.5 + sin(curr_time + y_offset);
    moved.z = pos.z + (cos(yawr) + sin(yawr)) * diff_time;

    bool reset_yaw = false;

    if (moved.x < xmin) {
        reset_yaw = true;
        moved.x = xmin;
    } else if (moved.x > xmax) {
        reset_yaw = true;
        moved.x = xmax;
    }

    if (moved.z < zmin) {
        reset_yaw = true;
        moved.z = zmin;
    } else if (moved.z > zmax) {
        reset_yaw = true;
        moved.z = zmax;
    }

    if (reset_yaw) {
        yawr = rand_float(0.0, 6.28);
    } else {
        yawr += rand_float(-0.3, 0.3);
    }

    pos = moved;

}

glm::mat4 Ghost::get_model(glm::vec3 &camera_pos) {

    glm::mat4 ghost_model = glm::mat4(1.0f);
    ghost_model = glm::translate(ghost_model, pos);
    glm::vec3 ghost_direction = pos - camera_pos;
    ghost_model = glm::rotate(ghost_model, atan2f(ghost_direction.x, ghost_direction.z), glm::vec3(0.0f, 1.0f, 0.0f));
    return ghost_model;

}

float rand_float(float rmin, float rmax) {
    float rdiff = rmax - rmin;
    float res = (float)rand()/(float)RAND_MAX;
    return res * rdiff + rmin;
}
