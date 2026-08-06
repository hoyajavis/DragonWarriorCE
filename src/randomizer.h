#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include <stdint.h>
#include <stdbool.h>
#include "item_data.h"
#include "monster_data.h"
#include "level_data.h"

// Randomizer Bitmask Flags
#define RND_FLAG_SHUFFLE_ITEMS     (1 << 0)  // Key Item Shuffling
#define RND_FLAG_SHUFFLE_MONSTERS  (1 << 1)  // Monster Zone Randomization
#define RND_FLAG_SCALE_MONSTERS    (1 << 2)  // Monster Stat Variance (0.75x - 1.25x)
#define RND_FLAG_SHUFFLE_STATS     (1 << 3)  // Hero Level-up Stat Curves
#define RND_FLAG_SHUFFLE_SHOPS     (1 << 4)  // Shop Inventory & Price Randomization
#define RND_FLAG_QOL_FAST_EXP      (1 << 5)  // 2x EXP & Gold Multiplier
#define RND_FLAG_QOL_FAST_TEXT     (1 << 6)  // Instant Dialogue Rendering
#define RND_FLAG_QOL_LOWER_ENCNT   (1 << 7)  // 50% Encounter Rate
#define RND_FLAG_QOL_ALWAYS_RUN    (1 << 8)  // Guaranteed Flee/Run Success

#define is_flag_active(flag) ((state.randoFlags & (flag)) != 0)

// Presets
#define PRESET_STANDARD 0
#define PRESET_SPEEDRUN 1
#define PRESET_CHAOS    2
#define PRESET_VANILLA  3
#define PRESET_CUSTOM   4

// Overlay structure for mutated monster stats (~10 bytes per monster)
typedef struct {
    uint8_t hp_min;
    uint8_t hp_max;
    uint8_t strength;
    uint8_t defense;
    uint8_t agility;
    uint16_t xp;
    uint16_t gp;
} MonsterStatOverlay;

// Randomizer Module State
extern uint32_t active_seed;
extern uint16_t active_flags;

void seed_prng(uint32_t seed);
uint32_t next_prng(void);
uint32_t prng_range(uint32_t min, uint32_t max);

void init_randomizer(uint32_t seed, uint16_t flags);
uint16_t get_preset_flags(uint8_t preset);

// Data Accessors
ItemEnum get_item_mapping(ItemEnum original_item);
const MonsterDef* get_monster_def(uint8_t monster_id);
uint8_t get_zone_monster(uint8_t zone_id, uint8_t index);
const LevelDef* get_level_def(uint8_t level_index);
uint16_t get_item_price(ItemEnum item);

#endif // RANDOMIZER_H
