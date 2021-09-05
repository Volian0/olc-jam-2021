#define OLC_PGE_APPLICATION
#include "glm.hpp"
#include "olcPixelGameEngine.h"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <soloud.h>
#include <soloud_wav.h>
#include <stdexcept>

#include "sokoban_level.h"
#include "sokoban_levels.h"

double total_elapsed_time = 0.0;
bool teleport_flag = false;
std::optional<double> last_teleport;

bool its_over = false;

class Engine : public olc::PixelGameEngine
{
  public:
	[[nodiscard]] bool OnUserUpdate(float t_delta_time) override
	{
		if (its_over)
		{
			Clear(olc::BLACK);
			DrawString({8, 8}, "GOOD GAME", olc::GREEN, 2);
			DrawString({8, 8 + 16}, "im too lazy to make more levels", olc::GREEN);
			DrawString({8, 8 + 16 + 8}, "i hope you enjoyed it!", olc::GREEN);
			return true;
		}
		total_elapsed_time += t_delta_time;
		if (teleport_flag)
		{
			last_teleport = total_elapsed_time;
			teleport_flag = false;
		}
		if (sokoban_level)
		{
			if (sokoban_level->is_completed())
			{
				// change levelino
				++level_n;
				music.stop();
				play_sound(4);
				if (level_n == 4)
				{
					its_over = true;
					return true;
				}
				sokoban_level.emplace(*all_levels.at(level_n));
				last_teleport = total_elapsed_time + 0.25;
				soloud.play(music);
				return true;
			}
			Clear(olc::BLACK);
			if (sokoban_allow_control)
			{
				if (GetKey(olc::Key::W).bPressed || GetKey(olc::Key::UP).bPressed)
				{
					sokoban_level->move_up();
				}
				else if (GetKey(olc::Key::S).bPressed || GetKey(olc::Key::DOWN).bPressed)
				{
					sokoban_level->move_down();
				}
				else if (GetKey(olc::Key::A).bPressed || GetKey(olc::Key::LEFT).bPressed)
				{
					sokoban_level->move_left();
				}
				else if (GetKey(olc::Key::D).bPressed || GetKey(olc::Key::RIGHT).bPressed)
				{
					sokoban_level->move_right();
				}
				else if (GetKey(olc::Key::Z).bPressed)
				{
					sokoban_level->undo();
				}
				else if (GetKey(olc::Key::R).bPressed)
				{
					sokoban_level->reset();
				}
			}
			// RENDER SOKOBAN LEVEL
			for (unsigned x = 0; x < sokoban_level_size.x; ++x)
				for (unsigned y = 0; y < sokoban_level_size.y; ++y)
				{
					glm::ivec2 offset{0, 0};
					if (last_teleport)
					{
						int diff = (0.5 - std::clamp((total_elapsed_time - last_teleport.value()), 0.0, 0.5)) * 8.0;
						if (diff > 0)
						{
							offset.x = rand() % diff;
							offset.y = rand() % diff;
						}
					}
					if (rand() % 2 == 0)
						offset.x = offset.x * -1;
					if (rand() % 2 == 0)
						offset.y = offset.y * -1;

					auto index = y * sokoban_level_size.x + x;
					DrawSprite(olc::vi2d(int(x * 16) + offset.x, int(y * 16) + offset.y),
							   sokoban_level->walls.at(index) ? &spr_wall : &spr_floor);
				}
			for (const auto &button_position : sokoban_level->buttons_positions)
			{
				DrawSprite(olc::vi2d(button_position.x * 16, button_position.y * 16), &spr_button);
			}
			for (const auto &box_position : sokoban_level->states.back().boxes_positions)
			{
				DrawSprite(olc::vi2d{box_position.x * 16, box_position.y * 16}, &spr_box);
			}
			this->SetPixelMode(olc::Pixel::MASK);
			for (uint8_t i = 0; i < 2; ++i)
			{
				DrawSprite(olc::vi2d(sokoban_level->teleports_positions[i].x * 16,
									 sokoban_level->teleports_positions[i].y * 16),
						   i == 0 ? &spr_portal_blue : &spr_portal_orange);
			}
			DrawSprite(olc::vi2d(sokoban_level->states.back().player_position.x * 16,
								 sokoban_level->states.back().player_position.y * 16),
					   &spr_robot);
			this->SetPixelMode(olc::Pixel::NORMAL);
			if (level_n == 0)
			{
				for (int i = 1;;)
				{
					this->DrawString({8 + i, 8 + i}, "Sokoban of", i == 0 ? olc::WHITE : olc::BLACK);
					this->DrawString({8 + i, 16 + i}, "the Future", i == 0 ? olc::WHITE : olc::BLACK);

					this->DrawString({8 + i, 32 + 8 + i}, "WASD=Move", i == 0 ? olc::WHITE : olc::BLACK);
					this->DrawString({8 + i, 32 + 16 + i}, "Z=Undo", i == 0 ? olc::WHITE : olc::BLACK);
					this->DrawString({8 + i, 32 + 24 + i}, "R=Reset", i == 0 ? olc::WHITE : olc::BLACK);
					this->DrawString({8 + i, 24 + i}, "by Volian0", i == 0 ? olc::WHITE : olc::BLACK);
					if (i == 1)
					{
						i = -1;
					}
					else if (i == -1)
					{
						i = 0;
					}
					else if (i == 0)
					{
						break;
					}
				}
			}
		}
		return true;
	}

	[[nodiscard]] bool OnUserCreate() override
	{
		sAppName = "Sokoban of the Future";

		soloud.init();

		music.load("assets/sound/music/world_of_2052.ogg");
		music.setLooping(true);

		music.setVolume(1.35);

		sfx_b0.load("assets/sound/sfx/b0.wav");
		sfx_b1.load("assets/sound/sfx/b1.wav");
		sfx_exp.load("assets/sound/sfx/exp.wav");
		sfx_hit.load("assets/sound/sfx/hit.wav");
		sfx_text.load("assets/sound/sfx/text.wav");

		sfx_b0.setVolume(0.7);
		sfx_b1.setVolume(0.6);
		sfx_exp.setVolume(0.85);
		sfx_hit.setVolume(0.8);
		sfx_text.setVolume(0.65);

		spr_box.LoadFromFile("assets/textures/box.png");
		spr_button.LoadFromFile("assets/textures/button.png");
		spr_floor.LoadFromFile("assets/textures/floor.png");
		spr_portal_blue.LoadFromFile("assets/textures/portal_blue.png");
		spr_portal_orange.LoadFromFile("assets/textures/portal_orange.png");
		spr_robot.LoadFromFile("assets/textures/robot.png");
		spr_wall.LoadFromFile("assets/textures/wall.png");

		soloud.play(music);

		sokoban_level = *all_levels[0];
		sokoban_allow_control = true;

		return true;
	}

	[[nodiscard]] bool OnUserDestroy() override
	{
		soloud.deinit();
		return true;
	}

	static inline SoLoud::Soloud soloud;
	SoLoud::Wav music;

	static inline SoLoud::Wav sfx_b0;
	static inline SoLoud::Wav sfx_b1;
	static inline SoLoud::Wav sfx_exp;
	static inline SoLoud::Wav sfx_hit;
	static inline SoLoud::Wav sfx_text;

	olc::Sprite spr_box;
	olc::Sprite spr_button;
	olc::Sprite spr_floor;
	olc::Sprite spr_portal_blue;
	olc::Sprite spr_portal_orange;
	olc::Sprite spr_robot;
	olc::Sprite spr_wall;

	std::optional<SokobanLevel> sokoban_level;
	bool sokoban_allow_control = false;
	unsigned level_n = 0;
};

void play_sound(unsigned t_index)
{
	switch (t_index)
	{
	case 0:
		Engine::soloud.play(Engine::sfx_b0);
		break;
	case 1:
		Engine::soloud.play(Engine::sfx_b1);
		break;
	case 2:
		Engine::soloud.play(Engine::sfx_exp);
		break;
	case 3:
		Engine::soloud.play(Engine::sfx_hit);
		break;
	case 4:
		Engine::soloud.play(Engine::sfx_text);
		break;
	}
}

auto main() -> int
try
{
	Engine engine;
	if (engine.Construct(320, 240, 1, 1) != olc::rcode::OK)
	{
		throw std::runtime_error("Couldn't construct olc::PixelGameEngine");
	}
	if (engine.Start() != olc::rcode::OK)
	{
		throw std::runtime_error("Couldn't start olc::PixelGameEngine");
	}
	return EXIT_SUCCESS;
}
catch (const std::exception &e)
{
	std::cout << "Error:\n" << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...)
{
	std::cout << "Unknown error" << std::endl;
	return EXIT_FAILURE;
}
