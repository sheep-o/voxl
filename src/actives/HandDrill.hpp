#include "../BlockEntity.hpp"
#include "../MeshingEngine.hpp"
#include "../Items.hpp"

class HandDrill : public BlockEntity {
public:
    void OnInteract(glm::ivec3 pos, MeshingEngine *engine) override {
        if (!m_enabled) return;
        if (m_durability == 1) return;
        if (m_storage.count != 0) return;
        m_durability--;

        m_count++;
        if (m_count >= 5) {
            m_count = 0;

            /*
            engine->SetBlock(pos + glm::ivec3{0, 0, 1}, {});
            engine->Request(glm::ivec3{
                static_cast<int>(std::floor(static_cast<float>(pos.x) / CHUNK_WIDTH)),
                static_cast<int>(std::floor(static_cast<float>(pos.y) / CHUNK_HEIGHT)),
                static_cast<int>(std::floor(static_cast<float>(pos.z) / CHUNK_DEPTH))
            });
            */

            if (rand() % 100 < 20) {
                m_storage = {ItemID::IRON_DUST, 2};
            } else if (rand() % 100 < 30) {
                m_storage = {ItemID::COAL_DUST, 3};
            } else {
                m_storage = {ItemID::STONE_DUST, 4};
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
private:
    uint8_t m_count = 0;
    uint8_t m_durability = 50;
    bool m_enabled = false;
    ItemStack m_storage = {ItemID::EMPTY, 0};
};