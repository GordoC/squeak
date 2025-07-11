#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include <entt.hpp>

#include "render_system.hpp"


// Container for all our entities and game logic.
// Individual rendering / updates are deferred to the update() methods.
class WorldSystem
{
public:
	WorldSystem();

	// creates main window
	GLFWwindow* create_window();

	// starts and loads music and sound effects
	bool start_and_load_sounds();

	// call to close the window
	void close_window();

	// starts the game
	void init(RenderSystem* renderer);

	void initUI(RenderSystem* renderer);

	// releases all associated resources
	~WorldSystem();

	// steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	bool cutsceneStep();

	bool startScreenStep();

	void handle_player_block_collisions(entt::entity player_entity, entt::entity block_entity);
	
	void handle_player_wall_collisions(entt::entity entity1, entt::entity entity2);

	void handle_bullet_wall_collisions(entt::entity entity1, entt::entity entity2);

	void handle_bullet_door_collisions(entt::entity entity1, entt::entity entity2);

	void handle_bullet_exit_collisions(entt::entity entity1, entt::entity entity2);

	void handle_sniper_bullet_cat_collisions(entt::entity entity1, entt::entity entity2);
	void handle_portal_bullet_cat_collisions(entt::entity entity1, entt::entity entity2);

	void handle_player_door_collisions(entt::entity entity1, entt::entity entity2);

	// returns true if player has exited the level, false otherwise
	bool handle_player_exit_collisions(entt::entity entity1, entt::entity entity2);

	void handle_player_key_collisions(entt::entity entity1, entt::entity entity2);

	void handle_player_ice_collisions(entt::entity entity1, entt::entity entity2, bool* is_on_ice);

	void handle_player_portal_collisions(entt::entity player_entity, entt::entity wall_with_portal_entity);
	
	void handle_player_cheese_collisions(entt::entity entity1, entt::entity entity2);

	// darken screen
	void darken_screen();
  
	bool handle_player_harmful_collisions(entt::entity entity1, entt::entity entity2);

	void update_player_movement(float elapsed_ms_since_last_update);

	// check for collisions generated by the physics system
	void handle_collisions();

	// should the game be over ?
	bool is_over() const;

	entt::entity get_player() { return player_entity; }

	GAME_SCREEN_ID get_game_screen() { return game_screen; }

	int get_current_cutscene() { return current_cutscene; }

	int get_updated_cutscene() { return update_cutscene; }

	std::tuple<Direction, vec2> get_wall_collision_direction_and_position(vec2 wall_position, vec2 wall_scale, vec2 other_entity_position);


	bool has_other_portal(entt::entity portal_entity);
	
	vec2 get_other_portal_position(entt::entity portal_entity);

	inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
	inline std::string user_Path(const std::string& name) { return data_path() + "/user/" + std::string(name); };

	bool save_level(const std::string& filename);
	
	bool has_saved_level(const std::string& filename); 

	bool get_saved_level(const std::string& filename, unsigned int& level);

	static bool can_teleport;
    static entt::entity nearby_player_entity;
    static entt::entity nearby_wall_entity;

	bool clear_saved_level(const std::string& filename);

private:

	float mouse_pos_x = 0.0f;
	float mouse_pos_y = 0.0f;

	entt::entity player_entity;
	entt::entity weapon_indicator_entity;

	std::string world_level_filename = "saved_level.txt";

	// input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button_pressed(int button, int action, int mods);

	// player keyboard state tracker for movement
	std::set<int> key_state;

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	int next_invader_spawn;
	int invader_spawn_rate_ms;	// see default value in common.hpp

	int max_towers;	// see default value in common.hpp

	int max_portals; // see for default value in common.hpp

	int portal_charge; // see for default value in common.hpp

	bool has_opening_portal_placed;

	std::set<int> tiles_with_towers;

	unsigned int level_points; // points earned from current level
	unsigned int past_points;  // points earned from past levels

	// Game state
	RenderSystem* renderer;
	float current_speed;
	unsigned int level;
	void generate_level();

	// cutscene
	GAME_SCREEN_ID game_screen = GAME_SCREEN_ID::START_SCREEN;
	int update_cutscene = 0;
	int current_cutscene = 0;

	// grid
	std::vector<entt::entity> grid_lines;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* chicken_dead_sound;
	Mix_Chunk* chicken_eat_sound;
	Mix_Chunk* key_collect_sound;
	Mix_Chunk* portal_sound;

	// text ui
	TextRenderRequest *cheese_trr;
	TextRenderRequest *keys_trr;
	TextRenderRequest *portal_charge_trr;
	TextRenderRequest *level_text_trr;
	TextRenderRequest *fps_trr;

	entt::entity stats_ui;
	entt::entity tutorial_ui;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
