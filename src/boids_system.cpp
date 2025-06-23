#include "boids_system.hpp"
#include "world_init.hpp"
#include <random>
#include <cmath>

BoidsSystem::BoidsSystem() {
}

entt::entity BoidsSystem::createBoid(RenderSystem* renderer, vec2 position) {
    // Create a new entity
    auto entity = registry.create();
    
    registry.emplace<Boid>(entity);

    Harmful& harmful = registry.emplace<Harmful>(entity);
	harmful.damage = HARMFUL_DAMAGE;

	registry.emplace<Cat>(entity);
    
    Motion& motion = registry.emplace<Motion>(entity);
    motion.position = position;
    motion.angle = 0.0f;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    motion.velocity = vec2(dis(gen), dis(gen)) * 50.0f;
    
    motion.scale = vec2(40.0f, 40.0f);
    
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::EVERYTHING);
	registry.emplace<MeshPtr>(entity, &mesh);
    
    registry.emplace<RenderRequest>(
        entity,
        TEXTURE_ASSET_ID::SNIPER_CAT_2,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE,
        5
    );
    
    return entity;
}

void BoidsSystem::createBoidsFlock(RenderSystem* renderer, vec2 center, float radius, int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disRadius(0.0f, radius);
    std::uniform_real_distribution<float> disAngle(0.0f, 2.0f * M_PI);
    
    for (int i = 0; i < count; i++) {
        float angle = disAngle(gen);
        float r = disRadius(gen);
        vec2 position = center + vec2(cos(angle) * r, sin(angle) * r);
        createBoid(renderer, position);
    }
}

void BoidsSystem::updateBoids(float elapsed_ms) {
    float deltaTime = elapsed_ms / 1000.0f;
    
    // Get all boids
    auto view = registry.view<Boid, Motion>();
    
    // Store all valid boid entities for neighbor calculations
    std::vector<entt::entity> allBoids;
    for (auto entity : view) {
        allBoids.push_back(entity);
    }
    
    // Update each boid
    for (auto entity : view) {
        // Get neighbors for this boid
        std::vector<entt::entity> neighbors = getNeighbors(entity, BOID_NEIGHBOR_RADIUS);
        
        // Calculate steering forces from flocking behaviors
        vec2 sep = separation(entity, neighbors) * BOID_SEPARATION_WEIGHT;
        vec2 ali = alignment(entity, neighbors) * BOID_ALIGNMENT_WEIGHT;
        vec2 coh = cohesion(entity, neighbors) * BOID_COHESION_WEIGHT;
        vec2 wan = wander(entity) * BOID_WANDER_WEIGHT;
        
        // Apply all forces to the boid's acceleration
        Motion& motion = view.get<Motion>(entity);
        vec2 acceleration = sep + ali + coh + wan;
        
        // Update velocity
        motion.velocity += acceleration;
        motion.velocity = limit(motion.velocity, BOID_MAX_SPEED);
        
        // Update position
        motion.position += motion.velocity * deltaTime;
        
        // Update rotation (point in the direction of movement)
        if (length(motion.velocity) > 0.001f) {
            motion.angle = atan2(motion.velocity.y, motion.velocity.x) * (180.0f / M_PI);
        }
        
        // Wrap around screen edges
        // Wrap around screen edges (safer version)
        if (motion.position.x < 0) {
            motion.position.x += WINDOW_WIDTH_PX;
        }
        if (motion.position.x > WINDOW_WIDTH_PX) {
            motion.position.x -= WINDOW_WIDTH_PX;
        }
        if (motion.position.y < 0) {
            motion.position.y += WINDOW_HEIGHT_PX;
        }
        if (motion.position.y > WINDOW_HEIGHT_PX) {
            motion.position.y -= WINDOW_HEIGHT_PX;
        }

        // Ensure position is within bounds
        if (motion.position.x < padding) {
            motion.position.x = padding;
            motion.velocity.x = std::abs(motion.velocity.x) * 0.5f; // Bounce with reduced velocity
        } else if (motion.position.x > WINDOW_WIDTH_PX - padding) {
            motion.position.x = WINDOW_WIDTH_PX - padding;
            motion.velocity.x = -std::abs(motion.velocity.x) * 0.5f; // Bounce with reduced velocity
        }

        if (motion.position.y < padding) {
            motion.position.y = padding;
            motion.velocity.y = std::abs(motion.velocity.y) * 0.5f; // Bounce with reduced velocity
        } else if (motion.position.y > WINDOW_HEIGHT_PX - padding) {
            motion.position.y = WINDOW_HEIGHT_PX - padding;
            motion.velocity.y = -std::abs(motion.velocity.y) * 0.5f; // Bounce with reduced velocity
        }
    }
}

vec2 BoidsSystem::separation(entt::entity entity, const std::vector<entt::entity>& neighbors) {
    const Motion& motion = registry.get<Motion>(entity);
    vec2 steer(0, 0);
    int count = 0;
    
    for (auto neighbor : neighbors) {
        if (neighbor == entity) continue;
        
        const Motion& neighborMotion = registry.get<Motion>(neighbor);
        vec2 diff = motion.position - neighborMotion.position;
        float distance = length(diff);
        
        if (distance < BOID_SEPARATION_RADIUS && distance > 0) {
            // Weight by distance (closer neighbors have more influence)
            vec2 normalizedDiff = normalize(diff);
            steer += normalizedDiff / distance;
            count++;
        }
    }
    
    if (count > 0) {
        steer = steer / static_cast<float>(count);
    }
    
    if (length(steer) > 0) {
        steer = normalize(steer) * BOID_MAX_SPEED;
        steer = steer - motion.velocity;
        steer = limit(steer, BOID_MAX_FORCE);
    }
    
    return steer;
}

vec2 BoidsSystem::alignment(entt::entity entity, const std::vector<entt::entity>& neighbors) {
    const Motion& motion = registry.get<Motion>(entity);
    vec2 sum(0, 0);
    int count = 0;
    
    for (auto neighbor : neighbors) {
        if (neighbor == entity) continue;
        
        const Motion& neighborMotion = registry.get<Motion>(neighbor);
        sum += neighborMotion.velocity;
        count++;
    }
    
    if (count > 0) {
        sum = sum / static_cast<float>(count);
        sum = normalize(sum) * BOID_MAX_SPEED;
        
        vec2 steer = sum - motion.velocity;
        return limit(steer, BOID_MAX_FORCE);
    }
    
    return vec2(0, 0);
}

vec2 BoidsSystem::cohesion(entt::entity entity, const std::vector<entt::entity>& neighbors) {
    const Motion& motion = registry.get<Motion>(entity);
    vec2 sum(0, 0);
    int count = 0;
    
    for (auto neighbor : neighbors) {
        if (neighbor == entity) continue;
        
        const Motion& neighborMotion = registry.get<Motion>(neighbor);
        sum += neighborMotion.position;
        count++;
    }
    
    if (count > 0) {
        sum = sum / static_cast<float>(count);
        return seek(entity, sum);
    }
    
    return vec2(0, 0);
}

vec2 BoidsSystem::wander(entt::entity entity) {
    Motion& motion = registry.get<Motion>(entity);
    Boid& boid = registry.get<Boid>(entity);
    
    // Update the wander angle with a small random value
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.3f, 0.3f);
    boid.wanderAngle += dis(gen);
    
    // Calculate the wander force
    vec2 circleCenter = normalize(motion.velocity);
    if (length(circleCenter) < 0.01f) {
        circleCenter = vec2(1, 0);
    }
    
    circleCenter *= 30.0f;
    
    vec2 displacement = vec2(cos(boid.wanderAngle), sin(boid.wanderAngle)) * 10.0f;
    vec2 wanderForce = circleCenter + displacement;
    
    return limit(wanderForce, BOID_MAX_FORCE);
}

vec2 BoidsSystem::seek(entt::entity entity, const vec2& target) {
    const Motion& motion = registry.get<Motion>(entity);
    
    vec2 desired = target - motion.position;
    if (length(desired) > 0) {
        desired = normalize(desired) * BOID_MAX_SPEED;
    }
    
    vec2 steer = desired - motion.velocity;
    return limit(steer, BOID_MAX_FORCE);
}

vec2 BoidsSystem::limit(const vec2& vector, float max) {
    if (length(vector) > max) {
        return normalize(vector) * max;
    }
    return vector;
}

std::vector<entt::entity> BoidsSystem::getNeighbors(entt::entity entity, float radius) {
    std::vector<entt::entity> neighbors;
    
    const Motion& motion = registry.get<Motion>(entity);
    auto view = registry.view<Boid, Motion>();
    
    for (auto neighbor : view) {
        if (neighbor == entity) continue;
        
        const Motion& neighborMotion = registry.get<Motion>(neighbor);
        float distance = length(motion.position - neighborMotion.position);
        
        if (distance < radius) {
            neighbors.push_back(neighbor);
        }
    }
    
    return neighbors;
}

entt::entity createBoid(RenderSystem* renderer, vec2 position);
void createBoidsFlock(RenderSystem* renderer, vec2 center, float radius, int count);

entt::entity createBoid(RenderSystem* renderer, vec2 position) {
    return BoidsSystem::createBoid(renderer, position);
}

void createBoidsFlock(RenderSystem* renderer, vec2 center, float radius, int count) {
    BoidsSystem::createBoidsFlock(renderer, center, radius, count);
}