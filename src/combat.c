#include "combat.h"
#include "game.h"
#include "hero.h"
#include "map.h"
#include "effects.h"
#include "randomizer.h"
#include "monster_data.h"
#include "spells.h"
#include "item_data.h"
#include "interactables.h"
#include "input.h"
#include "ui.h"
#include "gfx/gfx.h"
#include <stdio.h>
#include <stdlib.h>

CombatPhase combatPhase = PHASE_ENCOUNTER;
EntityStats enemyStats;
const MonsterDef *currentMonsterDef = NULL;
int lastHeroDmg = 0;
int lastEnemyDmg = 0;
bool isScriptedCombat = false;
static uint8_t combatMenuIndex = 0;

void triggerCombat(void) {
    state.currentState = STATE_COMBAT;
    
    int px = state.playerX >> 4;
    int py = state.playerY >> 4;
    
    uint8_t setId = current_map_global_monster_set;
    uint8_t *z = current_map_monster_zones;
    for (int i = 0; i < current_map_num_monster_zones; i++, z += 5) {
        uint8_t zx = z[0];
        uint8_t zy = z[1];
        uint8_t zw = z[2];
        uint8_t zh = z[3];
        uint8_t zset = z[4];
        
        if (px >= zx && px <= zx + zw && py >= zy && py <= zy + zh) {
            setId = zset;
            break;
        }
    }
    
    if (setId == 255) {
        state.currentState = STATE_EXPLORING;
        return; // No encounters here
    }
    
    // Count valid monsters in this set
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (get_zone_monster(setId, i) != 255) count++;
        else break;
    }
    
    if (count == 0) {
        state.currentState = STATE_EXPLORING;
        return;
    }
    
    state.currentMonster = get_zone_monster(setId, rand() % count);
    
    const MonsterDef *def = get_monster_def(state.currentMonster);
    currentMonsterDef = def;
    
    if (state.repelSteps > 0 && current_map_is_outside) {
        if ((getHeroDefense() / 2) > (def->strength / 2)) {
            state.currentState = STATE_EXPLORING;
            return;
        }
    }
    
    // Calculate random HP between hp_min and hp_max
    int range = def->hp_max - def->hp_min;
    if (range < 0) range = 0;
    enemyStats.hp = def->hp_min + (range > 0 ? (rand() % (range + 1)) : 0);
    enemyStats.maxHp = enemyStats.hp;
    
    enemyStats.strength = def->strength;
    enemyStats.defense = def->defense;
    enemyStats.agility = def->agility;
    enemyStats.xp = def->xp;
    enemyStats.gold = def->gp;
    enemyStats.isAsleep = false;
    enemyStats.turnsAsleep = 0;
    enemyStats.isSpellsBlocked = false;
    
    combatPhase = PHASE_ENCOUNTER;
    lastHeroDmg = 0;
    lastEnemyDmg = 0;
    sprintf(state.genericMsg, "A %s appears! [2ND]", def->name);
    
    fadeToBlack();
}

void triggerScriptedCombat(uint8_t monster_id, uint16_t victory_action_id) {
    state.currentState = STATE_COMBAT;
    isScriptedCombat = true;
    
    state.currentMonster = monster_id;
    state.combatVictoryActionId = victory_action_id;
    
    const MonsterDef *def = get_monster_def(state.currentMonster);
    currentMonsterDef = def;
    
    int range = def->hp_max - def->hp_min;
    if (range < 0) range = 0;
    enemyStats.hp = def->hp_min + (range > 0 ? (rand() % (range + 1)) : 0);
    enemyStats.maxHp = enemyStats.hp;
    
    enemyStats.strength = def->strength;
    enemyStats.defense = def->defense;
    enemyStats.agility = def->agility;
    enemyStats.xp = def->xp;
    enemyStats.gold = def->gp;
    enemyStats.isAsleep = false;
    enemyStats.turnsAsleep = 0;
    enemyStats.isSpellsBlocked = false;
    
    combatPhase = PHASE_ENCOUNTER;
    lastHeroDmg = 0;
    lastEnemyDmg = 0;
    
    fadeToBlack();
}

void updateCombat(void) {
    if (state.currentState == STATE_COMBAT) {
        switch (combatPhase) {
            case PHASE_ENCOUNTER:
                if (isScriptedCombat || BTN_CONFIRM) {
                    combatPhase = PHASE_COMMAND;
                    isScriptedCombat = false;
                }
                break;
                
            case PHASE_COMMAND:
                if (BTN_UP) combatMenuIndex = (combatMenuIndex > 0) ? combatMenuIndex - 1 : 3;
                if (BTN_DOWN) combatMenuIndex = (combatMenuIndex < 3) ? combatMenuIndex + 1 : 0;
                
                if (BTN_CONFIRM) {
                    if (combatMenuIndex == 0) { // FIGHT
                        if (heroStats.isAsleep) {
                            combatPhase = PHASE_PLAYER_ATK;
                            sprintf(state.genericMsg, "Hero is asleep! [2ND]");
                        } else {
                            uint8_t atk = getHeroAttack();
                            int min_d = (atk - (enemyStats.defense / 2)) / 4;
                            int max_d = (atk - (enemyStats.defense / 2)) / 2;
                            if (min_d <= 0) { min_d = 0; max_d = 1; }
                            if (heroStats.level >= 20 || atk > enemyStats.defense) {
                                min_d = atk / 2; max_d = atk;
                            }
                            bool crit = false;
                            const MonsterDef *def = currentMonsterDef;
                            if (def->allowsCriticalHits && (rand() % 32 == 0)) {
                                min_d = getHeroAttack() / 2; max_d = getHeroAttack();
                                crit = true;
                            }
                            
                            if (max_d < min_d) max_d = min_d;
                            lastHeroDmg = min_d;
                            if (max_d > min_d) lastHeroDmg += rand() % (max_d - min_d + 1);
                            if (lastHeroDmg < 1) lastHeroDmg = rand() % 2;
                            
                            enemyStats.hp -= (lastHeroDmg > enemyStats.hp) ? enemyStats.hp : lastHeroDmg;
                            if (crit) sprintf(state.genericMsg, "Excellent move! %d damage! [2ND]", lastHeroDmg);
                            else sprintf(state.genericMsg, "Hero attacks! %d damage! [2ND]", lastHeroDmg);
                            combatPhase = PHASE_PLAYER_ATK;
                        }
                    } else if (combatMenuIndex == 1) { // SPELL
                        extern uint8_t spellMenuIndex;
                        extern uint8_t spellScrollOffset;
                        spellMenuIndex = 0; spellScrollOffset = 0;
                        state.currentState = STATE_COMBAT_SPELLS;
                    } else if (combatMenuIndex == 2) { // ITEM
                        extern uint8_t inventoryMenuIndex;
                        extern uint8_t inventoryScrollOffset;
                        inventoryMenuIndex = 0; inventoryScrollOffset = 0;
                        state.currentState = STATE_COMBAT_ITEM;
                    } else if (combatMenuIndex == 3) { // RUN
                        bool runSuccess = false;
                        if (is_flag_active(RND_FLAG_QOL_ALWAYS_RUN)) {
                            runSuccess = true;
                        } else {
                            int heroAgil = heroStats.agility;
                            int enemyAgil = enemyStats.agility;
                            if (heroAgil * (rand() % 16) >= enemyAgil * (rand() % 16)) {
                                runSuccess = true;
                            }
                        }
                        
                        if (runSuccess) {
                            sprintf(state.genericMsg, "Thou hast run away! [2ND]");
                            combatPhase = PHASE_FLED;
                        } else {
                            sprintf(state.genericMsg, "Blocked! Cannot run! [2ND]");
                            combatPhase = PHASE_FLEE_FAIL;
                        }
                    }
                }
                break;
                
            case PHASE_PLAYER_ATK:
                if (BTN_CONFIRM) {
                    if (enemyStats.hp == 0) {
                        uint16_t xpReward = enemyStats.xp;
                        uint16_t goldReward = enemyStats.gold;
                        if (is_flag_active(RND_FLAG_QOL_FAST_EXP)) {
                            xpReward *= 2; goldReward *= 2;
                        }
                        heroStats.xp += xpReward;
                        heroStats.gold += goldReward;
                        sprintf(state.genericMsg, "Victory! Gained %d XP & %d Gold! [2ND]", xpReward, goldReward);
                        combatPhase = PHASE_VICTORY;
                    } else {
                        executeMonsterAction();
                        combatPhase = PHASE_ENEMY_ATK;
                    }
                }
                break;
                
            case PHASE_ENEMY_ATK:
                if (BTN_CONFIRM) {
                    if (heroStats.hp == 0) {
                        sprintf(state.genericMsg, "Thou hast perished... [2ND]");
                        combatPhase = PHASE_DEFEAT;
                    } else {
                        combatPhase = PHASE_COMMAND;
                    }
                }
                break;
                
            case PHASE_VICTORY:
                if (BTN_CONFIRM) {
                    state.currentState = STATE_EXPLORING;
                    checkLevelUp();
                    if (state.combatVictoryActionId != 0) {
                        uint16_t actId = state.combatVictoryActionId;
                        state.combatVictoryActionId = 0;
                        start_action(actId);
                    }
                }
                break;
                
            case PHASE_DEFEAT:
                if (BTN_CONFIRM) {
                    heroStats.gold /= 2;
                    initNewGame();
                }
                break;
                
            case PHASE_FLED:
                if (BTN_CONFIRM) state.currentState = STATE_EXPLORING;
                break;
                
            case PHASE_FLEE_FAIL:
                if (BTN_CONFIRM) {
                    executeMonsterAction();
                    combatPhase = PHASE_ENEMY_ATK;
                }
                break;

            default:
                break;
        }
    } else if (state.currentState == STATE_COMBAT_SPELLS) {
        SpellEnum known[16];
        int count = getKnownSpells(known);
        extern uint8_t spellMenuIndex;
        extern uint8_t spellScrollOffset;
        
        if (BTN_CANCEL) {
            state.currentState = STATE_COMBAT;
            combatPhase = PHASE_COMMAND;
        }
        if (count > 0) {
            if (BTN_UP) {
                if (spellMenuIndex > 0) {
                    spellMenuIndex--;
                    if (spellMenuIndex < spellScrollOffset) spellScrollOffset = spellMenuIndex;
                }
            }
            if (BTN_DOWN) {
                if (spellMenuIndex < count - 1) {
                    spellMenuIndex++;
                    if (spellMenuIndex >= spellScrollOffset + 8) spellScrollOffset = spellMenuIndex - 7;
                }
            }
            if (BTN_CONFIRM) {
                if (castHeroSpell(known[spellMenuIndex])) {
                    state.currentState = STATE_COMBAT;
                    combatPhase = PHASE_PLAYER_ATK;
                }
            }
        }
    } else if (state.currentState == STATE_COMBAT_ITEM) {
        extern uint8_t inventoryMenuIndex;
        extern uint8_t inventoryScrollOffset;
        if (BTN_CANCEL) {
            state.currentState = STATE_COMBAT;
            combatPhase = PHASE_COMMAND;
        }
        if (numInventoryItems > 0) {
            if (BTN_UP) {
                if (inventoryMenuIndex > 0) {
                    inventoryMenuIndex--;
                    if (inventoryMenuIndex < inventoryScrollOffset) inventoryScrollOffset = inventoryMenuIndex;
                }
            }
            if (BTN_DOWN) {
                if (inventoryMenuIndex < numInventoryItems - 1) {
                    inventoryMenuIndex++;
                    if (inventoryMenuIndex >= inventoryScrollOffset + 6) inventoryScrollOffset = inventoryMenuIndex - 5;
                }
            }
            if (BTN_CONFIRM) {
                ItemEnum selItem = currentInventoryList[inventoryMenuIndex];
                if (useItem(selItem)) {
                    state.currentState = STATE_COMBAT;
                    combatPhase = PHASE_PLAYER_ATK;
                }
            }
        }
    }
}

void renderCombat(void) {
    gfx_ZeroScreen();
    gfx_sprite_t *monSpr = (gfx_sprite_t*)monsterTable[state.currentMonster].sprite;
    if (monSpr) {
        gfx_TransparentSprite_NoClip(monSpr, 130, 40);
    }
    ui_DrawCombatScreen();
    ui_DrawCombatStats(heroStats.hp, heroStats.maxHp, heroStats.mp, heroStats.maxMp, enemyStats.hp);

    if (state.currentState == STATE_COMBAT) {
        if (combatPhase == PHASE_COMMAND) {
            ui_DrawCombatCommandMenu(combatMenuIndex);
        } else {
            ui_DrawCombatMessage(state.genericMsg);
        }
    } else if (state.currentState == STATE_COMBAT_SPELLS) {
        extern uint8_t spellMenuIndex;
        extern uint8_t spellScrollOffset;
        ui_DrawSpellMenu(heroStats.level, spellMenuIndex, spellScrollOffset);
    } else if (state.currentState == STATE_COMBAT_ITEM) {
        extern uint8_t inventoryMenuIndex;
        extern uint8_t inventoryScrollOffset;
        ui_DrawInventoryMenu(currentInventoryList, numInventoryItems, inventoryMenuIndex, inventoryScrollOffset, 0);
    }
}
