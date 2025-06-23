#pragma once

#include "common.hpp"
#include "render_system.hpp"

class AISystem
{
    public:
        void step(RenderSystem* renderer, float elapsed_ms);

    private:
        // sniper cats
        void processSniperCats(RenderSystem* renderer, float elapsed_ms);
        bool is_tom_visible_to_sniper(entt::entity sniper_entity, entt::entity player_entity);
        bool is_blocking_view(int block_tile_x, int block_tile_y, int sniper_tile_x, int sniper_tile_y, int player_tile_x, int player_tile_y, Direction direction);
        // patrol cats
        void processPatrolCats(float elapsed_ms);
        bool is_blocked(vec2 start, vec2 end);
        void animateCats();
};