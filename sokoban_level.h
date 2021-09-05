#pragma once

#include "sokoban_level_info.h"
#include "sokoban_level_state.h"

#include <algorithm>
#include <optional>
#include <stdexcept>

extern void play_sound(unsigned t_index);

extern bool teleport_flag;

class SokobanLevel
{
  public:
	SokobanLevel(const SokobanLevelInfo &level_info)
	{
		teleports_positions = level_info.teleports_positions;
		walls = level_info.walls;
		states.push_back(SokobanLevelState{level_info.player_position, level_info.boxes_positions});
		buttons_positions = level_info.buttons_positions;
		states.reserve(5000);
	}

	std::array<glm::ivec2, 2> teleports_positions;
	std::array<bool, sokoban_level_size.x * sokoban_level_size.y> walls;
	std::vector<SokobanLevelState> states;
	std::vector<glm::ivec2> buttons_positions;

	inline bool is_solid(glm::ivec2 t_position)
	{
		if (t_position.x < 0 || t_position.y < 0 || t_position.x >= sokoban_level_size.x ||
			t_position.y >= sokoban_level_size.y)
		{
			throw std::runtime_error("Invalid position");
		}
		return walls.at(t_position.y * sokoban_level_size.x + t_position.x);
	}

	inline bool contains_box(glm::ivec2 t_position)
	{
		for (const auto &box_position : states.back().boxes_positions)
		{
			if (box_position == t_position)
			{
				return true;
			}
		}
		return false;
	}

	inline std::optional<unsigned> get_box_index(glm::ivec2 t_position)
	{
		for (unsigned i = 0; i < states.back().boxes_positions.size(); ++i)
		{
			if (states.back().boxes_positions[i] == t_position)
			{
				return i;
			}
		}
		return {};
	}

	inline std::optional<glm::ivec2> move_box(unsigned t_box_index, glm::ivec2 t_direction)
	{
		bool teleport_used = false;
		t_direction.x = std::clamp(t_direction.x, -1, 1);
		t_direction.y = std::clamp(t_direction.y, -1, 1);
		if (t_direction.x == 0 && t_direction.y == 0)
		{
			throw std::runtime_error("Invalid direction parameters");
		}
		if (t_direction.x != 0 && t_direction.y != 0)
		{
			throw std::runtime_error("Invalid direction parameters");
		}
		auto new_box_position = states.back().boxes_positions[t_box_index] + t_direction;
		for (uint8_t i = 0; i < 2; ++i)
		{
			if (new_box_position == teleports_positions.at(i))
			{
				teleport_used = true;

				const auto &teleport_position = teleports_positions.at(i);
				const auto &other_teleport_position = teleports_positions.at(1 - i);

				new_box_position = other_teleport_position + t_direction;

				break;
			}
		}
		if (is_solid(new_box_position))
		{
			return {};
		}
		if (contains_box(new_box_position))
		{
			return {};
		}
		if (teleport_used)
		{
			teleport_flag = true;
			play_sound(4);
		}
		return new_box_position;
	}

	inline bool move(glm::ivec2 t_direction)
	{
		bool teleport_used = false;
		t_direction.x = std::clamp(t_direction.x, -1, 1);
		t_direction.y = std::clamp(t_direction.y, -1, 1);
		if (t_direction.x == 0 && t_direction.y == 0)
		{
			throw std::runtime_error("Invalid direction parameters");
		}
		if (t_direction.x != 0 && t_direction.y != 0)
		{
			throw std::runtime_error("Invalid direction parameters");
		}
		SokobanLevelState new_state = states.back();
		auto new_player_position = states.back().player_position + t_direction;
		for (uint8_t i = 0; i < 2; ++i)
		{
			if (new_player_position == teleports_positions.at(i))
			{
				teleport_used = true;

				const auto &teleport_position = teleports_positions.at(i);
				const auto &other_teleport_position = teleports_positions.at(1 - i);

				new_player_position = other_teleport_position + t_direction;

				break;
			}
		}
		if (is_solid(new_player_position))
		{
			play_sound(0);
			return false;
		}
		auto box_index = get_box_index(new_player_position);
		if (box_index)
		{
			auto new_box_position = move_box(box_index.value(), t_direction);
			if (!new_box_position)
			{
				play_sound(0);
				return false;
			}
			new_state.boxes_positions[box_index.value()] = new_box_position.value();
		}
		new_state.player_position = new_player_position;
		states.push_back(new_state);
		if (teleport_used)
		{
			play_sound(4);
			teleport_flag = true;
		}
		else
		{
			play_sound(1);
		}
		return true;
	}

	inline bool move_up()
	{
		return move({0, -1});
	}

	inline bool move_down()
	{
		return move({0, 1});
	}

	inline bool move_left()
	{
		return move({-1, 0});
	}

	inline bool move_right()
	{
		return move({1, 0});
	}

	inline bool undo() noexcept
	{
		if (states.size() > 1)
		{
			play_sound(3);
			states.pop_back();
			return true;
		}
		return false;
	}

	inline void reset()
	{
		play_sound(2);
		states.resize(1);
	}

	inline bool is_completed() const
	{
		for (const auto &box_position : states.back().boxes_positions)
		{
			bool found = false;
			for (const auto &button_position : buttons_positions)
			{
				if (box_position == button_position)
				{
					found = true;
					break;
				}
			}
			if (found == false)
			{
				return false;
			}
		}
		return true;
	}
};
