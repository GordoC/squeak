#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <optional>
#include "gl3w.h"

#include "tinyECS/components.hpp"
#include "render_system.hpp"

// f(n) = g(n) + h(n)
// g(n): cost from start to this node
// h(n): cost from this node to goal
struct StarNode {
    int x;
    int y;
    StarNode* parent;
    int fScore;
    int gScore;

    StarNode(int x, int y, StarNode* par, int f, int g)
        : x(x), y(y), parent(par), fScore(f), gScore(g) {}
};

// lower score has higher priority
struct StarNodeComparator {
    bool operator()(const StarNode* a, const StarNode* b) const {
        if (a->fScore == b->fScore) {
            return a->gScore > b->gScore;
        }
        return a->fScore > b->fScore;
    }
};

struct PairHash {
    std::size_t operator()(const ivec2& vec) const {
        return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
    }
};


int manhattanDistance(ivec2 a, ivec2 b);
std::optional<StarNode> findStarNodeByTextureType(std::map<TEXTURE_ASSET_ID, bool> textures);
void processDirection(
    const ivec2& direction,
    StarNode* curr,
    const StarNode* start,
    const StarNode* goal,
    const std::vector<std::vector<Tile>>& tile_map, 
    std::unordered_set<ivec2, PairHash>& visited,
    std::priority_queue<StarNode*, std::vector<StarNode*>, StarNodeComparator>& queue);
void aStar(std::vector<ivec2>& path, std::unordered_set<ivec2, PairHash>& visited, StarNode* start, StarNode* goal, const std::vector<std::vector<Tile>>& tile_map);