#include <cstdint>

enum class ItemID {
    EMPTY,
    STONE_DUST,
    COAL_DUST,
    IRON_DUST,
};

struct ItemStack {
    ItemID id;
    uint8_t count;
};