#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "player.h"

Player::Player(glm::vec3 startpos, float startyaw) {

    position = startpos;
    yaw = startyaw;

    pitch = 0.0f;
    first_mouse = false;
    first_frame = false;
    light_xpersist = 0.0f;
    light_ypersist = 0.0f;
    armlength = 0.5f;
    camera_offset = glm::vec3(0.0f, 2.5f, 0.0f);
    camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    fov = 60.0f;

    velocity = glm::vec3(0.0f, 0.0f, 0.0f);

}

glm::vec3 Player::get_pos() const {
    return position;
}

glm::vec3 Player::get_camera_front() const {
    return camera_front;
}

glm::vec3 Player::get_camera_up() const {
    return camera_up;
}

glm::vec3 Player::get_camera_pos() const {
    return position + camera_offset;
}

glm::vec3 Player::get_light_pos() const {
    return get_camera_pos() + get_light_front() * armlength;
}

glm::vec3 Player::get_light_front() const {
    float yawp = yaw + light_xpersist;
    float pitchp = pitch + light_ypersist;
    glm::vec3 front;
    front.x = cos(glm::radians(yawp)) * cos(glm::radians(pitchp));
    front.y = sin(glm::radians(pitchp));
    front.z = sin(glm::radians(yawp)) * cos(glm::radians(pitchp));
    return normalize(front);
}

float Player::get_yaw() {
    return yaw;
}

float Player::get_pitch() {
    return pitch;
}

float Player::get_fov() {
    return fov;
}

void Player::mouse_callback(GLFWwindow* window, double xposI, double yposI) {

    float xpos = static_cast<float>(xposI);
    float ypos = static_cast<float>(yposI);

    if (first_mouse) {
        prevx = xpos;
        prevy = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - prevx;
    float yoffset = prevy - ypos;
    prevx = xpos;
    prevy = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    float prev_yaw = yaw;
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    mouse_xoffset = xoffset;
    mouse_yoffset = yoffset;

}

void Player::process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    bool W, A, S, D, space;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        W = true;
    } else {
        W = false;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        S = true;
    } else {
        S = false;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        A = true;
    } else {
        A = false;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        D = true;
    } else {
        D = false;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        space = true;
    } else {
        space = false;
    }

    if (!is_in_air()) {

        glm::vec3 relvel;

        if (space) {
            velocity.y = jumpvelocity;
            return;
        } else {
            velocity.y = 0.0;
        }

        // intentionally don't update velocity if contradictory keys are pressed
        if (!W && !S) {
            relvel.z = 0.0;
        } else if (W) {
            relvel.z = 1.0;
        } else if (S) {
            relvel.z = -1.0;
        }

        if (!A && !D) {
            relvel.x = 0.0;
        } else if (A) {
            relvel.x = -1.0;
        } else if (D) {
            relvel.x = 1.0;
        }

        if (relvel.x == 0.0 && relvel.z == 0.0) {
            velocity.x = 0.0;
            velocity.z = 0.0;
            return;
        }

        relvel = glm::normalize(relvel);
        relvel *= hvelocity;

        float yawr = glm::radians(yaw);
        velocity.z = relvel.x * cos(yawr) + relvel.z * sin(yawr);
        velocity.x = -relvel.x * sin(yawr) + relvel.z * cos(yawr);

    }

}

bool Player::is_in_air() {
    return position.y > 0.0;
}

void Player::set_light_offset(float xoffset, float yoffset) {

    xoffset *= light_movement_multiplier;
    yoffset *= light_movement_multiplier;

    light_xpersist = light_xpersist * light_persist_factor + xoffset * light_offset_factor;
    if (light_xpersist > lightoffsetmax) {
        light_xpersist = lightoffsetmax;
    } else if (light_xpersist < -lightoffsetmax) {
        light_xpersist = -lightoffsetmax;
    }
    light_ypersist = light_ypersist * light_persist_factor + yoffset * light_offset_factor;
    if (light_ypersist > lightoffsetmax) {
        light_ypersist = lightoffsetmax;
    } else if (light_ypersist < -lightoffsetmax) {
        light_ypersist = -lightoffsetmax;
    }

}

void Player::apply_movement(float timed) {

    // mouse movement
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = normalize(front);

    // light movement;
    set_light_offset(mouse_xoffset, mouse_yoffset);
    mouse_xoffset = 0.0;
    mouse_yoffset = 0.0;

    // positional movement
    glm::vec3 movement = velocity;
    movement *= timed;
    glm::vec3 intersected = check_intersection(movement);
    if (is_in_air()) {
        if (intersected.x == 0.0) {
            velocity.x = 0.0;
        }
        if (intersected.z == 0.0) {
            velocity.z = 0.0;
        }
    } else {  // not in air
        if (intersected.x != movement.x) {
            intersected.x = -movement.x;
        }
        if (intersected.z != movement.z) {
            intersected.z = -movement.z;
        }
    }

    position.y += intersected.y;
    if (position.y <= 0.0f) {
        position.y = 0.0f;
        velocity.y = 0.0f;
    } else {
        velocity.y += timed * vacceleration;
    }

    position.x += intersected.x;
    position.z += intersected.z;

}

void Player::reset_position(glm::vec3 startpos, float startyaw) {
    position = startpos;
    yaw = startyaw;
    velocity = glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 Player::get_normal_from_index(int ix) {
    return glm::vec3(
        wall_vertices[ix * 6 * 6 + 3],
        wall_vertices[ix * 6 * 6 + 4],
        wall_vertices[ix * 6 * 6 + 5]
    );
}

glm::vec3 Player::get_vec3_from_indices(int ix, int jx) {
    return glm::vec3(
        wall_vertices[ix * 6 * 6 + jx * 6],
        wall_vertices[ix * 6 * 6 + jx * 6 + 1],
        wall_vertices[ix * 6 * 6 + jx * 6 + 2]
    );
}

glm::vec3 Player::check_intersection(glm::vec3 movement) {

    glm::vec3 vec_start, vec_stop, p1, p2, p3, res;
    bool gotX = false, gotZ = false;
    vec_start = get_camera_pos();
    vec_stop = vec_start + movement * 20.0f;
    res.x = movement.x;
    res.y = movement.y;
    res.z = movement.z;

    for (int ix = 0; ix < num_walls; ix++) {
        if (gotX && gotZ) {
            break;
        }
        glm::vec3 norm = get_normal_from_index(ix);
        if (!gotX) {
            if ((movement.x > 0 && norm.x < 0) || (movement.x < 0 && norm.x > 0)) {
                p1 = get_vec3_from_indices(ix, 1);
                p2 = get_vec3_from_indices(ix, 0);
                p3 = get_vec3_from_indices(ix, 2);
                if (get_intersection(vec_start, vec_stop, p1, p2, p3)) {
                    gotX = true;
                    res.x = 0.0;
                }
            }
        }
        if (!gotZ) {
            if ((movement.z > 0 && norm.z < 0) || (movement.z < 0 && norm.z > 0)) {
                p1 = get_vec3_from_indices(ix, 1);
                p2 = get_vec3_from_indices(ix, 0);
                p3 = get_vec3_from_indices(ix, 2);
                if (get_intersection(vec_start, vec_stop, p1, p2, p3)) {
                    gotZ = true;
                    res.z = 0.0;
                }
            }
        }
    }

    return res;

}

void Player::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    if (fov > 90.0f) {
        fov = 90.0f;
    }
}

