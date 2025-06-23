// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <glm/geometric.hpp>
#include <iostream>
#include "tinyECS/registry.hpp"
#include "world_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// debug purposes
void drawDebugPolygon(const std::vector<vec2>& points, vec3 color = {1, 0, 0})
{
	for (size_t i = 0; i < points.size(); ++i) {
		vec2 start = points[i];
		vec2 end = points[(i + 1) % points.size()]; 

		entt::entity line = registry.create();

		GridLine& grid_line = registry.emplace<GridLine>(line);
		grid_line.start_pos = start;
		grid_line.end_pos = end;

		registry.emplace<RenderRequest>(
			line,
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE,
			99 
		);

		registry.emplace<Color>(line, color);
	}
}

// debug purposes
void drawMeshOutline(entt::entity entity, vec3 color = {0, 1, 0})
{
    if (!registry.all_of<MeshPtr, Motion>(entity))
        return;

    const Mesh& mesh = *registry.get<MeshPtr>(entity);
    const Motion& motion = registry.get<Motion>(entity);

    float angle = motion.angle * 3.14159265359f / 180.0f;
    mat2 rotation = {
        {cos(angle), -sin(angle)},
        {sin(angle),  cos(angle)}
    };

    std::vector<vec2> transformed;
    for (const auto& v : mesh.vertices)
    {
        vec2 local = vec2(v.position);          
        vec2 scaled = local * motion.scale;     
        vec2 rotated = rotation * scaled;       
        vec2 world = rotated + motion.position; 

        transformed.push_back(world);
    }

    drawDebugPolygon(transformed, color);
}

// debug purposes
void drawAABB(const Motion& motion, vec3 color = {1, 0, 0})
{
	vec2 pos = motion.position;
	vec2 half = get_bounding_box(motion) / 2.f;

	std::vector<vec2> box = {
		pos + vec2{-half.x, -half.y},
		pos + vec2{ half.x, -half.y},
		pos + vec2{ half.x,  half.y},
		pos + vec2{-half.x,  half.y},
	};

	drawDebugPolygon(box, color);
}

bool boundingBoxOverlap(const Motion& motion1, const Motion& motion2)
{
    vec2 halfSize1 = abs(get_bounding_box(motion1)) / 2.f;
    vec2 halfSize2 = abs(get_bounding_box(motion2)) / 2.f;

    vec2 pos1 = motion1.position;
    vec2 pos2 = motion2.position;

    bool overlapX = (pos1.x - halfSize1.x < pos2.x + halfSize2.x) && (pos1.x + halfSize1.x > pos2.x - halfSize2.x);

    bool overlapY = (pos1.y - halfSize1.y < pos2.y + halfSize2.y) && (pos1.y + halfSize1.y > pos2.y - halfSize2.y);

    return overlapX && overlapY;
}

bool collides(entt::entity entity1, entt::entity entity2)
{
    if (!registry.all_of<Motion, MeshPtr>(entity1) || !registry.all_of<Motion, MeshPtr>(entity2))
        return false;

    const Motion& motion1 = registry.get<Motion>(entity1);
    const Motion& motion2 = registry.get<Motion>(entity2);

    const Mesh& mesh1 = *registry.get<MeshPtr>(entity1);
    const Mesh& mesh2 = *registry.get<MeshPtr>(entity2);

    // drawMeshOutline(entity1, vec3(0,1,0)); 
    // drawMeshOutline(entity2, vec3(1,0,0));

    auto transformVerts = [](const Mesh& mesh, const Motion& motion) -> std::vector<vec2> {
        std::vector<vec2> out;
        float angle = motion.angle * 3.14159265359f / 180.0f;
        mat2 rot = {
            {cos(angle), -sin(angle)},
            {sin(angle),  cos(angle)}
        };

        for (auto& v : mesh.vertices) {
            vec2 local = vec2(v.position);
            vec2 scaled = local * motion.scale;
            vec2 rotated = rot * scaled;
            vec2 world = rotated + motion.position;
            out.push_back(world);
        }
        return out;
    };

    std::vector<vec2> verts1 = transformVerts(mesh1, motion1);
    std::vector<vec2> verts2 = transformVerts(mesh2, motion2);

    auto pointInPolygon = [](const vec2& p, const std::vector<vec2>& poly) -> bool {
        int count = 0;
        for (size_t i = 0; i < poly.size(); ++i) {
            vec2 a = poly[i];
            vec2 b = poly[(i + 1) % poly.size()];

            if ((a.y > p.y) != (b.y > p.y)) {
                float dy = b.y - a.y;
                if (fabs(dy) < 1e-6) continue; 
                float x = (b.x - a.x) * (p.y - a.y) / dy + a.x;
                if (p.x < x)
                    count++;
            }
        }
        return (count % 2) == 1;
    };

    for (const vec2& v : verts1) {
        if (pointInPolygon(v, verts2))
            return true;
    }

    for (const vec2& v : verts2) {
        if (pointInPolygon(v, verts1)) 
            return true;
    }

    return false;
}

glm::vec2 getBezierPosition(const Boomerang& path, float t) {
    // Ensure t is between 0 and 1
    t = std::max(0.f, std::min(1.f, t));
    
    // Cubic Bézier formula: B(t) = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    float t2 = t * t;
    float t3 = t2 * t;
    
    return mt3 * path.start_pos + 
           3.0f * mt2 * t * path.start_control + 
           3.0f * mt * t2 * path.end_control + 
           t3 * path.end_pos;
}

glm::vec2 getBezierVelocity(const Boomerang& path, float t) {
    // The derivative of the Bézier curve gives us the tangent/velocity
    // B'(t) = 3(1-t)²(P₁-P₀) + 6(1-t)t(P₂-P₁) + 3t²(P₃-P₂)
    t = std::max(0.f, std::min(1.f, t));
    
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float t2 = t * t;
    
    return 3.0f * mt2 * (path.start_control - path.start_pos) + 
           6.0f * mt * t * (path.end_control - path.start_control) + 
           3.0f * t2 * (path.end_pos - path.end_control);
}

bool isNearPortal(entt::entity entity, entt::entity portal)
{
    if (!registry.all_of<Motion>(entity) || !registry.all_of<Motion, Portal>(portal))
        return false;

    const Motion& entity_motion = registry.get<Motion>(entity);
    const Portal& portal_data = registry.get<Portal>(portal);
    
    // Calculate distance between entity center and portal center
    vec2 entity_pos = entity_motion.position;
    vec2 portal_pos = portal_data.position;
    
    std::cout << "Portal Data position:" << portal_pos.x << "," << portal_pos.y << std::endl;
    std::cout << "Player position:" << entity_pos.x << "," << entity_pos.y << std::endl;

    vec2 diff = entity_pos - portal_pos;
    float distance = 0;
    distance = glm::length(diff);
    
    // If within activation radius, return true
    return distance <= 75.0f;
}



// Add this new function to check all portals
void checkPortalProximity()
{
    // Initialize as false at the beginning of the function
    WorldSystem::can_teleport = false;
    WorldSystem::nearby_player_entity = entt::null;
    WorldSystem::nearby_wall_entity = entt::null;
    
    // Get all portals
    auto portal_view = registry.view<Portal, Motion>();
    
    // Check against all players that can interact with portals
    auto entities = registry.view<Player>();
    
    for (auto entity : entities)
    {
        // Only check players
        if (!registry.all_of<Player>(entity))
            continue;
            
        for (auto portal : portal_view)
        {
            Portal& portal_data = registry.get<Portal>(portal);
            
            // If player is near this portal
            if (isNearPortal(entity, portal))
            {
                // Set the static variables to enable teleportation
                WorldSystem::can_teleport = true;
                WorldSystem::nearby_player_entity = entity;
                
                // Find the corresponding wall
                auto wall_view = registry.view<Wall>();
                for (auto wall : wall_view) {
                    Wall& wall_data = registry.get<Wall>(wall);
                    Motion& wall_motion = registry.get<Motion>(wall);
                    
                    if (wall_motion.position == portal_data.position && wall_data.has_portal) {
                        WorldSystem::nearby_wall_entity = wall;
                        // Once we found a valid portal/wall combination, we can stop searching
                        return;
                    }
                }
            }
            // IMPORTANT: Remove the else clause that sets can_teleport = false
        }
    }
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto motion_view = registry.view<Motion>();
	for (auto motion_entity : motion_view)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity [DONE]
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		Motion& motion = registry.get<Motion>(motion_entity);
		float step_seconds = elapsed_ms / 1000.f;

        if (registry.all_of<Boomerang>(motion_entity)) {
            Boomerang& path = registry.get<Boomerang>(motion_entity);
            
            // Update elapsed time
            path.elapsed += elapsed_ms;
            
            // Calculate normalized time (0-1)
            float completionRatio = path.elapsed / path.duration;
            
            if (completionRatio >= 1.0f) {
                path.reverse = !path.reverse;
                // path.elapsed = path.duration - (path.elapsed - path.duration);
                path.elapsed = 0.0f;

                completionRatio = path.elapsed / path.duration;
            }
            
            // Calculate t based on direction
            float t = path.reverse ? 1.0f - completionRatio : completionRatio;
            
            // Set position using Bézier curve
            motion.position = getBezierPosition(path, t);
            
            // Update velocity to match curve tangent
            // Reverse direction if going backward
            glm::vec2 tangent = getBezierVelocity(path, t);
            motion.velocity = path.reverse ? -tangent * 0.01f : tangent * 0.01f;
            
            // Skip the standard velocity-based movement
            continue;
        }

		motion.position += step_seconds * motion.velocity;
	}

	// check for collisions between all moving entities
	auto it_i = motion_view.begin();
    for (it_i; it_i != motion_view.end(); ++it_i)
    {
        bool isMouseTrap = false;
        bool isWall = false;

        entt::entity entity_i = *it_i;
        if (registry.all_of<Floor>(entity_i))
            continue; 

        if (registry.all_of<Wall>(entity_i))
            isWall = true;
        
        if (registry.all_of<Mousetrap>(entity_i))
            isMouseTrap = true;
        
        for (auto it_j = std::next(it_i); it_j != motion_view.end(); ++it_j)
        {   
            entt::entity entity_j = *it_j;
            if (registry.all_of<Floor>(entity_j))
                continue; 

            if ((registry.all_of<Wall>(entity_j) || registry.all_of<Mousetrap>(entity_j)) && (isMouseTrap || isWall)) {
                continue;
            }

            // check AABB overlap (optimization)
            Motion& motion_i = registry.get<Motion>(entity_i);
            Motion& motion_j = registry.get<Motion>(entity_j);
            if (!boundingBoxOverlap(motion_i, motion_j)) 
                continue; 

            // mesh collision
            if (collides(entity_i, entity_j))
            {
                if (!registry.all_of<WeaponIndicator>(entity_i) && !registry.all_of<WeaponIndicator>(entity_j)) {
                    entt::entity collision = registry.create();
                    registry.emplace<Collision>(collision, entity_i, entity_j);
                }
            }
        }
    }

    // Only check Portal proximity if there is one made
    auto portal_view = registry.view<Portal>();
    if (portal_view.size() > 0) {
        checkPortalProximity();
    }

}
