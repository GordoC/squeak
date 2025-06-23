#include "../world_init.hpp"

class WorldGrid {
public:
    WorldGrid() = delete;

    static entt::entity createPlayerAtGridPos(RenderSystem *renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createPlayer(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createSniperAtGridPos(RenderSystem *renderer, vec2 grid_pos, Direction direction) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createSniperEnemy(renderer, vec2(x_pos, y_pos), direction);
    }

    static entt::entity createNWEWallAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createNWEWall(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createSouthWallAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createSouthWall(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createFloorAtGridPos(vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createFloorTile(vec2(x_pos, y_pos));
    }

    static entt::entity createFloorIceAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createFloorIce(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createBlankAtGridPos(vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createBlankTile(vec2(x_pos, y_pos));
    }

    static entt::entity createDoorAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createDoor(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createExitAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createExit(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createKeyAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createKey(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createCheeseAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createCheese(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createPatrolEnemyAtGridPos(RenderSystem* renderer, std::vector<vec2> positions) {
        std::vector<vec2> world_positions;

        // Convert positions to grid positions
        for (vec2 grid_pos : positions) {
            vec2 world_pos = vec2(
                GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX),
                GRID_CELL_HEIGHT_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX)
            );
            world_positions.push_back(world_pos);
        }

        return createPatrolEnemy(renderer, world_positions);
    }

    static entt::entity createMousetrapAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createMousetrap(renderer, vec2(x_pos, y_pos));
    }

    static entt::entity createBoomerangAtGridPos(RenderSystem* renderer, vec2 grid_pos) {
        int x_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[0] * GRID_CELL_WIDTH_PX);
        int y_pos = GRID_CELL_WIDTH_PX / 2 + (grid_pos[1] * GRID_CELL_HEIGHT_PX);
        return createBoomerang(renderer, vec2(x_pos, y_pos), BOOMERANG_SIZE, vec2(0,0), HARMFUL_DAMAGE, vec2(x_pos, y_pos), vec2(x_pos, y_pos),
            vec2(x_pos + (5 * GRID_CELL_WIDTH_PX), y_pos - (7 * GRID_CELL_HEIGHT_PX)), 
            vec2(x_pos - (5 * GRID_CELL_WIDTH_PX), y_pos - (7 * GRID_CELL_HEIGHT_PX))
            );
        //RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity, int damage, vec2 start, vec2 end, vec2 control_start, vec2 control_end
    }
};