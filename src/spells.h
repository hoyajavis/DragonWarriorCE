#ifndef SPELLS_H
#define SPELLS_H

#include <stdint.h>
#include <stdbool.h>
#include "spell_data.h"

// Casts a spell from the hero.
// Returns true if a turn should be consumed (i.e. successful or valid attempt).
// Returns false if the cast is invalid (e.g. not enough MP, wrong context).
bool castHeroSpell(SpellEnum spell);

// Performs the monster's chosen action (attack, spell, breath) against the hero.
void executeMonsterAction(void);

// Populates outSpells with known spells and returns the count
int getKnownSpells(SpellEnum* outSpells);

#endif
