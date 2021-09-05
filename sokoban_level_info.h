#pragma once

#include <glm.hpp>

#include <array>
#include <vector>

static constexpr glm::ivec2 sokoban_level_size{20, 15};

class SokobanLevelInfo
{
  public:
	std::array<glm::ivec2, 2> teleports_positions;
	std::array<bool, sokoban_level_size.x * sokoban_level_size.y> walls;
	std::vector<glm::ivec2> buttons_positions;
	glm::ivec2 player_position;
	std::vector<glm::ivec2> boxes_positions;
};
