#pragma once

#include <array>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.hpp"
#include "tinyECS/components.hpp"


// fonts
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PROJECTILE, mesh_path("portal_bubble.obj")),
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::MOUSETRAP, mesh_path("mousetrap.obj")),
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::MOUSEHORIZONTAL, mesh_path("mouse_horizontal.obj")),
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::MOUSEVERTICAL, mesh_path("mouse_vertical.obj")),
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::EVERYTHING, mesh_path("everything.obj")),
        std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::TEST, mesh_path("test.obj")),
		// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		// mouse facing east
		textures_path("mouse/east_mouse_1.png"),
		textures_path("mouse/east_mouse_2.png"),
		textures_path("mouse/east_mouse_3.png"),
		// mouse still facing east
		textures_path("mouse/east_mouse_4.png"),
		textures_path("mouse/east_mouse_5.png"),
		textures_path("mouse/east_mouse_6.png"),
		// mouse facing west
		textures_path("mouse/west_mouse_1.png"),
		textures_path("mouse/west_mouse_2.png"),
		textures_path("mouse/west_mouse_3.png"),
		// mouse still facing west
		textures_path("mouse/west_mouse_4.png"),
		textures_path("mouse/west_mouse_5.png"),
		textures_path("mouse/west_mouse_6.png"),
		// mouse facing south
		textures_path("mouse/south_mouse_1.png"),
		textures_path("mouse/south_mouse_2.png"),
		textures_path("mouse/south_mouse_3.png"),
		// mouse facing north
		textures_path("mouse/north_mouse_1.png"),
		textures_path("mouse/north_mouse_2.png"),
		textures_path("mouse/north_mouse_3.png"),

		// explosion asset paths
		textures_path("effects/explosion1.png"),
		textures_path("effects/explosion2.png"),
		textures_path("effects/explosion3.png"),
		// other assets
		textures_path("cat/west_patrol_cat_1.png"),
		textures_path("cat/west_patrol_cat_2.png"),
		textures_path("cat/west_patrol_cat_3.png"),
		textures_path("cat/east_patrol_cat_1.png"),
		textures_path("cat/east_patrol_cat_2.png"),
		textures_path("cat/east_patrol_cat_3.png"),
		textures_path("cat/sniper_cat_1.png"),
		textures_path("cat/sniper_cat_2.png"),
		textures_path("projectiles/portal_bubble.png"),
		textures_path("projectiles/boomerang.png"),
		textures_path("environment/NWE_wall.png"),
		textures_path("environment/south_wall.png"),
		textures_path("environment/floor_tile.png"),
		textures_path("environment/blank_tile.png"),
		textures_path("environment/ice_tile.png"),
		textures_path("environment/north_portal.png"),
		textures_path("environment/south_portal.png"),
		textures_path("environment/west_portal.png"),
		textures_path("environment/east_portal.png"),
		textures_path("environment/closed_door.png"),
		textures_path("environment/open_door.png"),
		textures_path("environment/closed_exit.png"),
		textures_path("environment/key.png"),
		textures_path("environment/mouseportal.png"),
		textures_path("environment/cheese.png"),
		textures_path("environment/mousetrap.png"),
		textures_path("effects/weapon_indicator.png"),
		// cutscene assets
		textures_path("cutscenes/test1.jpeg"),
		textures_path("cutscenes/test2.jpeg"),
		textures_path("cutscenes/test3.jpeg"),
		textures_path("cutscenes/start_screen.png"),
		textures_path("cutscenes/start_screen2.png"),
		textures_path("cutscenes/cutscene1.png"),
		textures_path("cutscenes/cutscene2.png"),
		textures_path("cutscenes/cutscene3.png"),
		textures_path("cutscenes/cutscene4.png"),
		textures_path("cutscenes/cutscene5.png"),

		textures_path("ui/background.png"),
		textures_path("ui/tutorial.png"),
		// assets
		textures_path("assets/skip_button.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("vignette"),
		shader_path("font")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

	// last invader-tower collision time
	float last_invader_tower_collision_time = (float)(glfwGetTime() * 10.0f) - 10.0; // initialize to a value before current time
	float death_time = (float)(glfwGetTime() * 10.0f) - 10.0;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	void load_font();

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(GAME_SCREEN_ID game_screen);

	mat3 createProjectionMatrix();

	entt::entity get_screen_state_entity() { return screen_state_entity; }

	void set_last_invader_tower_collision_time(float time) {last_invader_tower_collision_time = time; }

	void set_death_time(float time) {death_time = time; }

private:
	// Internal drawing functions for each entity type
	void drawGridLine(entt::entity entity, const mat3& projection);
	void drawTexturedMesh(entt::entity entity, const mat3& projection);
	void drawText(entt::entity entity, const mat3& projection);
	void drawChar(char c, glm::vec2 pos, glm::vec2 scale, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	entt::entity screen_state_entity;

	// fonts
	FT_Library ft;
    FT_Face face;
    std::map<char, Character> characters;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
	