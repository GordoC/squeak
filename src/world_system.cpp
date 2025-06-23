// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "util/world_grid.hpp"
#include <tuple> 

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include <entt.hpp>

#include "physics_system.hpp"
#include "map_system.hpp"
#include "boids_system.hpp"

#include "tinyECS/registry.hpp"


bool WorldSystem::can_teleport = false;
entt::entity WorldSystem::nearby_player_entity = entt::null;
entt::entity WorldSystem::nearby_wall_entity = entt::null;

// map
MapSystem map_system;

// create the world
WorldSystem::WorldSystem() :
	next_invader_spawn(0),
	invader_spawn_rate_ms(INVADER_SPAWN_RATE_MS),
	max_towers(MAX_TOWERS_START),
	max_portals(MAX_PORTALS_START),
	portal_charge(0),
	has_opening_portal_placed(false),
	level_points(0),
	past_points(0),
	level(0),
	current_cutscene(0)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	if (key_collect_sound != nullptr)
		Mix_FreeChunk(key_collect_sound);
	if (portal_sound != nullptr)
		Mix_FreeChunk(portal_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Towers vs Invaders Assignment", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };
	
	
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);	

	return window;
}

bool WorldSystem::start_and_load_sounds() {
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());
	key_collect_sound = Mix_LoadWAV(audio_path("dingboop.wav").c_str());
	portal_sound = Mix_LoadWAV(audio_path("portal.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr || key_collect_sound == nullptr || portal_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str(),
			audio_path("dingboop.wav").c_str(),
			audio_path("portal.wav").c_str());
		return false;
	}

	return true;
}

void WorldSystem::generate_level() {
	std::cout << "LOADING LEVEL " << level << std::endl;

	// this implemetation is temporary for now; will be replaced by our map placement system
	int start_portals;
	if (level < 12) {	
		start_portals = map_system.loadLevel(level);
		max_portals = start_portals;
		portal_charge = start_portals;
		player_entity = map_system.createLevel(renderer);

		if (std::get<0>(map_system.levels[level]) == "boss_map.csv") {
			BoidsSystem::createBoidsFlock(renderer, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2), 100.0f, 20);
		}
		

	} else {
		max_portals = 2;
		portal_charge = 2;
		player_entity = WorldGrid::createPlayerAtGridPos(renderer, vec2(1,1));

		WorldGrid::createKeyAtGridPos(renderer, vec2(0,0));
		WorldGrid::createExitAtGridPos(renderer, vec2(13,9));
	}
}

void WorldSystem::init(RenderSystem* renderer_arg) {

	this->renderer = renderer_arg;

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;

	// Background Music
	Mix_PlayMusic(background_music, -1);

	// Init UI
	initUI(renderer_arg);

	// Set all states to default
    restart_game();
}

void WorldSystem::initUI(RenderSystem* renderer_arg) {
	// UI
	stats_ui = createUI(renderer_arg, TEXTURE_ASSET_ID::BACKGROUND_UI, -1);
	tutorial_ui = createUI(renderer_arg, TEXTURE_ASSET_ID::TUTORIAL_UI, 10);

	// text renderer
	renderer_arg->load_font();
	glm::vec2 text_scale(TEXT_WIDTH, TEXT_HEIGHT);

	entt::entity cheese_text = createText("0", glm::vec2(140, 70), text_scale);
	cheese_trr = &registry.get<TextRenderRequest>(cheese_text);

	entt::entity keys_text = createText("0", glm::vec2(420, 70), text_scale);
	keys_trr = &registry.get<TextRenderRequest>(keys_text);

	entt::entity portal_charge_text = createText("0", glm::vec2(680, 70), text_scale);
	portal_charge_trr = &registry.get<TextRenderRequest>(portal_charge_text);

	std::string level_text = map_system.getLevelText(0);
	entt::entity entity = createText(level_text, glm::vec2(50, 760), text_scale);
	level_text_trr = &registry.get<TextRenderRequest>(entity);

	entt::entity fps_text = createText("0", glm::vec2(1024, 60), text_scale);
	fps_trr = &registry.get<TextRenderRequest>(fps_text);
}

// Update Custcenes if updated scene
bool WorldSystem::cutsceneStep() {
	// magic number for cutscene count
	if (update_cutscene > 5) {
		game_screen = GAME_SCREEN_ID::PLAYING;
		auto cutscene_view = registry.view<Cutscene>();
		for (auto entity : cutscene_view) {
			registry.destroy(entity);
		}

		restart_game();
		return false;
	}
	if (update_cutscene != current_cutscene) {

		auto cutscene_view = registry.view<Cutscene>();
		for (auto entity : cutscene_view) {
			registry.destroy(entity);
		}
		entt::entity new_cutscene = createCutscene(current_cutscene);

		current_cutscene++;
		return true;
	}
	
}

// Init the Start Screen
bool WorldSystem::startScreenStep() {
	if (game_screen == GAME_SCREEN_ID::START_SCREEN) {
		auto start_screen_view = registry.view<StartScreen>();
		if (start_screen_view.empty()) {
			entt::entity start_screen = createStartScreen();
		}
		if (key_state.find(GLFW_KEY_S) != key_state.end()) {
			std::cout << "Starting game..." << std::endl;
			game_screen = GAME_SCREEN_ID::CUTSCENE;
			clear_saved_level(world_level_filename);
			for (auto entity : start_screen_view) {
				registry.destroy(entity);
			}
		}

		if (key_state.find(GLFW_KEY_C) != key_state.end() && has_saved_level(world_level_filename)) {
			std::cout << "Starting playing..." << std::endl;
			update_cutscene = 6;
			for (auto entity : start_screen_view) {
				registry.destroy(entity);
			}
			game_screen = GAME_SCREEN_ID::PLAYING;
			get_saved_level(world_level_filename, level);
			restart_game();
			std::cout << level << std::endl;
			return false;
		}
		return true;
	}
}

bool WorldSystem::save_level(const std::string& filename) {
    std::ofstream outFile(user_Path(filename));
    std::cout << filename << std::endl;
    if (!outFile.is_open()) {
        return false;
    }
    std::cout << "Saving level " << level << " to " << filename << std::endl;

    // Write the level with proper formatting
    outFile << level << std::endl; 

    outFile.close();
    return true;
}

bool WorldSystem::has_saved_level(const std::string& filename) {
    std::ifstream inFile(user_Path(filename));
    
    if (!inFile.is_open()) {
        std::cout << "Could not open file: " << filename << std::endl;
        return false;
    }
    
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();
    inFile.close();
    
	std::cout << fileSize << std::endl;
    return (fileSize > 0);
}

bool WorldSystem::get_saved_level(const std::string& filename, unsigned int& level) {
    std::ifstream inFile(user_Path(filename));
    
    // Check if file opened successfully
    if (!inFile.is_open()) {
        std::cout << "Could not open file: " << filename << std::endl;
        return false;
    }
    
    // Try to read the number
    if (inFile >> level) {
        inFile.close();
        std::cout << "Successfully read level " << level << " from " << filename << std::endl;
        return true;
    } else {
        inFile.close();
        std::cout << "Failed to read level from " << filename << std::endl;
        return false;
    }
}

bool WorldSystem::clear_saved_level(const std::string& filename) {
    // Open the file in truncate mode to clear its contents
    std::ofstream outFile(user_Path(filename), std::ios::trunc);
    
    if (!outFile.is_open()) {
        std::cout << "Could not open file for clearing: " << filename << std::endl;
        return false;
    }
    
    outFile.close();
    std::cout << "Successfully cleared file: " << filename << std::endl;
    return true;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << level_points + past_points; // total_points = level_points + past_points
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// update player movement
	update_player_movement(elapsed_ms_since_last_update);

	// Removing out of screen entities
	auto motion_view = registry.view<Motion>();

	for (auto entity : motion_view) {
	    Motion& motion = registry.get<Motion>(entity);
		if (motion.position.x + abs(motion.scale.x) < 0.f ||
			motion.position.x > WINDOW_WIDTH_PX || 
			motion.position.y + abs(motion.scale.y) < 0.f ||
			motion.position.y > WINDOW_HEIGHT_PX) {
			if(!registry.any_of<Player, WeaponIndicator>(entity)) // don't remove the player
				registry.destroy(entity);
		}
	}

	auto render_request_view = registry.view<RenderRequest>();
	for (entt::entity entity : render_request_view) {
		if (registry.all_of<Animation>(entity)) {
			Animation& animation = registry.get<Animation>(entity);
			if (registry.all_of<Explosion>(entity)) {
				if (animation.cur_ind > animation.end_ind) {
					// delete explosions
					registry.destroy(entity);
					continue;
				}
			}

			// do not animate any other entities except explosion if darken_screen_factor >= 0
			// darken_screen_factor >= 0 only if the player dies
			ScreenState& screenState = registry.get<ScreenState>(renderer->get_screen_state_entity());
			if (screenState.darken_screen_factor >= 0 && !registry.all_of<Explosion>(entity)) {
				continue;
			}

			float time = (float)(glfwGetTime() * 10.0f);
			if (time - animation.cur_frame_start_time > FRAME_DURATION) {
				if (animation.cur_ind < animation.start_ind || animation.cur_ind > animation.end_ind) {
					animation.cur_ind = animation.start_ind;
				}
				animation.cur_ind += 1;
				animation.cur_frame_start_time = (float)(glfwGetTime() * 10.0f);
				if (animation.cur_ind > animation.end_ind && animation.loops) {
					animation.cur_ind = animation.start_ind;
				}
				RenderRequest& render_request = registry.get<RenderRequest>(entity);
				render_request.used_texture = TEXTURE_ASSET_ID(animation.cur_ind);
			}
		}
	}

	// update weapon indicator
	if (registry.all_of<WeaponIndicator>(weapon_indicator_entity)) { // 
		vec2 player_pos = registry.get<Motion>(player_entity).position;
		vec2 mouse_dir = normalize(vec2(mouse_pos_x, mouse_pos_y) - player_pos);
		vec2 indicator_pos = player_pos + (mouse_dir * WEAPON_INDICATOR_SPACING);

		Motion &indicator_motion = registry.get<Motion>(weapon_indicator_entity);
		indicator_motion.position = indicator_pos;
		float indicator_angle = 90 + atan2(mouse_dir.y, mouse_dir.x) * 180 / M_PI;
		indicator_motion.angle = indicator_angle;
	}

	// update cheese and keys count
	if (!registry.view<Player>().empty()) {
		cheese_trr->text = std::to_string(level_points + past_points);
		keys_trr->text = std::to_string(registry.get<Player>(player_entity).keys);
		portal_charge_trr->text = std::to_string(portal_charge);
		fps_trr->text = std::to_string(int(1000 / elapsed_ms_since_last_update));
	}

	// update boids swarm
	BoidsSystem::updateBoids(elapsed_ms_since_last_update);

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Reset the game speed
	current_speed = 1.f;

	level_points = 0;
	max_towers = MAX_TOWERS_START;
	next_invader_spawn = 0;
	invader_spawn_rate_ms = INVADER_SPAWN_RATE_MS;
	has_opening_portal_placed = false;

	// cutscene
	update_cutscene++;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome

	auto motion_view = registry.view<Motion>();
	for (auto entity : motion_view) {
		if (!registry.all_of<UI>(entity)) {
			registry.destroy(entity);
		}
	}

	ScreenState& screenState = registry.get<ScreenState>(renderer->get_screen_state_entity());
	screenState.darken_screen_factor = -1;
	screenState.show_vignette = false;
	max_towers = 5;
	tiles_with_towers.clear();

	// clear keys
	key_state.clear();

	// generate level
	WorldSystem::generate_level();

	// create weapon indicator
	vec2 player_pos = registry.get<Motion>(player_entity).position;
	weapon_indicator_entity = createWeaponIndicator(player_pos, 0);	

	// reset count UI
	cheese_trr->text = "0";
	keys_trr->text = "0";
	level_text_trr->text = map_system.getLevelText(level);
	if (level >= map_system.getNumTutorialLevels()) {
		if (registry.valid(tutorial_ui)) {
			registry.destroy(tutorial_ui);
		}
	}

	if (level == 0 && game_screen == GAME_SCREEN_ID::PLAYING) {
		createSkipButton({WINDOW_WIDTH_PX - 100, WINDOW_HEIGHT_PX - 75}, {100, 50}); 
	} 
}

bool WorldSystem::handle_player_harmful_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_harmful_1 = registry.all_of<Harmful>(entity1);
	bool is_harmful_2 = registry.all_of<Harmful>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_harmful_1 && is_player_2) || (is_harmful_2 && is_player_1))) {
		entt::entity player_entity = is_player_1 ? entity1 : entity2;
		entt::entity harmful_entity = is_harmful_1 ? entity1 : entity2;

		Player& player_component = registry.get<Player>(player_entity);
		Harmful& harmful_component = registry.get<Harmful>(harmful_entity);
		player_component.health -= harmful_component.damage;
	
		if (player_component.health <= 0) {
			Mix_PlayChannel(-1, chicken_dead_sound, 0);
			// replace invader component with explosion component

			// set explosion velocity to 0
			Motion& motion = registry.get<Motion>(player_entity);
			vec2 explosion_position = motion.position;

			createExplosion(explosion_position);
			registry.destroy(player_entity);
			registry.destroy(weapon_indicator_entity);

			renderer->set_death_time(glfwGetTime() * 10.0f);
			darken_screen();

			return true;
		}
	}

	return false;
}

void WorldSystem::handle_player_block_collisions(entt::entity player_entity, entt::entity block_entity) {
	// get the wall's position and scale
	Motion &wall_motion = registry.get<Motion>(block_entity);
	vec2 &wall_scale = wall_motion.scale;
	vec2 &wall_position = wall_motion.position;

	// get the player's position, scale, and velocity
	Motion &player_motion = registry.get<Motion>(player_entity);
	vec2 &player_scale = player_motion.scale;
	vec2 &player_position = player_motion.position;

	// calculate overlap in X and Y
	float overlapX = std::min(player_position.x + player_scale.x/2 - (wall_position.x - wall_scale.x/2),
							wall_position.x + wall_scale.x/2 - (player_position.x - player_scale.x/2));
	float overlapY = std::min(player_position.y + player_scale.y/2 - (wall_position.y - wall_scale.y/2),
							wall_position.y + wall_scale.y/2 - (player_position.y - player_scale.y/2));

	// move the player in the direction of the smallest overlap
	if (overlapX < overlapY) {
		if (player_position.x < wall_position.x)
			player_position.x -= overlapX; // push left
		else
			player_position.x += overlapX; // push right
	} else {
		if (player_position.y < wall_position.y)
			player_position.y -= overlapY; // push up
		else
			player_position.y += overlapY; // push down
	}
}

void WorldSystem::handle_player_portal_collisions(entt::entity player_entity, entt::entity wall_with_portal_entity) {
	// get the wall's position and scale
	Motion &wall_motion = registry.get<Motion>(wall_with_portal_entity);
	vec2 &wall_scale = wall_motion.scale;
	vec2 &wall_position = wall_motion.position;

	// get the player's position, scale, and velocity
	Motion &player_motion = registry.get<Motion>(player_entity);
	vec2 &player_scale = player_motion.scale;
	vec2 &player_position = player_motion.position;
	vec2 &player_velocity = player_motion.velocity; 

	// get the other portal
	// TODO: @samzhao, abstract this logic to seperate func.
	auto portal_view = registry.view<Portal>();
	entt::entity current_portal_entity;
	entt::entity other_portal_entity;
	vec2 other_portal_position = {0, 0};
	int other_portal_direction = 0;

	for (entt::entity portal_entity : portal_view) {
		Motion &portal_motion = registry.get<Motion>(portal_entity);
		vec2 &portal_position = portal_motion.position;
		Portal &portal = registry.get<Portal>(portal_entity);

		if (portal_position == wall_position) {
			current_portal_entity = portal_entity;
		}
	}
	Portal &portal = registry.get<Portal>(current_portal_entity);

	if (registry.valid(portal.other_portal) && registry.all_of<Portal>(portal.other_portal)) {
		Portal &other_portal_entity = registry.get<Portal>(portal.other_portal);
		other_portal_position = other_portal_entity.position;
		other_portal_direction = other_portal_entity.direction;
	} else {
		std::cout << "ERROR: Other portal entity is invalid or does not have a Portal component!" << std::endl;
	}
	
	// calculate the new player position and spawn the player on the side the other portal faces
	vec2 new_player_position = other_portal_position;
	if (other_portal_direction == Direction::TOP) {
		new_player_position.y = other_portal_position.y - GRID_CELL_HEIGHT_PX;
	} else if (other_portal_direction == Direction::RIGHT) {
		new_player_position.x = other_portal_position.x + GRID_CELL_WIDTH_PX;
	} else if (other_portal_direction == Direction::BOTTOM) {
		new_player_position.y = other_portal_position.y + GRID_CELL_HEIGHT_PX;
	} else if (other_portal_direction == Direction::LEFT) {
		new_player_position.x = other_portal_position.x - GRID_CELL_WIDTH_PX;
	}

	player_motion.position = new_player_position;
}

void WorldSystem::handle_player_wall_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_wall_1 = registry.all_of<Wall>(entity1);
	bool is_wall_2 = registry.all_of<Wall>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_wall_1 && is_player_2) || (is_wall_2 && is_player_1))) {
		entt::entity wall_entity = is_wall_1 ? entity1 : entity2;
		Wall &wall = registry.get<Wall>(wall_entity);
		entt::entity player_entity = is_player_1 ? entity1 : entity2;
		entt::entity current_portal_entity;
		Motion &wall_motion = registry.get<Motion>(wall_entity);
		vec2 &wall_scale = wall_motion.scale;
		vec2 &wall_position = wall_motion.position;

		// If the wall has a portal and there exists another portal corresponding to it, handle the collision differently
		bool has_portal = wall.has_portal;
		auto portal_view = registry.view<Portal>();

		// if (has_portal) {
		// 	for (entt::entity portal_entity : portal_view) {
		// 		Motion &portal_motion = registry.get<Motion>(portal_entity);
		// 		vec2 &portal_position = portal_motion.position;

		// 		if (portal_position == wall_position) {
		// 			current_portal_entity = portal_entity;
		// 		}
		// 	}

		// 	Portal &portal = registry.get<Portal>(current_portal_entity);
		// 	// Determine which direction the player came from as we need to handle direction in this fn.
		// 	if (registry.valid(portal.other_portal) && registry.all_of<Portal>(portal.other_portal)) {
		// 		// get the wall's position and scale

		// 		// get the player's position, scale, and velocity
		// 		Motion &player_motion = registry.get<Motion>(player_entity);
		// 		vec2 &player_position = player_motion.position;

		// 		entt::entity current_portal_entity;

		// 		int portal_direction = portal.direction;
		// 		std::tuple<Direction,vec2> direction_position_tuple = get_wall_collision_direction_and_position(wall_position, wall_scale, player_position);
		// 		Direction direction = std::get<0>(direction_position_tuple);

		// 		if (direction == portal_direction) {
		// 			if (key_state.find(GLFW_KEY_SPACE) != key_state.end()) {
		// 				handle_player_portal_collisions(player_entity, wall_entity);
		// 			} else {
		// 				handle_player_block_collisions(player_entity, wall_entity);
		// 			}
		// 		} else {
		// 			handle_player_block_collisions(player_entity, wall_entity);
		// 		}
		// 	} else {
		// 		handle_player_block_collisions(player_entity, wall_entity);
		// 	}
		// } else {
		handle_player_block_collisions(player_entity, wall_entity);
		// }	
	}
}

void WorldSystem::handle_bullet_wall_collisions(entt::entity entity1, entt::entity entity2) {
    bool is_wall_1 = registry.all_of<Wall>(entity1);
    bool is_wall_2 = registry.all_of<Wall>(entity2);
	bool is_portal_bullet_1 = registry.all_of<Projectile>(entity1);
	bool is_portal_bullet_2 = registry.all_of<Projectile>(entity2);
	bool is_sniper_bullet_1 = registry.all_of<SniperBullet>(entity1);
	bool is_sniper_bullet_2 = registry.all_of<SniperBullet>(entity2);
    bool is_projectile_1 = is_portal_bullet_1 || is_sniper_bullet_1;
	bool is_projectile_2 = is_portal_bullet_2 || is_sniper_bullet_2;

    if ((is_wall_1 && is_projectile_2) || (is_wall_2 && is_projectile_1)) {
        entt::entity wall_entity = is_wall_1 ? entity1 : entity2;
        entt::entity projectile_entity = is_projectile_1 ? entity1 : entity2;

		Wall &wall = registry.get<Wall>(wall_entity);
		bool has_portal = wall.has_portal;

        // Get the wall's position and scale
        Motion &wall_motion = registry.get<Motion>(wall_entity);
        vec2 &wall_scale = wall_motion.scale;
        vec2 &wall_position = wall_motion.position;

        // Get the projectile's position, scale, and velocity
        Motion &projectile_motion = registry.get<Motion>(projectile_entity);
        vec2 &projectile_scale = projectile_motion.scale;
        vec2 &projectile_position = projectile_motion.position;

		// Check if the wall has a portal
		auto portal_view = registry.view<Portal>();
		if (has_portal) {
			entt::entity portal_entity;
			vec2 other_portal_position = {0.f, 0.f};
			int direction;

			// Get the portal entity
			for (entt::entity portal : portal_view) {
				Motion &portal_motion = registry.get<Motion>(portal);
				vec2 &portal_position = portal_motion.position;

				if (portal_position == wall_position) {
					portal_entity = portal;
				}
			}
			// Check if there is another portal
			Portal &portal = registry.get<Portal>(portal_entity);
			std::cout << "Checking for other portal" << std::endl;
			
			if (registry.valid(portal.other_portal) && registry.all_of<Portal>(portal.other_portal) && (is_sniper_bullet_1 || is_sniper_bullet_2)) {
				Portal &other_portal_entity = registry.get<Portal>(portal.other_portal);
				other_portal_position = other_portal_entity.position;
				direction = other_portal_entity.direction;
			} else {
				registry.destroy(projectile_entity);
				return;
				std::cout << "ERROR: Other portal entity is invalid or does not have a Portal component!" << std::endl;
			}

			
			vec2 new_projectile_velocity = {0.f, 0.f};
			vec2 new_projectile_position = other_portal_position;

			float bullet_speed = PORTAL_BULLET_SPEED;
			if (registry.all_of<SniperBullet>(projectile_entity)) {
				bullet_speed = SNIPER_BULLET_SPEED;
			}
			// Set the new projectiles velocity based on direction the portal is facing
			if (direction == Direction::TOP) {
				new_projectile_velocity = {0.f, -bullet_speed};  // Moving up
				new_projectile_position.y = other_portal_position.y - GRID_CELL_HEIGHT_PX;
			} else if (direction == Direction::RIGHT) {
				new_projectile_velocity = {bullet_speed, 0.f};  // Moving right
				new_projectile_position.x = other_portal_position.x + GRID_CELL_WIDTH_PX;
			} else if (direction == Direction::BOTTOM) {
				new_projectile_velocity = {0.f, bullet_speed}; // Moving down
				new_projectile_position.y = other_portal_position.y + GRID_CELL_HEIGHT_PX;
			} else if (direction == Direction::LEFT) {
				new_projectile_velocity = {-bullet_speed, 0.f}; // Moving left
				new_projectile_position.x = other_portal_position.x - GRID_CELL_WIDTH_PX;
			}

			// Only want to create sniper bullet through portals
			if (registry.all_of<SniperBullet>(projectile_entity)) {
				Harmful &harmful = registry.get<Harmful>(projectile_entity);
				createSniperBullet(renderer, new_projectile_position, projectile_scale, new_projectile_velocity, harmful.damage);
			}
			registry.destroy(projectile_entity);
			return;
		}

		std::tuple<Direction, vec2> direction_position_tuple = get_wall_collision_direction_and_position(wall_position, wall_scale, projectile_position);
		Direction direction = std::get<0>(direction_position_tuple);
		vec2 wall_collision_position = std::get<1>(direction_position_tuple);


		if (portal_charge > 0) {
			if (wall.has_portal) {
				registry.destroy(projectile_entity);
				return;
			}
			if (registry.all_of<Projectile>(projectile_entity)) {
				if (!has_opening_portal_placed) {
					createPortal(renderer, wall_position, wall_scale, direction);
					has_opening_portal_placed = true;
				} else {
					// Get the most recent portal placed and connect it to the current portal
					if (portal_view.size() > 0) {
						std::cout << "Found previous portal" << std::endl;
						entt::entity previous_portal_entity = *(portal_view.begin());
						for (entt::entity portal : portal_view) {
							Portal &portal_pos = registry.get<Portal>(portal);
						}
						createPortal(renderer, wall_position, wall_scale, direction, previous_portal_entity);
						has_opening_portal_placed = false;
					} else {
						std::cout << "Error: No previous portal found" << std::endl;
					}
				}
				portal_charge -= 1;
				wall.has_portal = true;
			}
		}

        // Destroy the projectile on impact of the wall
        registry.destroy(projectile_entity);
    }
}

void WorldSystem::handle_bullet_door_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_door_1 = registry.all_of<Door>(entity1);
	bool is_door_2 = registry.all_of<Door>(entity2);
	bool is_portal_bullet_1 = registry.all_of<Projectile>(entity1);
	bool is_portal_bullet_2 = registry.all_of<Projectile>(entity2);
	bool is_sniper_bullet_1 = registry.all_of<SniperBullet>(entity1);
	bool is_sniper_bullet_2 = registry.all_of<SniperBullet>(entity2);
    bool is_projectile_1 = is_portal_bullet_1 || is_sniper_bullet_1;
	bool is_projectile_2 = is_portal_bullet_2 || is_sniper_bullet_2;

	if ((is_door_1 && is_projectile_2) || (is_door_2 && is_projectile_1)) {
		entt::entity door_entity = is_door_1 ? entity1 : entity2;
		entt::entity projectile_entity = is_projectile_1 ? entity1 : entity2;

		Door &door = registry.get<Door>(door_entity);
		if (door.locked) {
			registry.destroy(projectile_entity);
		}
	}
}

void WorldSystem::handle_bullet_exit_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_exit_1 = registry.all_of<Exit>(entity1);
	bool is_exit_2 = registry.all_of<Exit>(entity2);
	bool is_portal_bullet_1 = registry.all_of<Projectile>(entity1);
	bool is_portal_bullet_2 = registry.all_of<Projectile>(entity2);
	bool is_sniper_bullet_1 = registry.all_of<SniperBullet>(entity1);
	bool is_sniper_bullet_2 = registry.all_of<SniperBullet>(entity2);
    bool is_projectile_1 = is_portal_bullet_1 || is_sniper_bullet_1;
	bool is_projectile_2 = is_portal_bullet_2 || is_sniper_bullet_2;

	if ((is_exit_1 && is_projectile_2) || (is_exit_2 && is_projectile_1)) {
		entt::entity projectile_entity = is_projectile_1 ? entity1 : entity2;

		registry.destroy(projectile_entity);
	}
}

void WorldSystem::handle_sniper_bullet_cat_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_cat_1 = registry.all_of<Cat>(entity1);
	bool is_cat_2 = registry.all_of<Cat>(entity2);
	bool is_bullet_1 = registry.all_of<SniperBullet>(entity1);
	bool is_bullet_2 = registry.all_of<SniperBullet>(entity2);

	if ((is_cat_1 && is_bullet_2) || (is_cat_2 && is_bullet_1)) {
		entt::entity cat_entity = is_cat_1 ? entity1 : entity2;
		entt::entity bullet_entity = is_bullet_1 ? entity1 : entity2;

		createExplosion(registry.get<Motion>(cat_entity).position);
		registry.destroy(bullet_entity);
		registry.destroy(cat_entity);
	}
}

void WorldSystem::handle_portal_bullet_cat_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_cat_1 = registry.all_of<Cat>(entity1);
	bool is_cat_2 = registry.all_of<Cat>(entity2);
	bool is_bullet_1 = registry.all_of<Projectile>(entity1);
	bool is_bullet_2 = registry.all_of<Projectile>(entity2);

	if ((is_cat_1 && is_bullet_2) || (is_cat_2 && is_bullet_1)) {
		entt::entity cat_entity = is_cat_1 ? entity1 : entity2;
		entt::entity bullet_entity = is_bullet_1 ? entity1 : entity2;

		registry.destroy(bullet_entity);
	}
}

void WorldSystem::handle_player_door_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_door_1 = registry.all_of<Door>(entity1);
	bool is_door_2 = registry.all_of<Door>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_door_1 && is_player_2) || (is_door_2 && is_player_1))) {
		entt::entity door_entity = is_door_1 ? entity1 : entity2;
		entt::entity player_entity = is_player_1 ? entity1 : entity2;

		Door &door = registry.get<Door>(door_entity);
		if (door.locked) {
			Player &player = registry.get<Player>(player_entity);
			if (player.keys > 0) {
				RenderRequest &request = registry.get<RenderRequest>(door_entity);
				request.used_texture = TEXTURE_ASSET_ID::OPEN_DOOR;
				player.keys -= 1;
				door.locked = false;
			} else {
				handle_player_block_collisions(player_entity, door_entity);
			}
		}
	}
}

bool WorldSystem::handle_player_exit_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_exit_1 = registry.all_of<Exit>(entity1);
	bool is_exit_2 = registry.all_of<Exit>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_exit_1 && is_player_2) || (is_exit_2 && is_player_1))) {
		level += 1;
		past_points += level_points;
		level_points = 0;
		restart_game();
		return true;
	}

	return false;
}

void WorldSystem::handle_player_key_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_key_1 = registry.all_of<Key>(entity1);
	bool is_key_2 = registry.all_of<Key>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_key_1 && is_player_2) || (is_key_2 && is_player_1))) {
		entt::entity key_entity = is_key_1 ? entity1 : entity2;
		entt::entity player_entity = is_player_1 ? entity1 : entity2;

		Player &player = registry.get<Player>(player_entity);
		player.keys += 1;
		Mix_PlayChannel(-1, key_collect_sound, 0);

		registry.destroy(key_entity);
	}
}

void WorldSystem::handle_player_ice_collisions(entt::entity entity1, entt::entity entity2, bool* is_on_ice) {
	bool is_ice_1 = registry.all_of<FloorIce>(entity1);
	bool is_ice_2 = registry.all_of<FloorIce>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_ice_1 && is_player_2) || (is_ice_2 && is_player_1))) {
		entt::entity ice_entity = is_ice_1 ? entity1 : entity2;
		entt::entity player_entity = is_player_1 ? entity1 : entity2;

		*is_on_ice = true;
	}
}

void WorldSystem::handle_player_cheese_collisions(entt::entity entity1, entt::entity entity2) {
	bool is_cheese_1 = registry.all_of<Cheese>(entity1);
	bool is_cheese_2 = registry.all_of<Cheese>(entity2);
	bool is_player_1 = registry.all_of<Player>(entity1);
	bool is_player_2 = registry.all_of<Player>(entity2);

	if (((is_cheese_1 && is_player_2) || (is_cheese_2 && is_player_1))) {
		entt::entity cheese_entity = is_cheese_1 ? entity1 : entity2;
		entt::entity player_entity = is_player_1 ? entity1 : entity2;

		Cheese &cheese = registry.get<Cheese>(cheese_entity);
		level_points += cheese.points;
		Mix_PlayChannel(-1, chicken_eat_sound, 0);

		registry.destroy(cheese_entity);
	}
}

void WorldSystem::darken_screen() {
    ScreenState& screenState = registry.get<ScreenState>(renderer->get_screen_state_entity());
    screenState.darken_screen_factor = 0; // set to 0 so that screen will start to darken
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	auto collision_view = registry.view<Collision>();
	auto proximity_view = registry.view<PortalProximity>();

	bool is_on_ice = false;

	bool player_died = false;
	bool player_exited = false;

	for (auto collision : collision_view) {
		entt::entity& entity1 = registry.get<Collision>(collision).entity1;
        entt::entity& entity2 = registry.get<Collision>(collision).entity2;
		
		handle_player_wall_collisions(entity1, entity2);
		handle_bullet_wall_collisions(entity1, entity2);
		handle_bullet_door_collisions(entity1, entity2);
		handle_bullet_exit_collisions(entity1, entity2);
		handle_player_ice_collisions(entity1, entity2, &is_on_ice);
		handle_player_key_collisions(entity1, entity2);
		handle_player_door_collisions(entity1, entity2);
		handle_player_cheese_collisions(entity1, entity2);
		handle_sniper_bullet_cat_collisions(entity1, entity2);
		handle_portal_bullet_cat_collisions(entity1, entity2);

		// if player exits the level and call restart_game, or 
		// if player dies and calls registry.destroy(entity),
		// we should break from the loop since looping through the deleted entities would crash the game
		player_died = handle_player_harmful_collisions(entity1, entity2);
		if (player_died) {
			break;
		}
		player_exited = handle_player_exit_collisions(entity1, entity2);
		if (player_exited) {
			break;
		}
	}

	for (auto proximity : proximity_view) {
		entt::entity& entity1 = registry.get<PortalProximity>(proximity).entity1;
		entt::entity& entity2 = registry.get<PortalProximity>(proximity).entity2;

		handle_player_portal_collisions(entity1, entity2);
	}

	if (!player_died && !player_exited) {
		Player &player = registry.get<Player>(player_entity);
		player.is_on_ice = is_on_ice;
	} 
	
	// Remove all collisions from this simulation step
	registry.clear<Collision>();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		close_window();
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}
		// SHIFT - record whether the SHIFT key is depressed or not to support macOS "right-click" --> SHIFT + left-click
	if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
		key_state.insert(key);
		if (action == GLFW_RELEASE) {
			key_state.erase(key);
		}
	}

	// C to continue where the player last left off
	if (key == GLFW_KEY_C) {
		key_state.insert(key);
		if (action == GLFW_RELEASE) {
			key_state.erase(key);
		}
	}
	
	if (key == GLFW_KEY_SPACE) {
		key_state.insert(key);
		if (action == GLFW_RELEASE) {
			key_state.erase(key);
		}
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE && can_teleport) {
		handle_player_portal_collisions(nearby_player_entity, nearby_wall_entity);
	}

	// K to save the level if you're playing
	if (key == GLFW_KEY_K && game_screen == GAME_SCREEN_ID::PLAYING) {
		if (action == GLFW_RELEASE) {
			if (save_level(world_level_filename)) {
				std::cout << "INFO: level saved to: " << world_level_filename << std::endl;
			}
			else {
				std::cout << "ERROR: failed to save level: " << world_level_filename << std::endl;
			}
		}
	}

	const float speed = 200.f; // Adjust movement speed
    if (registry.view<Player>().size() > 0) {
        // entt::entity player = registry.players.entities[0];
        // Motion& motion = registry.get<Motion>(player);
		Animation& animation = registry.get<Animation>(player_entity);

		if (action == GLFW_PRESS) {
        	key_state.insert(key);
    	} else if (action == GLFW_RELEASE) {
        	key_state.erase(key);
			animation.start_ind = (int)TEXTURE_ASSET_ID::MOUSE_4_EAST;
			animation.end_ind = (int)TEXTURE_ASSET_ID::MOUSE_6_EAST;
    	}
    }
	// Debugging - not used in A1, but left intact for the debug lines
	// if (key == GLFW_KEY_D) {
	// 	if (action == GLFW_RELEASE) {
	// 		if (debugging.in_debug_mode) {
	// 			debugging.in_debug_mode = false;
	// 		}
	// 		else {
	// 			debugging.in_debug_mode = true;
	// 		}
	// 	}
	// }
}

void WorldSystem::update_player_movement(float elapsed_ms) {
    if (registry.view<Player>().size() > 0) {
		Player& player = registry.get<Player>(player_entity);
        Motion& motion = registry.get<Motion>(player_entity);
		Animation& animation = registry.get<Animation>(player_entity);

		float step_seconds = elapsed_ms / 1000.f;
		float acceleration = (player.is_on_ice ? ICE_ACCEL : PLAYER_ACCEL) * elapsed_ms;

		// no horizontal movement
		if ((key_state.find(GLFW_KEY_A) != key_state.end() && key_state.find(GLFW_KEY_D) != key_state.end())
		|| key_state.find(GLFW_KEY_A) == key_state.end() && key_state.find(GLFW_KEY_D) == key_state.end()) {
			bool going_right = motion.velocity.x > 0;
			motion.velocity.x = (going_right ? 1 : -1) * max(abs(motion.velocity.x) - acceleration, 0.f);
		}
		// moving left
		if (key_state.find(GLFW_KEY_A) != key_state.end()) {
            motion.velocity.x = max(motion.velocity.x - acceleration, -PLAYER_MAX_SPEED);
			animation.start_ind = (int)TEXTURE_ASSET_ID::MOUSE_1_WEST;
			animation.end_ind = (int)TEXTURE_ASSET_ID::MOUSE_3_WEST;
        }
		// moving right
		else if (key_state.find(GLFW_KEY_D) != key_state.end()) {
			motion.velocity.x = min(motion.velocity.x + acceleration, PLAYER_MAX_SPEED);
			animation.start_ind = (int)TEXTURE_ASSET_ID::MOUSE_1_EAST;
			animation.end_ind = (int)TEXTURE_ASSET_ID::MOUSE_3_EAST;
        }

		// no vertical movement
		if ((key_state.find(GLFW_KEY_W) != key_state.end() && key_state.find(GLFW_KEY_S) != key_state.end()
		|| key_state.find(GLFW_KEY_W) == key_state.end() && key_state.find(GLFW_KEY_S) == key_state.end())) {
			bool going_up = motion.velocity.y > 0;
			motion.velocity.y = (going_up ? 1 : -1) * max(abs(motion.velocity.y) - acceleration, 0.f);
		}

		// moving up
        if (key_state.find(GLFW_KEY_W) != key_state.end()) {
            motion.velocity.y = max(motion.velocity.y - acceleration, -PLAYER_MAX_SPEED);
			animation.start_ind = (int)TEXTURE_ASSET_ID::MOUSE_1_NORTH;
			animation.end_ind = (int)TEXTURE_ASSET_ID::MOUSE_3_NORTH;
        }
		// moving down
        else if (key_state.find(GLFW_KEY_S) != key_state.end()) {
            motion.velocity.y = min(motion.velocity.y + acceleration, PLAYER_MAX_SPEED);
			animation.start_ind = (int)TEXTURE_ASSET_ID::MOUSE_1_SOUTH;
			animation.end_ind = (int)TEXTURE_ASSET_ID::MOUSE_3_SOUTH;
        }
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Handle mouse clicking for invader and tower placement. [DONE]
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	ScreenState& screen_state = registry.get<ScreenState>(renderer->get_screen_state_entity());

	if (game_screen == GAME_SCREEN_ID::CUTSCENE) {
		if (action == GLFW_PRESS) {
			std::cout << "next screen" << std::endl;
			update_cutscene++;
		}
		return;
	}

	if (game_screen == GAME_SCREEN_ID::PLAYING && level == 0) {
		if (action == GLFW_PRESS) {
			vec2 mouse = {mouse_pos_x, mouse_pos_y};
			auto skip_buttons = registry.view<SkipButton, Motion>();

			for (auto entity : skip_buttons) {
				Motion& motion = registry.get<Motion>(entity);
				vec2 pos = motion.position;
				vec2 size = motion.scale;

				// Check if the mouse is within the bounds of the button
				if (mouse.x >= pos.x - size.x / 2 && mouse.x <= pos.x + size.x / 2 &&
					mouse.y >= pos.y - size.y / 2 && mouse.y <= pos.y + size.y / 2) {

					std::cout << "skip button pressed" << std::endl;

					game_screen = GAME_SCREEN_ID::PLAYING;
					level = 6;

					for (auto b : skip_buttons) {
						registry.destroy(b);
					}
					restart_game();
					return;
				}
			}
		}
		return;
	}

	// on button press
	if (action == GLFW_PRESS && screen_state.darken_screen_factor == -1) {

		int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
		int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
		std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;

		if (button == GLFW_MOUSE_BUTTON_LEFT && key_state.find(GLFW_KEY_LEFT_SHIFT) == key_state.end()) {
			vec2 player_pos = registry.get<Motion>(player_entity).position;
			vec2 velocity = normalize(vec2(mouse_pos_x, mouse_pos_y) - player_pos) * PORTAL_BULLET_SPEED;

			if (portal_charge != 0) { 
				createPortalBullet(renderer, player_pos, PORTAL_PROJECTILE_SIZE, velocity);
				Mix_PlayChannel(-1, portal_sound, 0);
			}
		} 
	}

	// Remove portals
	if (button == GLFW_MOUSE_BUTTON_RIGHT || 
		(button == GLFW_MOUSE_BUTTON_LEFT && key_state.find(GLFW_KEY_LEFT_SHIFT) != key_state.end()) || 
		(button == GLFW_MOUSE_BUTTON_LEFT && key_state.find(GLFW_KEY_RIGHT_SHIFT) != key_state.end())) {
		auto portal_view = registry.view<Portal>();
		for (entt::entity portal_entity : portal_view) {
			registry.destroy(portal_entity);
		}
		has_opening_portal_placed = false;

		// Find the walls with portals and remove the portal
		auto wall_view = registry.view<Wall>();
		for (entt::entity wall_entity : wall_view) {
			Wall &wall = registry.get<Wall>(wall_entity);
			if (wall.has_portal) {
				wall.has_portal = false;
			}
		}
	}
}

std::tuple<Direction, vec2> WorldSystem::get_wall_collision_direction_and_position(vec2 wall_position, vec2 wall_scale, vec2 other_entity_position) {
    // Calculate wall boundaries
    float half_width = wall_scale.x / 2.0f;
    float half_height = wall_scale.y / 2.0f;

    // Compute the differences in each direction
    float dx = other_entity_position.x - wall_position.x;
    float dy = other_entity_position.y - wall_position.y;

    // Compute absolute distances to each edge
    int absDx = abs(dx) - half_width;
    int absDy = abs(dy) - half_height;

    // Determine collision direction, in the case where it hits the exact corner of the wall, determine surrounding walls
	// If the range on the corner is extremely small, then we can assume that the entity is hitting the corner
	if (abs(absDx - absDy) < 6) {
		bool has_left_wall = false;
		bool has_right_wall = false;
		bool has_top_wall = false;
		bool has_bottom_wall = false;

		vec2 neighbouring_wall_position_left = wall_position + vec2(-GRID_CELL_WIDTH_PX, 0);
		vec2 neighbouring_wall_position_right = wall_position + vec2(GRID_CELL_WIDTH_PX, 0);
		vec2 neighbouring_wall_position_top = wall_position + vec2(0, -GRID_CELL_HEIGHT_PX);
		vec2 neighbouring_wall_position_bottom = wall_position + vec2(0, GRID_CELL_HEIGHT_PX);

		auto wall_view = registry.view<Wall>();

		for (entt::entity entity : wall_view) {
			Motion &motion = registry.get<Motion>(entity);
			vec2 entity_position = motion.position;

			if (entity_position == neighbouring_wall_position_left) {
				has_left_wall = true;
			} else if (entity_position == neighbouring_wall_position_right) {
				has_right_wall = true;
			} else if (entity_position == neighbouring_wall_position_top) {
				has_top_wall = true;
			} else if (entity_position == neighbouring_wall_position_bottom) {
				has_bottom_wall = true;
			}
		}

		// Case where it hits a corner of a wall with all surrounding walls
		if (has_left_wall && has_right_wall && has_top_wall && has_bottom_wall) {
			float left_distance = length(neighbouring_wall_position_left - other_entity_position);
			float right_distance = length(neighbouring_wall_position_right - other_entity_position);
			float top_distance = length(neighbouring_wall_position_top - other_entity_position);
			float bottom_distance = length(neighbouring_wall_position_bottom - other_entity_position);

			float min_distance = std::min(std::min(left_distance, right_distance), std::min(top_distance, bottom_distance));

			if (min_distance == left_distance) {
				return get_wall_collision_direction_and_position(neighbouring_wall_position_left, wall_scale, other_entity_position);
			} else if (min_distance == right_distance) {
				return get_wall_collision_direction_and_position(neighbouring_wall_position_right, wall_scale, other_entity_position);
			} else if (min_distance == top_distance) {
				return get_wall_collision_direction_and_position(neighbouring_wall_position_top, wall_scale, other_entity_position);
			} else if (min_distance == bottom_distance) {
				return get_wall_collision_direction_and_position(neighbouring_wall_position_bottom, wall_scale, other_entity_position);
			}
		}

		// Case where it hits a corner of a wall with 3 surrounding walls
		if (has_top_wall && has_bottom_wall) {
			if (has_left_wall) {
				return {Direction::RIGHT, wall_position};
			}

			if (has_right_wall) {
				return {Direction::LEFT, wall_position};
			}

			return (dx > 0) ? std::make_tuple(Direction::RIGHT, wall_position) 
							: std::make_tuple(Direction::LEFT, wall_position);
		}

		if (has_right_wall && has_left_wall) {
			if (has_top_wall) {
				return {Direction::BOTTOM, wall_position};
			}

			if (has_bottom_wall) {
				return {Direction::TOP, wall_position};
			}

			return (dy > 0) ? std::make_tuple(Direction::BOTTOM, wall_position) 
							: std::make_tuple(Direction::TOP, wall_position);
		}
	}

	// General case: Determine direction based on greater displacement
	if (absDx > absDy) {
		return (dx > 0) ? std::make_tuple(Direction::RIGHT, wall_position)
						: std::make_tuple(Direction::LEFT, wall_position);
	} else {
		return (dy > 0) ? std::make_tuple(Direction::BOTTOM, wall_position)
						: std::make_tuple(Direction::TOP, wall_position);
	}
}

vec2 get_other_portal_position(entt::entity current_portal_entity) {
	Portal &portal = registry.get<Portal>(current_portal_entity);
	Portal &other_portal = registry.get<Portal>(portal.other_portal);

	return other_portal.position;
}