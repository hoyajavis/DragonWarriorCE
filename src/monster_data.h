#ifndef MONSTER_DATA_H
#define MONSTER_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include <graphx.h>
#include "spells.h"

typedef struct {
    char name[24];
    uint8_t hp_min;
    uint8_t hp_max;
    uint8_t strength;
    uint8_t defense;
    uint8_t agility;
    uint16_t xp;
    uint16_t gp;
    bool allowsCriticalHits;
    uint8_t sleepResist;
    uint8_t stopspellResist;
    uint8_t hurtResist;
    uint8_t blockFactor64;

    SpellEnum spellId1;
    uint8_t spellChance1;
    SpellEnum spellId2;
    uint8_t spellChance2;
    uint8_t healthThreshold;

    void* sprite;
} MonsterDef;

#define NUM_MONSTERS 41
extern const MonsterDef monsterTable[NUM_MONSTERS];

extern const uint8_t monsterSets[20][10];

#define MONSTER_STOPPER 0
#define MONSTER_SLIME 1
#define MONSTER_RED_SLIME 2
#define MONSTER_DRAKEE 3
#define MONSTER_GHOST 4
#define MONSTER_MAGICIAN 5
#define MONSTER_MAGIDRAKEE 6
#define MONSTER_SCORPION 7
#define MONSTER_DRUIN 8
#define MONSTER_POLTERGEIST 9
#define MONSTER_DROLL 10
#define MONSTER_DRAKEEMA 11
#define MONSTER_SKELETON 12
#define MONSTER_WARLOCK 13
#define MONSTER_METAL_SCORPION 14
#define MONSTER_WOLF 15
#define MONSTER_WRAITH 16
#define MONSTER_METAL_SLIME 17
#define MONSTER_SPECTER 18
#define MONSTER_WOLFLORD 19
#define MONSTER_DRUINLORD 20
#define MONSTER_DROLLMAGI 21
#define MONSTER_WYVERN 22
#define MONSTER_ROGUE_SCORPION 23
#define MONSTER_WRAITH_KNIGHT 24
#define MONSTER_GOLEM 25
#define MONSTER_GOLDMAN 26
#define MONSTER_KNIGHT 27
#define MONSTER_MAGIWYVERN 28
#define MONSTER_DEMON_KNIGHT 29
#define MONSTER_WEREWOLF 30
#define MONSTER_GREEN_DRAGON 31
#define MONSTER_STARWYVERN 32
#define MONSTER_WIZARD 33
#define MONSTER_AXE_KNIGHT 34
#define MONSTER_BLUE_DRAGON 35
#define MONSTER_STONEMAN 36
#define MONSTER_ARMORED_KNIGHT 37
#define MONSTER_RED_DRAGON 38
#define MONSTER_DRAGONLORD 39
#define MONSTER_DRAGONLORDS_TRUE_FORM 40

gfx_sprite_t *get_monster_sprite(uint8_t monster_id);

#endif // MONSTER_DATA_H
