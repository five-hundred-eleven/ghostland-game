#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

#include "ghost.h"

float pi = glm::pi<float>();
int counter = 0;
float direction_persist_factor = 0.02;

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

    y_offset = rand_float(0.0, pi);
    yawr = rand_float(0.0, pi*2.0f);

    first_frame = false;

    direction_persist = 0.0f;

}

void Ghost::apply_movement(float curr_time, float timed) {

    // vertical movement
    pos.y = 2.5 + sin(curr_time + y_offset);

    moved.x = -sin(yawr) + cos(yawr);
    moved.z = cos(yawr) + sin(yawr);

    pos += moved * timed;

    bool reset_yaw = false;

    if (pos.x < xmin) {
        reset_yaw = true;
        pos.x = xmin;
    } else if (pos.x > xmax) {
        reset_yaw = true;
        pos.x = xmax;
    }

    if (pos.z < zmin) {
        reset_yaw = true;
        pos.z = zmin;
    } else if (pos.z > zmax) {
        reset_yaw = true;
        pos.z = zmax;
    }

    // TODO make this face origin
    if (reset_yaw) {
        yawr = rand_float(0.0, pi * 2.0);
    } else {
        yawr += rand_float(-0.05, 0.05);
    }

}

glm::mat4 Ghost::get_model(glm::vec3 &camera_pos) {

    // initialize to identity matrix
    glm::mat4 ghost_model = glm::mat4(1.0f);
    // apply location
    ghost_model = glm::translate(ghost_model, pos);
    // apply direction
    // the cross product helps us determine which direction it's going relative to player
    glm::vec3 ghost_to_player = pos - camera_pos;
    glm::vec3 crossed = glm::cross(ghost_to_player, moved);
    //float dist2 = ghost_to_player.x * ghost_to_player.x + ghost_to_player.z * ghost_to_player.z;
    float theta = atan2f(ghost_to_player.x, ghost_to_player.z);
    if (crossed.y < 0.0) {
        direction_persist = (1.0 - direction_persist_factor) * direction_persist - direction_persist_factor;
    } else {
        direction_persist = (1.0 - direction_persist_factor) * direction_persist + direction_persist_factor;
    }
    if (direction_persist < 0.0) {
        theta += pi;
    }
    ghost_model = glm::rotate(ghost_model, theta, glm::vec3(0.0f, 1.0f, 0.0f));
    /*
    if (dist2 > 256) {
        yawr = theta;
    }
    if (ghost_id == 0) {
        printf("(%f, %f, %f)\n", pos.x, pos.y, pos.z);
        printf("%f %f %f %f\n", xmin, xmax, zmin, zmax);
    }
    */
    return ghost_model;

}

float rand_float(float rmin, float rmax) {
    float rdiff = rmax - rmin;
    float res = (float)rand()/(float)RAND_MAX;
    return res * rdiff + rmin;
}
