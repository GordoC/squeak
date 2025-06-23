
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

#include <entt.hpp>
#include "tinyECS/registry.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	  ai_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	PhysicsSystem physics_system;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// CK: be mindful of the order of your systems and rearrange this list only if necessary

		ScreenState& screen_state = registry.get<ScreenState>(renderer_system.get_screen_state_entity());

		GAME_SCREEN_ID game_screen = world_system.get_game_screen();

		switch (game_screen) {
			case GAME_SCREEN_ID::START_SCREEN:
				// start_screen_system.step(elapsed_ms);
				world_system.startScreenStep();
				break;
			case GAME_SCREEN_ID::CUTSCENE:
				// cutscene_system.step(elapsed_ms);;
				world_system.cutsceneStep();

				break;
			case GAME_SCREEN_ID::PLAYING:
				world_system.step(elapsed_ms);
				if (screen_state.darken_screen_factor < 0) {
					ai_system.step(&renderer_system, elapsed_ms);
					physics_system.step(elapsed_ms);
					world_system.handle_collisions();
				}
				break;
			default:
				break;
		}

		renderer_system.draw(game_screen);
	}

	return EXIT_SUCCESS;
}
