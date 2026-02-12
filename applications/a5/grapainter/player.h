#pragma once

#include <list>
#include <cppgl.h>

#include "../drawcollection.h"

void init_prototypes();

class Player {

public:
	glm::vec3 position=glm::vec3(0);
	glm::vec3 velocity=glm::vec3(0);

	float rotation=0;
	
	Player(glm::vec3 position);
	void start_moving_in_dir(int dir);
	void stop_moving_in_dir(int dir);
	void start_rotating_in_dir(int dir);
	void stop_rotating_in_dir(int dir);

	void update(float dt);
	void draw(cppgl::Shader &shader);

private:
	DrawCollection prototype;
	int is_moving_forward=false;
	int is_moving_right=false;
	int is_moving_backward=false;
	int is_moving_left=false;
	// always either zero or normalized
	glm::vec3 acceleration_input=glm::vec3(0);
	void recalculate_acceleration_input();
};

