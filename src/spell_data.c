#include "spell_data.h"

const SpellDef spellTable[NUM_SPELLS] = {
    { "None", 0, false, false },
    { "Heal", 4, true, true },
    { "Hurt", 2, true, false },
    { "Sleep", 2, true, false },
    { "Radiant", 3, false, true },
    { "Stopspell", 2, true, false },
    { "Outside", 6, false, true },
    { "Return", 8, false, true },
    { "Repel", 2, false, true },
    { "Healmore", 10, true, true },
    { "Hurtmore", 5, true, false },
    { "Healall", 20, true, true },
    { "BreathFire", 0, true, false },
    { "BreathSFire", 0, true, false },
};
