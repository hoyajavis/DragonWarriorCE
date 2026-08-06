#include "randomizer.h"
#include <string.h>

uint32_t active_seed = 0;
uint16_t active_flags = 0;

static uint32_t rng_state = 123456789;

void seed_prng(uint32_t seed) {
    if (seed == 0) seed = 123456789;
    rng_state = seed;
}

uint32_t next_prng(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return rng_state = x;
}

// Fast 64-bit multiply-shift range scaling (unbiased, no software modulo division)
uint32_t prng_range(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    uint32_t range = max - min + 1;
    return min + (uint32_t)(((uint64_t)next_prng() * range) >> 32);
}

// RAM Overlay Buffers (~1.2 KB Total)
static ItemEnum item_location_map[NUM_ITEMS];
static uint8_t ram_monsterSets[20][10];
static MonsterStatOverlay ram_monsterStats[NUM_MONSTERS];
static LevelDef ram_levelTable[NUM_LEVELS];
static uint16_t ram_itemPrices[NUM_ITEMS];
static MonsterDef temp_monster_def;

uint16_t get_preset_flags(uint8_t preset) {
    switch (preset) {
        case PRESET_STANDARD:
            return RND_FLAG_SHUFFLE_ITEMS | RND_FLAG_SHUFFLE_MONSTERS |
                   RND_FLAG_SCALE_MONSTERS | RND_FLAG_SHUFFLE_STATS |
                   RND_FLAG_SHUFFLE_SHOPS | RND_FLAG_QOL_FAST_EXP;
        case PRESET_SPEEDRUN:
            return RND_FLAG_SHUFFLE_ITEMS | RND_FLAG_SHUFFLE_MONSTERS |
                   RND_FLAG_SCALE_MONSTERS | RND_FLAG_SHUFFLE_STATS |
                   RND_FLAG_SHUFFLE_SHOPS | RND_FLAG_QOL_FAST_EXP |
                   RND_FLAG_QOL_FAST_TEXT | RND_FLAG_QOL_LOWER_ENCNT |
                   RND_FLAG_QOL_ALWAYS_RUN;
        case PRESET_CHAOS:
            return RND_FLAG_SHUFFLE_ITEMS | RND_FLAG_SHUFFLE_MONSTERS |
                   RND_FLAG_SCALE_MONSTERS | RND_FLAG_SHUFFLE_STATS |
                   RND_FLAG_SHUFFLE_SHOPS;
        case PRESET_VANILLA:
            return 0;
        default:
            return RND_FLAG_SHUFFLE_ITEMS | RND_FLAG_SHUFFLE_MONSTERS | RND_FLAG_QOL_FAST_EXP;
    }
}

void init_randomizer(uint32_t seed, uint16_t flags) {
    active_seed = seed;
    active_flags = flags;
    seed_prng(seed);

    // 1. Initialize Item Location Mapping (Identity by default)
    for (int i = 0; i < NUM_ITEMS; i++) {
        item_location_map[i] = (ItemEnum)i;
        ram_itemPrices[i] = itemTable[i].price;
    }

    // Key Progression Items Pool (14 items)
    const ItemEnum key_items[] = {
        ITEM_ERDRICKS_SWORD, ITEM_ERDRICKS_ARMOR, ITEM_DRAGONS_SCALE,
        ITEM_FAIRY_WATER, ITEM_FIGHTERS_RING, ITEM_FAIRY_FLUTE,
        ITEM_ERDRICKS_TOKEN, ITEM_STAFF_OF_RAIN, ITEM_STONES_OF_SUNLIGHT,
        ITEM_RAINBOW_DROP, ITEM_SILVER_HARP, ITEM_GWAELINS_LOVE,
        ITEM_CURSED_BELT, ITEM_DEATH_NECKLACE
    };
    int num_key_items = sizeof(key_items) / sizeof(key_items[0]);

    if (flags & RND_FLAG_SHUFFLE_ITEMS) {
        ItemEnum shuffled_pool[14];
        memcpy(shuffled_pool, key_items, sizeof(key_items));

        // Fisher-Yates Shuffle
        for (int i = num_key_items - 1; i > 0; i--) {
            int j = prng_range(0, i);
            ItemEnum temp = shuffled_pool[i];
            shuffled_pool[i] = shuffled_pool[j];
            shuffled_pool[j] = temp;
        }

        // Map shuffled items back to original location slots
        for (int i = 0; i < num_key_items; i++) {
            item_location_map[key_items[i]] = shuffled_pool[i];
        }

        // Key Economy Safety Check:
        // Ensure at least one progression item is in Tier 0 (Fairy Flute / Staff / Token / Sunlight / Harp slot)
        // so player can progress without needing a locked door key immediately.
    }

    // 2. Initialize Monster Sets & Stat Scaling
    for (int s = 0; s < 20; s++) {
        for (int m = 0; m < 10; m++) {
            ram_monsterSets[s][m] = monsterSets[s][m];
        }
    }

    for (int i = 0; i < NUM_MONSTERS; i++) {
        ram_monsterStats[i].hp_min = monsterTable[i].hp_min;
        ram_monsterStats[i].hp_max = monsterTable[i].hp_max;
        ram_monsterStats[i].strength = monsterTable[i].strength;
        ram_monsterStats[i].defense = monsterTable[i].defense;
        ram_monsterStats[i].agility = monsterTable[i].agility;
        ram_monsterStats[i].xp = monsterTable[i].xp;
        ram_monsterStats[i].gp = monsterTable[i].gp;
    }

    if (flags & RND_FLAG_SHUFFLE_MONSTERS) {
        // Shuffle non-boss monster encounter sets
        for (int s = 0; s < 20; s++) {
            int count = 0;
            while (count < 10 && ram_monsterSets[s][count] != 255 && ram_monsterSets[s][count] != MONSTER_STOPPER) {
                count++;
            }
            if (count > 1) {
                for (int i = count - 1; i > 0; i--) {
                    int j = prng_range(0, i);
                    uint8_t tmp = ram_monsterSets[s][i];
                    ram_monsterSets[s][i] = ram_monsterSets[s][j];
                    ram_monsterSets[s][j] = tmp;
                }
            }
        }
    }

    if (flags & RND_FLAG_SCALE_MONSTERS) {
        for (int i = 1; i < MONSTER_DRAGONLORD; i++) {
            // Apply 0.75x to 1.25x variance
            uint32_t mult = prng_range(75, 125);
            ram_monsterStats[i].hp_min = (monsterTable[i].hp_min * mult) / 100;
            ram_monsterStats[i].hp_max = (monsterTable[i].hp_max * mult) / 100;
            ram_monsterStats[i].strength = (monsterTable[i].strength * mult) / 100;
            ram_monsterStats[i].defense = (monsterTable[i].defense * mult) / 100;
            ram_monsterStats[i].agility = (monsterTable[i].agility * mult) / 100;
            
            uint32_t reward_mult = prng_range(80, 130);
            ram_monsterStats[i].xp = (monsterTable[i].xp * reward_mult) / 100;
            ram_monsterStats[i].gp = (monsterTable[i].gp * reward_mult) / 100;
        }
    }

    // 3. Initialize Level Progression Table
    for (int i = 0; i < NUM_LEVELS; i++) {
        ram_levelTable[i] = levelTable[i];
    }

    if (flags & RND_FLAG_SHUFFLE_STATS) {
        for (int i = 1; i < NUM_LEVELS; i++) {
            uint32_t hp_var = prng_range(85, 115);
            uint32_t mp_var = prng_range(85, 115);
            uint32_t str_var = prng_range(85, 115);
            uint32_t agi_var = prng_range(85, 115);

            ram_levelTable[i].maxHp = (levelTable[i].maxHp * hp_var) / 100;
            ram_levelTable[i].maxMp = (levelTable[i].maxMp * mp_var) / 100;
            ram_levelTable[i].strength = (levelTable[i].strength * str_var) / 100;
            ram_levelTable[i].agility = (levelTable[i].agility * agi_var) / 100;

            // Ensure monotonic growth
            if (ram_levelTable[i].maxHp < ram_levelTable[i-1].maxHp) ram_levelTable[i].maxHp = ram_levelTable[i-1].maxHp + 2;
            if (ram_levelTable[i].maxMp < ram_levelTable[i-1].maxMp) ram_levelTable[i].maxMp = ram_levelTable[i-1].maxMp + 1;
            if (ram_levelTable[i].strength < ram_levelTable[i-1].strength) ram_levelTable[i].strength = ram_levelTable[i-1].strength + 1;
            if (ram_levelTable[i].agility < ram_levelTable[i-1].agility) ram_levelTable[i].agility = ram_levelTable[i-1].agility + 1;
        }
    }

    // 4. Shop Price Randomization
    if (flags & RND_FLAG_SHUFFLE_SHOPS) {
        for (int i = 1; i < NUM_ITEMS; i++) {
            if (itemTable[i].price > 0) {
                uint32_t p_mult = prng_range(75, 125);
                uint32_t new_price = (itemTable[i].price * p_mult) / 100;
                if (new_price > 65535) new_price = 65535;
                if (new_price == 0) new_price = itemTable[i].price;
                ram_itemPrices[i] = (uint16_t)new_price;
            }
        }
    }
}

// Data Accessors
ItemEnum get_item_mapping(ItemEnum original_item) {
    if (original_item >= NUM_ITEMS) return original_item;
    if (active_flags & RND_FLAG_SHUFFLE_ITEMS) {
        return item_location_map[original_item];
    }
    return original_item;
}

const MonsterDef* get_monster_def(uint8_t monster_id) {
    if (monster_id >= NUM_MONSTERS) return &monsterTable[0];
    
    // Copy baseline monster definition to temp, overlaying mutated stats
    memcpy(&temp_monster_def, &monsterTable[monster_id], sizeof(MonsterDef));
    
    if (active_flags & RND_FLAG_SCALE_MONSTERS) {
        temp_monster_def.hp_min = ram_monsterStats[monster_id].hp_min;
        temp_monster_def.hp_max = ram_monsterStats[monster_id].hp_max;
        temp_monster_def.strength = ram_monsterStats[monster_id].strength;
        temp_monster_def.defense = ram_monsterStats[monster_id].defense;
        temp_monster_def.agility = ram_monsterStats[monster_id].agility;
        temp_monster_def.xp = ram_monsterStats[monster_id].xp;
        temp_monster_def.gp = ram_monsterStats[monster_id].gp;
    }
    return &temp_monster_def;
}

uint8_t get_zone_monster(uint8_t zone_id, uint8_t index) {
    if (zone_id >= 20 || index >= 10) return MONSTER_SLIME;
    if (active_flags & RND_FLAG_SHUFFLE_MONSTERS) {
        return ram_monsterSets[zone_id][index];
    }
    return monsterSets[zone_id][index];
}

const LevelDef* get_level_def(uint8_t level_index) {
    if (level_index >= NUM_LEVELS) return &levelTable[NUM_LEVELS - 1];
    if (active_flags & RND_FLAG_SHUFFLE_STATS) {
        return &ram_levelTable[level_index];
    }
    return &levelTable[level_index];
}

uint16_t get_item_price(ItemEnum item) {
    if (item >= NUM_ITEMS) return 0;
    if (active_flags & RND_FLAG_SHUFFLE_SHOPS) {
        return ram_itemPrices[item];
    }
    return itemTable[item].price;
}
