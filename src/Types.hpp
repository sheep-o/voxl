#ifndef VOXL_TYPES_HPP_
#define VOXL_TYPES_HPP_

#include <unordered_map>
#include <queue>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_set>
#include "Chunk.hpp"
#include "BlockEntity.hpp"

struct ivec3Hash {
    std::size_t operator()(const glm::ivec3 &c) const {
        std::size_t h1 = std::hash<int>{}(c.x);
        std::size_t h2 = std::hash<int>{}(c.y);
        std::size_t h3 = std::hash<int>{}(c.z);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

using ChunkMap 
    = std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, ivec3Hash>;
using ChunkQueue
    = std::queue<std::shared_ptr<Chunk>>;
using BlockSet
    = std::unordered_set<glm::ivec3, ivec3Hash>;
using BlockMap
    = std::unordered_map<glm::ivec3, std::shared_ptr<BlockEntity>, ivec3Hash>;


#endif