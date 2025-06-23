#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <optional>
#include "gl3w.h"

#include "map_system.hpp"
#include "util/world_grid.hpp"
#include "a_star.hpp"

// void MapSystem::init() {
//     // loadLevel(current_level);
// }

int MapSystem::loadLevel(int level_index) {
    std::tuple<std::string, int> level_and_portal = levels[level_index];
    std::string level_path_string = std::get<0>(level_and_portal);
    int portal_count = std::get<1>(level_and_portal);
    std::string level_path = map_path(level_path_string);

    std::ifstream file(level_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << level_path << std::endl;
        return 0;
    }
    std::string line;
    int row = 0;

    // clear patrol cat path end positions
    patrol_positions.clear();
    tile_map.clear();

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        int col = 0;

        while (getline(ss, cell, ',') && col < WINDOW_WIDTH_TILES) {
            if (!cell.empty()) {
                current_map[row][col] = cell; 

                // Check if the cell contains a patrol 
                if (cell[0] == 'X' && cell.size() > 1 && isdigit(cell[1])) {
                    current_map[row][col] = cell[1];
                    int patrol_id = cell[1] - '0';
                    patrol_positions[patrol_id].push_back(vec2(col, row));
                }
            }
            col++;
        }
        row++;
        if (row >= WINDOW_HEIGHT_TILES) break; // Stop if we exceed the map height
    }

    file.close();

    return portal_count;
    // mapDebugPrint();
}

std::string MapSystem::getLevelText(int level_index) {
    if (level_index >= level_texts.size()) {
        return "";
    } else {
        return level_texts[level_index];
    }
}

entt::entity MapSystem::createLevel(RenderSystem* renderer) {

    // player created from map but needs to be passed to world_system
    entt::entity player_entity;

    for (int row = 0; row < WINDOW_HEIGHT_TILES; row++) {
        std::vector<Tile> rowTiles;
        for (int col = 0; col < WINDOW_WIDTH_TILES; col++) {
            Tile tile;
            tile.walkable = true;

            std::string cell = current_map[row][col];

            // WorldGrid::createFloorAtGridPos(vec2(col,row));
            if (cell == "#") {
                // Render north, east, west wall
                tile.walkable = false;
                WorldGrid::createNWEWallAtGridPos(renderer, vec2(col,row));
            } else if (cell == "$") {
                // Render south wall
                tile.walkable = false;
                WorldGrid::createSouthWallAtGridPos(renderer, vec2(col,row));
            } else if (cell == "P") {
                // Render player
                tile.walkable = false;
                player_entity = WorldGrid::createPlayerAtGridPos(renderer,vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell[0] == 'S') {
                // Render sniper cat facing the specified direction
                if (cell[1] == 'N') {
                    WorldGrid::createSniperAtGridPos(renderer, vec2(col,row), Direction::TOP);
                } else if (cell[1] == 'E') {
                    WorldGrid::createSniperAtGridPos(renderer, vec2(col,row), Direction::RIGHT);
                } else if (cell[1] == 'S') {
                    WorldGrid::createSniperAtGridPos(renderer, vec2(col,row), Direction::BOTTOM);
                } else if (cell[1] == 'W') {
                    WorldGrid::createSniperAtGridPos(renderer, vec2(col,row), Direction::LEFT);
                }
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell == "K") {
                // Render key
                WorldGrid::createKeyAtGridPos(renderer, vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            }
            else if (cell == "I") {
                // Render ice
                WorldGrid::createFloorIceAtGridPos(renderer, vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell == "D") {
                // Render door
                WorldGrid::createDoorAtGridPos(renderer, vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell == "E") {
                // Render exit
                WorldGrid::createExitAtGridPos(renderer, vec2(col,row));
            } else if (cell == "C") {
                // Render cheese
                WorldGrid::createCheeseAtGridPos(renderer, vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell == "T") {
                WorldGrid::createMousetrapAtGridPos(renderer, vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell == "B") {
                // Render Boomerang
                WorldGrid::createBoomerangAtGridPos(renderer, vec2(col,row));
                WorldGrid::createFloorAtGridPos(vec2(col,row));
            } else if (cell == ".") {
                // Render floor
                WorldGrid::createFloorAtGridPos(vec2(col,row));
                // WorldGrid::createBlankAtGridPos(vec2(col,row));
            }
            rowTiles.push_back(tile);
        }
        tile_map.push_back(rowTiles);
    }

    for (auto& [patrol_id, positions] : patrol_positions) {
        if (!positions.empty()) {
            std::vector<ivec2> path;
            std::unordered_set<ivec2, PairHash> visited;
            
            // create path using A* algorithm
            StarNode startNode(positions[0].x, positions[0].y, nullptr, 0, 0);
            StarNode goalNode(positions[1].x, positions[1].y, nullptr, 0, 0);
            
            aStar(path, visited, &startNode, &goalNode, tile_map);

            // convert ivec2 to vec2
            std::vector<glm::vec2> floatPath;
            for (const auto& pos : path) {
                floatPath.push_back(glm::vec2(pos));
            }

            WorldGrid::createPatrolEnemyAtGridPos(renderer, floatPath);
            WorldGrid::createFloorAtGridPos(positions[0]);
            WorldGrid::createFloorAtGridPos(positions[1]);
        }
    }

    return player_entity;
}

void MapSystem::mapDebugPrint() {
    for (int row = 0; row < WINDOW_HEIGHT_TILES; row++) {
        for (int col = 0; col < WINDOW_WIDTH_TILES; col++) {
            std::cout << current_map[row][col] << " ";
        }
        std::cout << std::endl;
    }
}