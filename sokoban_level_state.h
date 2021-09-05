#pragma once

#include <glm.hpp>

#include <array>
#include <cstdint>
#include <vector>

class SokobanLevelState
{
  public:
	glm::ivec2 player_position;
	std::vector<glm::ivec2> boxes_positions;
};
