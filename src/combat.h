#ifndef COMBAT_H
#define COMBAT_H

#include <stdint.h>
#include <stdbool.h>
#include "game.h"
#include "monster_data.h"

typedef enum {
    PHASE_ENCOUNTER,
    PHASE_COMMAND,
    PHASE_PLAYER_ATK,
    PHASE_ENEMY_ATK,
    PHASE_VICTORY,
    PHASE_DEFEAT,
    PHASE_FLED,
    PHASE_FLEE_FAIL,
    PHASE_ENEMY_WAKES,
    PHASE_HERO_WAKES
} CombatPhase;

extern CombatPhase combatPhase;
extern EntityStats enemyStats;
extern const MonsterDef *currentMonsterDef;
extern int lastHeroDmg;
extern int lastEnemyDmg;
extern bool isScriptedCombat;

void triggerCombat(void);
void triggerScriptedCombat(uint8_t monster_id, uint16_t victory_action_id);
void updateCombat(void);
void renderCombat(void);

#endif // COMBAT_H
