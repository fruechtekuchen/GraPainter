#pragma once
#include <glm/glm.hpp>
#include <cppgl.h>
#include <memory>
#include <vector>
#include "player.h"


class Playercam {

public:
    Playercam(float _distance, glm::vec3 _direction, std::shared_ptr<Player> _player)
    : distance(_distance), direction(glm::normalize(_direction)), player(_player) {
        playercam = cppgl::Camera("playercam");
        playercam->pos = -distance*direction + player->position;
        playercam->dir = _direction;
        playercam->update();
        cppgl::make_camera_current(playercam);

        last_positions = std::vector<glm::vec3>(5, player->position);
        last_positions_index = 0;
    }

    void update(float dt) {
        last_positions[last_positions_index] = player->position;
        last_positions_index = (last_positions_index+1) % last_positions.size();
        glm::vec3 avg_pos(0);
        for(auto pos : last_positions) {
            avg_pos += pos;
        }
        avg_pos = 1.f/last_positions.size() * avg_pos;
        playercam->pos = -distance * direction + avg_pos;

    }

private:
    float distance;
    glm::vec3 direction;
    std::shared_ptr<Player> player;
    cppgl::Camera playercam;
    std::vector<glm::vec3> last_positions;
    int last_positions_index;
};