#ifndef LEVEL_DATA_H
#define LEVEL_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "spell_data.h"

#define NUM_LEVELS 40

typedef struct {
    uint24_t xpRequired;
    uint16_t strength;
    uint16_t agility;
    uint16_t maxHp;
    uint16_t maxMp;
    SpellEnum spellId;
} LevelDef;

extern const LevelDef levelTable[NUM_LEVELS];

#endif
