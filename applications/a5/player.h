#pragma once

#include <list>
#include <cppgl.h>

#include "rendering.h"
#include "drawcollection.h"

void init_prototypes();

class Player {
private:
	DrawCollection prototype;
public:
	glm::vec3 position;
	float move_duration;

	float rotation;
	
	Player(glm::vec3 position);

	void update();
	void draw(cppgl::Shader &shader);
};

