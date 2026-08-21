#include "../BlockEntity.hpp"
#include "../MeshingEngine.hpp"

class HandDrill : public BlockEntity {
public:
    void OnInteract(glm::ivec3 pos, MeshingEngine *engine) override {
        if (!m_enabled) return;
        if (m_durability == 1) return;
        if (!m_storage.empty()) return;
        m_durability--;

        m_count++;
        if (m_count >= 5) {
            m_count = 0;

            m_storage.resize(1);

            if (rand() % 100 < 20) {
                m_storage[0] = {ItemID::IRON_DUST, 2};
                printf("Got iron dust\n");
            } else if (rand() % 100 < 30) {
                m_storage[0] = {ItemID::COAL_DUST, 3};
                printf("Got coal dust\n");
            } else {
                m_storage[0] = {ItemID::STONE_DUST, 4};
                printf("Got stone dust\n");
            }
        }
    }

    void OnPlace(glm::ivec3 pos, MeshingEngine *engine) override {
        if (pos.y < 30) {
            m_enabled = true;
        }
    }
    void OnBreak() override {}
    void Tick(glm::ivec3 pos, MeshingEngine *engine) override {}
    std::vector<ItemStack> &GetStorage() override {
        return m_storage;
    }
private:
    uint8_t m_count = 0;
    uint8_t m_durability = 50;
    bool m_enabled = false;
    std::vector<ItemStack> m_storage;
};