#include <optional>
#include <unordered_set>
#include <cmath>
#include <iostream>

#include "a_star.hpp"
#include "tinyECS/registry.hpp"
#include "world_init.hpp"
#include "render_system.hpp"


int manhattanDistance(ivec2 a, ivec2 b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

void processDirection(
    const ivec2& direction,
    StarNode* curr,
    const StarNode* start,
    const StarNode* goal,
    const std::unordered_map<ivec2, StarNode*, PairHash>& validPositions,
    const std::vector<std::vector<Tile>>& tile_map, 
    std::unordered_set<ivec2, PairHash>& visited,
    std::priority_queue<StarNode*, std::vector<StarNode*>, StarNodeComparator>& queue)
{
    int x = direction.x;
    int y = direction.y;

    // check if within bounds
    if (x < 0 || y < 0 || x >= tile_map[0].size() || y >= tile_map.size())
        return;

    // skip if the tile is a wall/obsatcle
    if (!tile_map[y][x].walkable)
        return;

    // skip if already visited
    if (visited.find(direction) != visited.end())
        return;
    
    StarNode* node = validPositions.at(direction);

    int gScore = manhattanDistance({start->x, start->y}, direction);
    int hScore = manhattanDistance(direction, {goal->x, goal->y});
    int fScore = gScore + hScore;
    
    if (fScore >= node->fScore)
        return;

    node->fScore = fScore;
    node->gScore = gScore;
    node->parent = curr;
    queue.emplace(node);
}



void aStar(std::vector<ivec2>& path, std::unordered_set<ivec2, PairHash>& visited, StarNode* start, StarNode* goal, const std::vector<std::vector<Tile>>& tile_map) {
    std::unordered_map<ivec2, StarNode*, PairHash> validPositions;
    std::priority_queue<StarNode*, std::vector<StarNode*>, StarNodeComparator> queue;
    queue.emplace(start);

    // init valid positions
    for (int row = 0; row < WINDOW_HEIGHT_TILES; row++) {
        for (int col = 0; col < WINDOW_WIDTH_TILES; col++) {
            validPositions[{col, row}] = new StarNode(
                col,
                row,
                nullptr,
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max()
            );
        }
    }

    // process each valid positions
    while (!queue.empty()) {
        StarNode* curr = queue.top();
        queue.pop();

        if (visited.find({curr->x, curr->y}) != visited.end())
            continue;
        visited.insert({curr->x, curr->y});

        if (curr->x == goal->x && curr->y == goal->y) {
            StarNode* node = curr;
            
            while (node != nullptr) {
                path.emplace_back(ivec2(node->x, node->y));
                node = node->parent;
            }
            return;
        }
        
        processDirection({curr->x, curr->y - 1}, curr, start, goal, validPositions, tile_map, visited, queue);
        processDirection({curr->x + 1, curr->y}, curr, start, goal, validPositions, tile_map, visited, queue);
        processDirection({curr->x, curr->y + 1}, curr, start, goal, validPositions, tile_map, visited, queue);
        processDirection({curr->x - 1, curr->y}, curr, start, goal, validPositions, tile_map, visited, queue);
    }
}