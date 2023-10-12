#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <string>

#include "collisions.h"

const float lightoffsetmax = 15.0;
const float light_persist_factor = 0.99;
const float light_offset_factor = 0.01;
const float light_movement_multiplier = 128.0;
const float jumpvelocity = 8.104849;
const float vacceleration = -13.4058;
const float hvelocity = 10.0;

class Player {


    public:

        Player(glm::vec3 startpos, float startyaw);

        int num_walls;
        float *wall_vertices;

        glm::vec3 get_position();
        glm::vec3 get_camera_front();
        glm::vec3 get_camera_up();
        glm::vec3 get_camera_pos();
        glm::vec3 get_light_pos();
        glm::vec3 get_light_front();
        float get_yaw();
        float get_pitch();
        float get_fov();

        void mouse_callback(GLFWwindow *window, double xposI, double yposI);
        void process_input(GLFWwindow *window);
        void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        bool is_in_air();
        void apply_movement();

    private:
        float yaw;
        float pitch;
        float fov;
        float prevx;
        float prevy;
        bool first_mouse;
        bool first_frame;
        float light_xpersist;
        float light_ypersist;
        float armlength;
        float prev_move_time;
        float mouse_xoffset;
        float mouse_yoffset;

        glm::vec3 position;
        glm::vec3 camera_front;
        glm::vec3 camera_up;
        glm::vec3 light_pos;
        glm::vec3 light_front;
        glm::vec3 velocity;
        glm::vec3 camera_offset;

        void set_light_offset(float xoffset, float yoffset);

        glm::vec3 get_normal_from_index(int ix);
        glm::vec3 get_vec3_from_indices(int ix, int jx);
        glm::vec3 check_intersection(glm::vec3 movement);

};

#endif
