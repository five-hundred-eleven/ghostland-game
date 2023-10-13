#ifndef GHOST_H
#define GHOST_H

#include <glm/glm.hpp>

class Ghost {
    public:
        Ghost(float xmin, float xmax, float zmin, float zmax);
        void apply_movement();
        glm::mat4 get_model(glm::vec3 &camera_pos);

    private:
        int ghost_id;
        bool first_frame;
        glm::vec3 pos;
        float yawr;
        float xmin;
        float xmax;
        float zmin;
        float zmax;
        float y_offset;
        float prev_move_time;
};

float rand_float(float rmin, float rmax);

#endif
