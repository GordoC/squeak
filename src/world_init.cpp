#include "world_init.hpp"
#include "common.hpp"
#include "map_system.hpp"
#include "tinyECS/registry.hpp"
#include "world_system.hpp"
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
entt::entity createPlayer(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = registry.create();

	// invader
	Player& player = registry.emplace<Player>(entity);
	player.health = 10;
	player.keys = 0;

	Animation& animation = registry.emplace<Animation>(entity);
	animation.loops = true;
	animation.cur_frame_start_time = (float)(glfwGetTime() * 10.0f);
	animation.cur_ind = int(TEXTURE_ASSET_ID::MOUSE_4_EAST);
	animation.start_ind = int(TEXTURE_ASSET_ID::MOUSE_4_EAST);
	animation.end_ind = int(TEXTURE_ASSET_ID::MOUSE_6_EAST);

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::CHICKEN);
	registry.emplace<MeshPtr>(entity, &mesh);

	// TODO A1: initialize the position, scale, and physics components
	auto& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	// resize, set scale to negative if you want to make it face the opposite way
	// motion.scale = vec2({ -INVADER_BB_WIDTH, INVADER_BB_WIDTH });
	motion.scale = vec2({ INVADER_BB_WIDTH, INVADER_BB_HEIGHT });

	// create an (empty) Bug component to be able to refer to all bug
	registry.emplace<Eatable>(entity);
	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID(animation.cur_ind),
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		2
	);

	return entity;
}

entt::entity createPortalBullet(RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = registry.create();
	registry.emplace<Projectile>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = pos;
	motion.scale = size;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PROJECTILE);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::PROJECTILE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		2
	);

	return entity;
}

entt::entity createWeaponIndicator(vec2 pos, float rotation) {
	auto entity = registry.create();
	registry.emplace<WeaponIndicator>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.position = pos;
	motion.angle = rotation;
	motion.scale = {20, 20};

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::WEAPON_INDICATOR,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		4
	);

	return entity;
}

entt::entity createText(std::string text, vec2 pos, vec2 scale) {
	auto entity = registry.create();

	TextRenderRequest& rr = registry.emplace<TextRenderRequest>(entity);
	rr.position = pos;
	rr.scale = scale;
	rr.text = text;

	return entity;
}

entt::entity createSkipButton(vec2 pos, vec2 scale) {
	std::cout << "Creating skip button" << std::endl;
	auto entity = registry.create();
	registry.emplace<SkipButton>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.position = pos;
	motion.scale = scale;
	motion.velocity = {0, 0};

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::SKIP_BUTTON, 
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		11
	);

	std::cout << "Button created at: (" << pos.x << ", " << pos.y << ")" << std::endl;

	return entity;
}


entt::entity createExplosion(vec2 pos) {
	entt::entity entity = registry.create();

	Motion& motion = registry.emplace<Motion>(entity);
	motion.position = pos;
	motion.scale = {50, 50};

	Animation& animation = registry.emplace<Animation>(entity);
	animation.cur_frame_start_time = (float)(glfwGetTime() * 10.0f);
	animation.start_ind = int(TEXTURE_ASSET_ID::EXPLOSION_1);
	animation.end_ind = int(TEXTURE_ASSET_ID::EXPLOSION_3);
	animation.cur_ind = int(TEXTURE_ASSET_ID::EXPLOSION_1);
	animation.loops = false;

	registry.emplace<Explosion>(entity);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::EXPLOSION_1,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		3
	);

	return entity;
}

entt::entity createNWEWall(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Wall>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::NWE_WALL,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		2
	);

	// if south facing wall
	return entity;
}


entt::entity createSouthWall(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Wall>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::SOUTH_WALL,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		2
	);

	// if south facing wall
	return entity;
}


entt::entity createFloorTile(vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Floor>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::FLOOR_TILE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		0
	);

	// if south facing wall
	return entity;
}


entt::entity createBlankTile(vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Wall>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::BLANK_TILE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);

	// if south facing wall
	return entity;
}

entt::entity createDoor(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	Door &door = registry.emplace<Door>(entity);
	door.locked = true;

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::CLOSED_DOOR,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);
	return entity;
}

entt::entity createExit(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Exit>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::CLOSED_EXIT,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);
	return entity;
}

entt::entity createKey(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Key>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::KEY,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);
	return entity;
}

entt::entity createFloorIce(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<FloorIce>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::ICE_TILE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);
	return entity;
}


entt::entity createCheese(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	Cheese &cheese = registry.emplace<Cheese>(entity);
	cheese.points = CHEESE_POINTS;

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::MOUSETRAP);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::CHEESE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);
	
	return entity;
}

entt::entity createMousetrap(RenderSystem* renderer, vec2 position) {
	entt::entity entity = registry.create();

	registry.emplace<Mousetrap>(entity);

	Harmful& harmful = registry.emplace<Harmful>(entity);
	harmful.damage = HARMFUL_DAMAGE;

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::MOUSETRAP);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::MOUSETRAP,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);
	
	return entity;
}

entt::entity createPortal(RenderSystem* renderer, vec2 position, vec2 scale, int direction, std::optional<entt::entity> previous_portal) {
	entt::entity entity = registry.create();

	Portal &portal = registry.emplace<Portal>(entity);
	portal.direction = direction;
	portal.position = position;
	
	if (previous_portal.has_value()) {
		portal.other_portal = previous_portal.value();
		Portal &portal_other = registry.get<Portal>(previous_portal.value());
		portal_other.other_portal = entity;
	}

	Motion &motion = registry.emplace<Motion>(entity);
	
	TEXTURE_ASSET_ID texture_id;
	if (direction == Direction::TOP) {
		// motion.angle = 180.f;
		texture_id = TEXTURE_ASSET_ID::NORTH_PORTAL;
	}
	else if (direction == Direction::RIGHT) {
		// motion.angle = 270.f;
		texture_id = TEXTURE_ASSET_ID::EAST_PORTAL;
	}
	else if (direction == Direction::BOTTOM) {
		// motion.angle = 0.f;
		texture_id = TEXTURE_ASSET_ID::PORTAL;
	}
	else if (direction == Direction::LEFT) {
		// motion.angle = 90.f;
		texture_id = TEXTURE_ASSET_ID::WEST_PORTAL;
	}

	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		texture_id,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		3
	);

	return entity;
}

entt::entity createPatrolEnemy(RenderSystem* renderer, std::vector<vec2> positions) {
    entt::entity entity = registry.create();

	Harmful& harmful = registry.emplace<Harmful>(entity);
	harmful.damage = HARMFUL_DAMAGE;

	registry.emplace<Cat>(entity);

    Motion& motion = registry.emplace<Motion>(entity);
    motion.position = positions[0]; 
    motion.velocity = { 0, 0 };
    motion.scale = vec2({ GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX });

    Patrol& patrol = registry.emplace<Patrol>(entity);
    for (vec2 pos : positions) {
        patrol.waypoints.push_back(pos);
    }

    patrol.currentTargetIndex = 0; 

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::MOUSETRAP);
	registry.emplace<MeshPtr>(entity, &mesh);

    registry.emplace<RenderRequest>(
        entity,
        TEXTURE_ASSET_ID::PATROL_CAT_1_WEST, 
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE,
        2
    );

	Animation anim;
    anim.cur_frame_start_time = (float)(glfwGetTime() * 10.0f);
    anim.cur_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_1_WEST;
    anim.start_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_1_WEST;
    anim.end_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_3_WEST;
    anim.loops = true;
    registry.emplace<Animation>(entity, anim);

    return entity;
}

entt::entity createSniperEnemy(RenderSystem* renderer, vec2 position, Direction direction)
{
	auto entity = registry.create();

	// new tower
	Sniper& sniper = registry.emplace<Sniper>(entity);
	sniper.timer_ms = SNIPER_TIMER_MS;
	sniper.direction = direction;
	
	registry.emplace<Cat>(entity);

	Harmful& harmful = registry.emplace<Harmful>(entity);
	harmful.damage = HARMFUL_DAMAGE;

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.emplace<Motion>(entity);
	if (direction == Direction::TOP) {
		motion.angle = 0.0f;
	} else if (direction == Direction::RIGHT) {
		motion.angle = 90.0f;
	} else if (direction == Direction::BOTTOM) {
		motion.angle = 180.0f;
	} else if (direction == Direction::LEFT) {
		motion.angle = 270.0f;
	}
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ CAT_BB_WIDTH, CAT_BB_HEIGHT });

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::SNIPER_CAT_1,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		1
	);

	return entity;
}

entt::entity createSniperBullet(RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity, int damage)
{
	auto entity = registry.create();
	registry.emplace<SniperBullet>(entity);

	Harmful& harmful = registry.emplace<Harmful>(entity);
	harmful.damage = damage;

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = pos;
	motion.scale = size;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::PROJECTILE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		2
	);

	return entity;
}

entt::entity createBoomerang(RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity, int damage, vec2 start, vec2 end, vec2 control_start, vec2 control_end)
{
	auto entity = registry.create();
	Boomerang& boomerang = registry.emplace<Boomerang>(entity);
	// bezier curve paramters
	boomerang.start_pos = start;
	boomerang.end_pos = end;
	boomerang.start_control = control_start;
	boomerang.end_control = control_end;
	boomerang.elapsed = 0.f;
	boomerang.duration = 2000.f; // Set an appropriate duration in milliseconds
	boomerang.reverse = false;   // Start in forward direction


	Harmful& harmful = registry.emplace<Harmful>(entity);
	harmful.damage = damage;

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = pos;
	motion.scale = size;
	

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PROJECTILE);
	registry.emplace<MeshPtr>(entity, &mesh);

	registry.emplace<RenderRequest>(
		entity,
		TEXTURE_ASSET_ID::BOMERANGE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		2
	);

	return entity;
}

entt::entity createCutscene(int cutscene_index) {
	entt::entity entity = registry.create();

	registry.emplace<Cutscene>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 };
	motion.scale = { WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX };

	registry.emplace<RenderRequest>(
		entity,
		static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::CUTSCENE_1) + cutscene_index),
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		15
	);

	return entity;
}

entt::entity createUI(RenderSystem* renderer, TEXTURE_ASSET_ID texture, int z) {
	auto entity = registry.create();

	registry.emplace<UI>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 };
	motion.scale = { WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX };

	registry.emplace<RenderRequest>(
		entity,
		texture,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		z
	);

	return entity;
}

entt::entity createStartScreen() {
	entt::entity entity = registry.create();

	registry.emplace<StartScreen>(entity);

	Motion& motion = registry.emplace<Motion>(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 };
	motion.scale = { WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX };


	TEXTURE_ASSET_ID texture_id;

	if (is_level_saved()) {
		std::cout << "Level is not saved" << std::endl;
		texture_id = TEXTURE_ASSET_ID::START_SCREEN;
	} else {
		std::cout << "Level is saved" << std::endl;
		texture_id = TEXTURE_ASSET_ID::START_SCREEN_2;
	}
	
	registry.emplace<RenderRequest>(
		entity,
		texture_id,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		10
	);

	return entity;
}

bool is_level_saved() {
    std::ifstream inFile(std::string(PROJECT_SOURCE_DIR) + "data/user/saved_level.txt");
    
    // Check if file opened successfully
    if (!inFile.is_open()) {
        std::cout << "Could not open file: " << std::endl;
        return true; // Treating unopenable files as empty
    }
    
    // Check if the file is empty by peeking at the first character
    bool isEmpty = inFile.peek() == std::ifstream::traits_type::eof();
    
    inFile.close();
    
    if (isEmpty) {
        std::cout << "File is empty: " << std::endl;
    } else {
        std::cout << "File contains data: " << std::endl;
    }
    
    return !isEmpty;
}