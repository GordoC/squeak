#pragma once

#include "common.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "render_system.hpp"
// const std::string map_path = "team-22/data/maps";

class MapSystem {

public:
    // current level index
    int current_level = 0;

    void init();
    
    int loadLevel(int level_index);

    std::string getLevelText(int level_index);

    entt::entity createLevel(RenderSystem* renderer);

    void mapDebugPrint();

    int getNumTutorialLevels() {
        return level_texts.size();
    }

    // current map parsed info
    std::array<std::array<std::string, WINDOW_WIDTH_TILES>, WINDOW_HEIGHT_TILES> current_map = {};

    std::vector<std::tuple<std::string, int>> levels = {
        {"1 - Basic Movement.csv", 0},
        {"2 - Mousetraps.csv", 2},
        {"3 - Doors and Keys.csv", 2},
        {"4 - Patrol Cats.csv", 2},
        {"5 - Sniper Cats.csv", 2},
        {"6 - Killing Cats.csv", 4},
        {"7 - Puzzle.csv", 2},
        // {"8- test.csv", 4},
        {"9 - test.csv", 4},
        {"Bombardilo crocodilo.csv", 0},
        {"Tripi Tropi.csv", 0},
        {"Tralalero Tralala.csv",2},
        {"Tum Tum Sahur.csv", 2},
        {"boss_map.csv", 6},
        {"test_map.csv", 4},
    };

private:
    

    std::vector<std::string> level_texts = {
        "WASD to move + R to restart level", 
        "LClick for portals + Space to teleport", 
        "Unlock doors by collecting keys first",
        "Avoid the patrol cats proximity",
        "Dont get shot by sniper cats",
        "Solve the puzzle"
    };

    // list of patrol positions
    std::unordered_map<int, std::vector<vec2>> patrol_positions;

    // map of tile entities
    std::vector<std::vector<Tile>> tile_map;
};