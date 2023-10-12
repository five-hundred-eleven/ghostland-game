#include <glm/glm.hpp>

float abs(float x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

int signbit(float x) {
    if (x < 0) {
        return 1;
    }
    return 0;
}

int get_intersection(glm::vec3 vec_start, glm::vec3 vec_stop, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3) {

    glm::vec3 lba = vec_start - vec_stop;
    glm::vec3 v01 = P2 - P1;
    glm::vec3 v02 = P3 - P1;
    glm::vec3 cross12 = glm::cross(v01, v02);
    float det = glm::dot(lba, cross12);
    if (abs(det) < 0.0000001) {
        return 0;
    }

    glm::vec3 diff_start = vec_start - P1;
    float tn = glm::dot(cross12, diff_start);
    if (signbit(tn) != signbit(det) || abs(det) < abs(tn)) {
        return 0;
    }

    glm::vec3 coefU = glm::cross(v02, lba);
    float un = glm::dot(coefU, diff_start);
    if (signbit(un) != signbit(det) || abs(det) < abs(un)) {
        return 0;
    }

    glm::vec3 coefV = glm::cross(lba, v01);
    float vn = glm::dot(coefV, diff_start);
    if (signbit(vn) != signbit(det) || abs(det) < abs(vn)) {
        return 0;
    }

    return 1;

}
