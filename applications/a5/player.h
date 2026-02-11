#pragma once

#include <list>
#include <cppgl.h>

#include "rendering.h"
#include "drawcollection.h"

void init_prototypes();

class Player {
private:
	DrawCollection prototype;
	glm::vec3 acceleration_input=glm::vec3(0);
	int rotation_input=0;
public:
	glm::vec3 position=glm::vec3(0);
	glm::vec3 velocity=glm::vec3(0);

	float rotation=0;
	float rotation_vel=0;
	
	Player(glm::vec3 position);
	void start_moving_in_dir(int dir);
	void stop_moving_in_dir(int dir);
	void start_rotating_in_dir(int dir);
	void stop_rotating_in_dir(int dir);

	void update(float dt);
	void draw(cppgl::Shader &shader);
};

