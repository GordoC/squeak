#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <entt.hpp>

enum INVADER_COLOR {
	BLUE = 0,
	GREEN = 1,
	RED = 2,
	GREY = 3,
	NUM_INVADER_COLORS = 4
};

enum Direction {
    TOP = 0,
    RIGHT = 1,
    BOTTOM = 2,
    LEFT = 3
};

// Invader
struct Player {
	int health;
	int keys;
	bool is_on_ice;
};

struct Harmful {
	int damage;
};

struct Projectile {
};

struct SniperBullet {
};

struct Boomerang {
	glm::vec2 start_pos;
	glm::vec2 end_pos;
	glm::vec2 start_control;
	glm::vec2 end_control;
	float elapsed;
	float duration;
	bool reverse; // travelling forward or backward
};

// used for edible entities
struct Eatable
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position = { 0, 0 };
	float angle    = 0;
	vec2  velocity = { 0, 0 };
	vec2  scale    = { 10, 10 };
};

// Stucture to store collision information
struct Collision
{
	entt::entity entity1; 
	entt::entity entity2;
	Collision(entt::entity& entity1, entt::entity& entity2) { this->entity1 = entity1; this->entity2 = entity2; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	bool show_vignette = false;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct GridLine {
	vec2 start_pos = {  0,  0 };
	vec2 end_pos   = { 10, 10 };	// default to diagonal line
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

typedef vec3 Color;

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

typedef Mesh* MeshPtr;

struct Explosion
{

};

struct WeaponIndicator
{

};

struct Wall
{
	bool has_portal;
};

struct Floor
{
};

struct Door
{
	bool locked;
};

struct Exit
{

};

struct Key
{

};

struct Cheese
{
	int points;
};

struct Mousetrap
{

};

struct Animation
{
	int cur_frame_start_time;
	int cur_ind;
	int start_ind;
	int end_ind;
	bool loops;
};

// Store the other portal entity
struct Portal
{
    vec2 position;
    float range = GRID_CELL_WIDTH_PX;
    int direction;
    entt::entity other_portal = {}; 
};


struct PortalProximity
{
	entt::entity entity1; 
	entt::entity entity2;
};

struct Cat {
};

struct Patrol {
    std::vector<vec2> waypoints; // List of patrol points
    int currentTargetIndex = 0; // Index of the current target
    bool chasing = false; // Flag to check if chasing
    vec2 lastPatrolPos; // Store last patrol position before chasing
    int lastTargetIndex = 0; // Store the patrol waypoint it was heading toward
    bool reversing = false; // Flag to check if reversing patrol path
};

struct Sniper {
	float timer_ms;
	Direction direction;
};

struct Boid {
    float wanderAngle = 0.0f;
};

struct Tile {
    bool walkable; 
};

struct Cutscene {
};

struct FloorIce {
};

struct StartScreen {
};

struct SkipButton {
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	// mouse moving east
	MOUSE_1_EAST = 0,
	MOUSE_2_EAST = MOUSE_1_EAST + 1,
	MOUSE_3_EAST = MOUSE_2_EAST + 1,
	// mouse still facing east
	
	MOUSE_4_EAST = MOUSE_3_EAST + 1,
	MOUSE_5_EAST = MOUSE_4_EAST + 1,
	MOUSE_6_EAST = MOUSE_5_EAST + 1,
	// mouse moving west
	MOUSE_1_WEST = MOUSE_6_EAST + 1,
	MOUSE_2_WEST = MOUSE_1_WEST + 1,
	MOUSE_3_WEST = MOUSE_2_WEST + 1,
	// mouse still facing west
	MOUSE_4_WEST = MOUSE_3_WEST + 1,
	MOUSE_5_WEST = MOUSE_4_WEST + 1,
	MOUSE_6_WEST = MOUSE_5_WEST + 1,
	// mouse moving south
	MOUSE_1_SOUTH = MOUSE_6_WEST + 1,
	MOUSE_2_SOUTH = MOUSE_1_SOUTH + 1,
	MOUSE_3_SOUTH = MOUSE_2_SOUTH + 1,
	// mouse moving north
	MOUSE_1_NORTH = MOUSE_3_SOUTH + 1,
	MOUSE_2_NORTH = MOUSE_1_NORTH + 1,
	MOUSE_3_NORTH = MOUSE_2_NORTH + 1,

	// explosion assets
	EXPLOSION_1 = MOUSE_3_NORTH + 1,
	EXPLOSION_2 = EXPLOSION_1 + 1,
	EXPLOSION_3 = EXPLOSION_2 + 1,
	// remaining assets
	PATROL_CAT_1_WEST = EXPLOSION_3 + 1,
	PATROL_CAT_2_WEST = PATROL_CAT_1_WEST + 1,
	PATROL_CAT_3_WEST = PATROL_CAT_2_WEST + 1,
	PATROL_CAT_1_EAST = PATROL_CAT_3_WEST + 1,
	PATROL_CAT_2_EAST = PATROL_CAT_1_EAST + 1,
	PATROL_CAT_3_EAST = PATROL_CAT_2_EAST + 1,
	SNIPER_CAT_1 = PATROL_CAT_3_EAST + 1,
	SNIPER_CAT_2 = SNIPER_CAT_1 + 1,
	PROJECTILE = SNIPER_CAT_2 + 1,
	BOMERANGE = PROJECTILE + 1,
	NWE_WALL = BOMERANGE + 1,
	SOUTH_WALL = NWE_WALL + 1,
	FLOOR_TILE = SOUTH_WALL + 1,
	BLANK_TILE = FLOOR_TILE + 1,
	ICE_TILE = BLANK_TILE + 1,
	NORTH_PORTAL = ICE_TILE + 1,
	SOUTH_PORTAL = NORTH_PORTAL + 1,
	WEST_PORTAL = SOUTH_PORTAL + 1,
	EAST_PORTAL = WEST_PORTAL + 1,
	CLOSED_DOOR = EAST_PORTAL + 1,
	OPEN_DOOR = CLOSED_DOOR + 1,
	CLOSED_EXIT = OPEN_DOOR + 1,
	KEY = CLOSED_EXIT + 1,
	PORTAL = KEY + 1,
	CHEESE = PORTAL + 1,
	MOUSETRAP = CHEESE + 1,
	WEAPON_INDICATOR = MOUSETRAP + 1,
	// cutscene assets
	TEST_1 = WEAPON_INDICATOR + 1,
	TEST_2 = TEST_1 + 1,
	TEST_3 = TEST_2 + 1,
	START_SCREEN = TEST_3 + 1,
	START_SCREEN_2 = START_SCREEN + 1,
	CUTSCENE_1 = START_SCREEN_2 + 1,
	CUTSCENE_2 = CUTSCENE_1 + 1,
	CUTSCENE_3 = CUTSCENE_2 + 1,
	CUTSCENE_4 = CUTSCENE_3 + 1,
	CUTSCENE_5 = CUTSCENE_4 + 1,
	BACKGROUND_UI = CUTSCENE_5 + 1,
	TUTORIAL_UI = BACKGROUND_UI + 1,

	SKIP_BUTTON = TUTORIAL_UI + 1,
	TEXTURE_COUNT = SKIP_BUTTON + 1
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	VIGNETTE = TEXTURED + 1,
	FONT = VIGNETTE + 1,
	EFFECT_COUNT = FONT + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	PROJECTILE = CHICKEN + 1,
    MOUSETRAP = PROJECTILE + 1,
    MOUSEHORIZONTAL = MOUSETRAP + 1,
    MOUSEVERTICAL = MOUSEHORIZONTAL + 1,
    EVERYTHING = MOUSEVERTICAL + 1,
    TEST = EVERYTHING + 1,
	SPRITE = TEST + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	int z = 0;
};

struct TextRenderRequest {
	vec2  position = { 0, 0 };
	vec2  scale    = { 10, 10 };
	std::string text = "0";
	int z = 10;
};

struct UI {
	
};

enum class GAME_SCREEN_ID {
	START_SCREEN = 0,
	CUTSCENE = START_SCREEN + 1,
	PLAYING = CUTSCENE + 1,
	GAME_SCREEN_COUNT = PLAYING + 1
};
