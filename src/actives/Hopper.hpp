#include "../BlockEntity.hpp"
#include "../MeshingEngine.hpp"
#include <iostream>

class Hopper : public BlockEntity {
public:
    void OnInteract(glm::ivec3 pos, MeshingEngine *engine) override {
        for (auto &stack : m_storage) {
            printf("x%d of Item #%d\n", stack.count, stack.id);
        }
    }

    void OnPlace(glm::ivec3 pos, MeshingEngine *engine) override {}
    void OnBreak() override {}

    void Tick(glm::ivec3 pos, MeshingEngine *engine) override {
        auto above = pos + glm::ivec3{0, 1, 0};
        auto b = engine->GetActive(above);
        if (b) {
            auto &storage = b->GetStorage();
            while (!storage.empty() && m_storage.size() < 3) {
                ItemStack &back = storage.back();
                m_storage.push_back(back);
                storage.pop_back();
            }
        }
    }


    std::vector<ItemStack> &GetStorage() override {
        return m_storage;
    }
private:
    std::vector<ItemStack> m_storage;
};