// boids_system.hpp
#pragma once

#include "common.hpp"
#include <vector>
#include "tinyECS/registry.hpp"
#include "render_system.hpp"

// Flocking behavior parameters
constexpr float BOID_MAX_SPEED = 100.0f;
constexpr float BOID_MAX_FORCE = 0.5f;
constexpr float BOID_SEPARATION_RADIUS = 50.0f;
constexpr float BOID_NEIGHBOR_RADIUS = 70.0f;
constexpr float BOID_SEPARATION_WEIGHT = 1.5f;
constexpr float BOID_ALIGNMENT_WEIGHT = 1.0f;
constexpr float BOID_COHESION_WEIGHT = 1.0f;
constexpr float BOID_WANDER_WEIGHT = 0.3f;
const float padding = 50.0f; // Small padding from the screen edge

// System to manage boid behaviors
class BoidsSystem {
public:
    // Initialize the system
    BoidsSystem();

    // Create a new boid at the specified position
    static entt::entity createBoid(RenderSystem* renderer, vec2 position);

    // Create multiple boids randomly distributed within a radius
    static void createBoidsFlock(RenderSystem* renderer, vec2 center, float radius, int count);

    // Update all boids (call in WorldSystem::step)
    static void updateBoids(float elapsed_ms);

private:
    // BOID behaviors (calculated for each boid)
    static vec2 separation(entt::entity entity, const std::vector<entt::entity>& neighbors);
    static vec2 alignment(entt::entity entity, const std::vector<entt::entity>& neighbors);
    static vec2 cohesion(entt::entity entity, const std::vector<entt::entity>& neighbors);
    static vec2 wander(entt::entity entity);
    
    // Helper functions
    static vec2 limit(const vec2& vector, float max);
    static vec2 seek(entt::entity entity, const vec2& target);
    static std::vector<entt::entity> getNeighbors(entt::entity entity, float radius);
};