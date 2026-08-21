#ifndef VOXL_BLOCKENTITY_HPP_
#define VOXL_BLOCKENTITY_HPP_

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Items.hpp"

class MeshingEngine;

class BlockEntity {
public:
    virtual ~BlockEntity() = default;

    virtual void Tick(glm::ivec3 pos, MeshingEngine *engine) = 0;
    
    virtual void OnPlace(glm::ivec3 pos, MeshingEngine *engine) = 0;
    virtual void OnBreak() = 0;
    virtual void OnInteract(glm::ivec3 pos, MeshingEngine *engine) = 0;

    virtual std::vector<ItemStack> &GetStorage() = 0;
};


#endif