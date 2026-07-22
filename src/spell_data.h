#ifndef SPELL_DATA_H
#define SPELL_DATA_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_SPELLS 14

typedef enum {
    SPELL_NONE,
    SPELL_HEAL,
    SPELL_HURT,
    SPELL_SLEEP,
    SPELL_RADIANT,
    SPELL_STOPSPELL,
    SPELL_OUTSIDE,
    SPELL_RETURN,
    SPELL_REPEL,
    SPELL_HEALMORE,
    SPELL_HURTMORE,
    SPELL_HEALALL,
    SPELL_BREATH_FIRE,
    SPELL_BREATH_STRONG_FIRE,
} SpellEnum;

typedef struct {
    char name[12];
    uint8_t mp;
    bool availableInCombat;
    bool availableOutsideCombat;
} SpellDef;

extern const SpellDef spellTable[NUM_SPELLS];

#endif
