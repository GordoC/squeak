#include <iostream>
#include "common.hpp"
#include "ai_system.hpp"
#include "world_init.hpp"
#include "tinyECS/registry.hpp"

void AISystem::step(RenderSystem* renderer, float elapsed_ms)
{
    animateCats();
    // Patrol AI
    processSniperCats(renderer, elapsed_ms);
    processPatrolCats(elapsed_ms);
}


// invader detection system for sniper cats
// - for each sniper, scan its direction
// - if Tom is detected and the tower's shooting timer has expired,
//   create a projectile in the direction of Tom and reset timer
void AISystem::processSniperCats(RenderSystem* renderer, float elapsed_ms) {
    auto sniper_view = registry.view<Sniper>();
    for (auto sniper_entity : sniper_view) {
        Sniper& sniper = registry.get<Sniper>(sniper_entity);
        vec2 sniper_pos = registry.get<Motion>(sniper_entity).position;
        sniper.timer_ms -= elapsed_ms;

        // skip shooting if still reloading
        if (sniper.timer_ms > 0) {
            continue;
        }

        auto player_view = registry.view<Player>();
        for (auto player_entity : player_view) {
            if (is_tom_visible_to_sniper(sniper_entity, player_entity)) {
                vec2 bullet_position;
                vec2 bullet_velocity;

                // shoot bullet in correct direction if sniper sees Tom
                switch (sniper.direction) {
                    case Direction::TOP:
                        bullet_position = {sniper_pos.x, sniper_pos.y - 2 * SNIPER_BULLET_SIZE.y};
                        bullet_velocity = {0, -SNIPER_BULLET_SPEED};
                        break;
                    case Direction::BOTTOM:
                        bullet_position = {sniper_pos.x, sniper_pos.y + 2 * SNIPER_BULLET_SIZE.y};
                        bullet_velocity = {0, SNIPER_BULLET_SPEED};
                        break;
                    case Direction::LEFT:
                        bullet_position = {sniper_pos.x - 2 * SNIPER_BULLET_SIZE.x, sniper_pos.y};
                        bullet_velocity = {-SNIPER_BULLET_SPEED, 0};
                        break;
                    case Direction::RIGHT:
                        bullet_position = {sniper_pos.x + 2 * SNIPER_BULLET_SIZE.x, sniper_pos.y};
                        bullet_velocity = {SNIPER_BULLET_SPEED, 0};
                        break;
                }

                createSniperBullet(renderer, bullet_position, SNIPER_BULLET_SIZE, bullet_velocity, HARMFUL_DAMAGE);
                sniper.timer_ms = SNIPER_TIMER_MS;
                break;
            }
        }
    }
}

// check if Tom is visible to the sniper cat based on the sniper cat's direction and if there are walls between them
bool AISystem::is_tom_visible_to_sniper(entt::entity sniper_entity, entt::entity player_entity) {
    Sniper& sniper = registry.get<Sniper>(sniper_entity);
    Direction direction = sniper.direction;

    vec2 sniper_pos = registry.get<Motion>(sniper_entity).position;
    int sniper_tile_x = (int)(sniper_pos.x / GRID_CELL_WIDTH_PX);
    int sniper_tile_y = (int)(sniper_pos.y / GRID_CELL_HEIGHT_PX);

    vec2 player_pos = registry.get<Motion>(player_entity).position;
    int player_tile_x = (int)(player_pos.x / GRID_CELL_WIDTH_PX);
    int player_tile_y = (int)(player_pos.y / GRID_CELL_HEIGHT_PX);

    // check if Tom and sniper are aligned and sniper is facing Tom
    if (direction == Direction::TOP) {
        if (player_tile_x != sniper_tile_x) {
            return false;
        }
        if (player_tile_y > sniper_tile_y) {
            return false;
        }
    } else if (direction == Direction::BOTTOM) {
        if (player_tile_x != sniper_tile_x) {
            return false;
        }
        if (player_tile_y < sniper_tile_y) {
            return false;
        }
    } else if (direction == Direction::LEFT) {
        if (player_tile_y != sniper_tile_y) {
            return false;
        }
        if (player_tile_x > sniper_tile_x) {
            return false;
        }
    } else if (direction == Direction::RIGHT) {
        if (player_tile_y != sniper_tile_y) {
            return false;
        }
        if (player_tile_x < sniper_tile_x) {
            return false;
        }
    }

    // check if there are any walls that block sniper's view of Tom
    for (const entt::entity &wall_entity : registry.view<Wall>()) {
        Motion& wall_motion = registry.get<Motion>(wall_entity);
        int wall_tile_x = (int)(wall_motion.position.x / GRID_CELL_WIDTH_PX);
        int wall_tile_y = (int)(wall_motion.position.y / GRID_CELL_HEIGHT_PX);
        if (is_blocking_view(wall_tile_x, wall_tile_y, sniper_tile_x, sniper_tile_y, player_tile_x, player_tile_y, direction)) {
            return false;
        }
    }

    // check if there are any locked doors that block sniper's view of Tom
    for (const entt::entity &door_entity : registry.view<Door>()) {
        Door &door = registry.get<Door>(door_entity);
        if (!door.locked) {
            continue;
        }

        Motion& door_motion = registry.get<Motion>(door_entity);
        int door_tile_x = (int)(door_motion.position.x / GRID_CELL_WIDTH_PX);
        int door_tile_y = (int)(door_motion.position.y / GRID_CELL_HEIGHT_PX);
        if (is_blocking_view(door_tile_x, door_tile_y, sniper_tile_x, sniper_tile_y, player_tile_x, player_tile_y, direction)) {
            return false;
        }
    }

    return true;
}

bool AISystem::is_blocking_view(int block_tile_x, int block_tile_y, int sniper_tile_x, int sniper_tile_y, int player_tile_x, int player_tile_y, Direction direction) {
    if (direction == Direction::TOP) {
        return block_tile_x == sniper_tile_x && block_tile_y < sniper_tile_y && block_tile_y > player_tile_y;
    } else if (direction == Direction::BOTTOM) {
        return block_tile_x == sniper_tile_x && block_tile_y > sniper_tile_y && block_tile_y < player_tile_y;
    } else if (direction == Direction::LEFT) {
        return block_tile_y == sniper_tile_y && block_tile_x < sniper_tile_x && block_tile_x > player_tile_x;
    } else if (direction == Direction::RIGHT) {
        return block_tile_y == sniper_tile_y && block_tile_x > sniper_tile_x && block_tile_x < player_tile_x;
    }
    return false;
}


// patrol cat AI
void AISystem::processPatrolCats(float elapsed_ms) {
    auto patrol_view = registry.view<Patrol, Motion>();
    auto player_view = registry.view<Player, Motion>();

    if (player_view.begin() == player_view.end()) {
        // Freeze all patrols if no players are present
        for (auto patrol_entity : patrol_view) {
            registry.get<Motion>(patrol_entity).velocity = {0, 0};
        }
        return;
    }

    for (auto entity : patrol_view) {
        Patrol& patrol = registry.get<Patrol>(entity);
        Motion& motion = registry.get<Motion>(entity);

        if (patrol.waypoints.empty()) continue;

        vec2 player_pos;
        bool player_nearby = false;

        // Check for nearby players
        for (auto player_entity : player_view) {
            Motion& player_motion = registry.get<Motion>(player_entity);
            player_pos = player_motion.position;

            float grid_distance_x = abs(player_pos.x - motion.position.x) / GRID_CELL_WIDTH_PX;
            float grid_distance_y = abs(player_pos.y - motion.position.y) / GRID_CELL_HEIGHT_PX;

            if (grid_distance_x <= PATROL_RANGE && grid_distance_y <= PATROL_RANGE && !is_blocked(motion.position, player_pos)) {
                player_nearby = true;
                break;
            }
        }

        vec2 direction = {0, 0};
        const float ARRIVAL_THRESHOLD = 5.0f; // increase for smoother arrival
        const float SNAP_THRESHOLD = 1.0f;    // if within threshold, snap to waypoint
        const float MIN_MOVEMENT = 0.05f;     // prevents oscillation 

        if (player_nearby) { 
            // first time detecting player so store last patrol position
            if (!patrol.chasing) {
                patrol.lastPatrolPos = motion.position;
                patrol.lastTargetIndex = patrol.currentTargetIndex;
                patrol.chasing = true;
            }

            // chase player
            vec2 chase_direction = player_pos - motion.position;
            if (length(chase_direction) > MIN_MOVEMENT) {
                direction = normalize(chase_direction);
            }
        } 
        else if (patrol.chasing) {
            // return to last patrol position
            vec2 return_direction = patrol.waypoints[patrol.lastTargetIndex] - motion.position;

            if (length(return_direction) > ARRIVAL_THRESHOLD) {
                direction = normalize(return_direction);
            } 
            else {
                // snap to waypoint and resume normal patrol
                motion.position = patrol.waypoints[patrol.lastTargetIndex];
                patrol.currentTargetIndex = patrol.lastTargetIndex;
                patrol.chasing = false;
            }
        } 
        else {
            // normal patrol movement
            vec2 target_pos = patrol.waypoints[patrol.currentTargetIndex];
            vec2 patrol_direction = target_pos - motion.position;

            if (length(patrol_direction) > MIN_MOVEMENT) {
                direction = normalize(patrol_direction);
            }

            if (length(target_pos - motion.position) < ARRIVAL_THRESHOLD) {
                if (length(target_pos - motion.position) < SNAP_THRESHOLD) {
                    // snap to avoid overshooting or stalling
                    motion.position = target_pos;
                }

                // move to next waypoint (reverse if at end of waypoints)
                if (!patrol.reversing) {
                    if (patrol.currentTargetIndex < patrol.waypoints.size() - 1) {
                        patrol.currentTargetIndex++;
                    } else {
                        patrol.reversing = true;
                        patrol.currentTargetIndex--;
                    }
                } else {
                    if (patrol.currentTargetIndex > 0) {
                        patrol.currentTargetIndex--;
                    } else {
                        patrol.reversing = false;
                        patrol.currentTargetIndex++;
                    }
                }
            }
        }

        // no small movement oscillations
        if (length(direction) > MIN_MOVEMENT) {
            motion.velocity = direction * PATROL_SPEED;
            motion.position += motion.velocity * (elapsed_ms / 1000.f);
        } else {
            motion.velocity = {0, 0}; // stop jittering
        }
    }
}

bool AISystem::is_blocked(vec2 start, vec2 end) {
    auto wall_view = registry.view<Wall, Motion>();

    // Define the number of steps for the raycast
    int steps = 10;  
    vec2 step_vector = (end - start) / float(steps);

    // Check along the path from enemy to player
    for (int i = 0; i <= steps; i++) {
        vec2 check_pos = start + step_vector * float(i);

        for (auto wall_entity : wall_view) {
            Motion& wall_motion = registry.get<Motion>(wall_entity);
            vec2 wall_pos = wall_motion.position;
            vec2 wall_size = wall_motion.scale;

            // Check if the current step position is inside a wall
            if (check_pos.x >= wall_pos.x - wall_size.x / 2 &&
                check_pos.x <= wall_pos.x + wall_size.x / 2 &&
                check_pos.y >= wall_pos.y - wall_size.y / 2 &&
                check_pos.y <= wall_pos.y + wall_size.y / 2) {
                return true; // Obstructed
            }
        }
    }

    return false; // No obstruction, clear sight
}

void AISystem::animateCats() {
    auto animated_cats = registry.view<Cat, Animation, Motion>();
    for (auto entity : animated_cats) {
        Motion& motion = registry.get<Motion>(entity);
        Animation& animation = registry.get<Animation>(entity);

        vec2 velocity = motion.velocity;
        float angle = atan2(velocity.y, velocity.x);
        if (abs(velocity.x) > abs(velocity.y)) {
            if (velocity.x > 0) {
                animation.start_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_1_EAST;
                animation.end_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_3_EAST;
            } else {
                animation.start_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_1_WEST;
                animation.end_ind = (int)TEXTURE_ASSET_ID::PATROL_CAT_3_WEST;
            }
        }
    }
}
