#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spells.h"
#include "game.h"
#include "map.h"
#include "level_data.h"

static inline int min(int a, int b) { return a < b ? a : b; }

int getKnownSpells(SpellEnum* outSpells) {
    uint8_t count = 0;
    uint8_t limit = heroStats.level;
    if (limit >= NUM_LEVELS) limit = NUM_LEVELS - 1;
    
    const LevelDef *lvl = &levelTable[1];
    for (uint8_t i = 1; i <= limit; i++, lvl++) {
        SpellEnum sid = lvl->spellId;
        if (sid != SPELL_NONE) {
            outSpells[count++] = sid;
        }
    }
    return count;
}

bool castHeroSpell(SpellEnum spell) {
    const SpellDef *def = &spellTable[spell];
    const MonsterDef *mDef = &monsterTable[state.currentMonster];
    
    // Validation
    if (state.currentState == STATE_EXPLORING && !def->availableOutsideCombat) {
        sprintf(state.genericMsg, "Thou cannot cast %s here! [2ND]", def->name);
        state.currentState = STATE_MESSAGE;
        return false;
    }
    
    // In combat, we might need a check if spells are blocked
    if (heroStats.isSpellsBlocked) {
        sprintf(state.genericMsg, "Thy magic is sealed! [2ND]");
        // Consumes turn
        return true;
    }
    
    if (heroStats.mp < def->mp) {
        sprintf(state.genericMsg, "Thou hast not enough magic! [2ND]");
        if (state.currentState == STATE_EXPLORING) {
            state.currentState = STATE_MESSAGE;
        }
        return false;
    }
    
    heroStats.mp -= def->mp;
    
    switch (spell) {
        case SPELL_HEAL: {
            int heal = 10 + (rand() % 6);
            heroStats.hp = min(heroStats.maxHp, heroStats.hp + heal);
            sprintf(state.genericMsg, "Thy wounds are healed by %d! [2ND]", heal);
            break;
        }
        case SPELL_HEALMORE: {
            int heal = 85 + (rand() % 16);
            heroStats.hp = min(heroStats.maxHp, heroStats.hp + heal);
            sprintf(state.genericMsg, "Thy wounds are fully restored! [2ND]");
            break;
        }
        case SPELL_HEALALL: {
            heroStats.hp = heroStats.maxHp;
            sprintf(state.genericMsg, "Thy wounds are fully restored! [2ND]");
            break;
        }
        case SPELL_HURT: {
            if (rand() % 100 < mDef->hurtResist) {
                sprintf(state.genericMsg, "The spell had no effect! [2ND]");
            } else {
                int dmg = 5 + (rand() % 8);
                enemyStats.hp -= min(enemyStats.hp, dmg);
                sprintf(state.genericMsg, "A fireball hits! %d dmg! [2ND]", dmg);
            }
            break;
        }
        case SPELL_HURTMORE: {
            if (rand() % 100 < mDef->hurtResist) {
                sprintf(state.genericMsg, "The spell had no effect! [2ND]");
            } else {
                int dmg = 58 + (rand() % 8);
                enemyStats.hp -= min(enemyStats.hp, dmg);
                sprintf(state.genericMsg, "A mighty blast hits! %d dmg! [2ND]", dmg);
            }
            break;
        }
        case SPELL_SLEEP: {
            if (rand() % 100 >= mDef->sleepResist) {
                enemyStats.isAsleep = true;
                enemyStats.turnsAsleep = 0;
                sprintf(state.genericMsg, "The monster falls asleep! [2ND]");
            } else {
                sprintf(state.genericMsg, "The spell had no effect! [2ND]");
            }
            break;
        }
        case SPELL_STOPSPELL: {
            if (rand() % 100 >= mDef->stopspellResist) {
                enemyStats.isSpellsBlocked = true;
                sprintf(state.genericMsg, "The monster's magic is sealed! [2ND]");
            } else {
                sprintf(state.genericMsg, "The spell had no effect! [2ND]");
            }
            break;
        }
        case SPELL_RADIANT: {
            state.lightDiameter = 7;
            state.lightDecaySteps = 70;
            state.lightDecayCounter = 70;
            sprintf(state.genericMsg, "A warm light surrounds thee. [2ND]");
            break;
        }
        case SPELL_REPEL: {
            state.repelSteps = 100;
            sprintf(state.genericMsg, "Monsters are repelled! [2ND]");
            break;
        }
        case SPELL_OUTSIDE: {
            if (current_map_is_outside) {
                sprintf(state.genericMsg, "But that spell will\nnot work. [2ND]");
            } else {
                strncpy(state.currentMapName, state.lastOutsideMap, 8);
                state.currentMapName[7] = '\0';
                state.playerX = state.lastOutsideX;
                state.playerY = state.lastOutsideY;
                map_Load(state.currentMapName);
                sprintf(state.genericMsg, "Hero was transported\noutside. [2ND]");
            }
            break;
        }
        case SPELL_RETURN: {
            if (!current_map_is_outside) {
                sprintf(state.genericMsg, "But that spell will\nnot work. [2ND]");
            } else {
                strncpy(state.currentMapName, "PYDW001", 8);
                state.currentMapName[7] = '\0';
                state.playerX = 43 * TILE_SIZE;
                state.playerY = 44 * TILE_SIZE;
                map_Load(state.currentMapName);
                sprintf(state.genericMsg, "Hero was transported\nto Tantegel Castle. [2ND]");
            }
            break;
        }
        default:
            sprintf(state.genericMsg, "%s was cast! [2ND]", def->name);
            break;
    }
    
    if (state.currentState == STATE_EXPLORING) {
        state.currentState = STATE_MESSAGE;
    }
    
    return true;
}

void executeMonsterAction(void) {
    const MonsterDef *def = &monsterTable[state.currentMonster];
    
    // Check if monster casts a spell
    SpellEnum castSpell = SPELL_NONE;
    int roll = rand() % 100;
    
    // Check health threshold for spellId1
    bool healthTrigger = true;
    if (def->healthThreshold > 0) {
        if ((enemyStats.hp * 100 / enemyStats.maxHp) > def->healthThreshold) {
            healthTrigger = false;
        }
    }
    
    if (healthTrigger && def->spellId1 != SPELL_NONE && roll < def->spellChance1) {
        castSpell = def->spellId1;
    } else if (def->spellId2 != SPELL_NONE && roll < (def->spellChance1 + def->spellChance2)) {
        castSpell = def->spellId2;
    }
    
    if (castSpell != SPELL_NONE && enemyStats.isSpellsBlocked && castSpell != SPELL_BREATH_FIRE && castSpell != SPELL_BREATH_STRONG_FIRE) {
        sprintf(state.genericMsg, "The monster's magic is sealed! [2ND]");
        return;
    }
    
    if (castSpell == SPELL_HURT) {
        sprintf(state.genericMsg, "%s casts Hurt! [2ND]", def->name);
        int dmg = 5 + (rand() % 8);
        if (state.equippedArmor == ITEM_MAGIC_ARMOR || state.equippedArmor == ITEM_ERDRICKS_ARMOR) {
            dmg = (dmg * 2) / 3;
        }
        heroStats.hp -= min(heroStats.hp, dmg);
    } else if (castSpell == SPELL_HURTMORE) {
        sprintf(state.genericMsg, "%s casts Hurtmore! [2ND]", def->name);
        int dmg = 58 + (rand() % 8);
        if (state.equippedArmor == ITEM_MAGIC_ARMOR || state.equippedArmor == ITEM_ERDRICKS_ARMOR) {
            dmg = (dmg * 2) / 3;
        }
        heroStats.hp -= min(heroStats.hp, dmg);
    } else if (castSpell == SPELL_HEAL || castSpell == SPELL_HEALMORE) {
        sprintf(state.genericMsg, "%s casts Heal! [2ND]", def->name);
        int heal = (castSpell == SPELL_HEAL) ? (10 + (rand() % 6)) : (85 + (rand() % 16));
        enemyStats.hp = min(enemyStats.maxHp, enemyStats.hp + heal);
    } else if (castSpell == SPELL_SLEEP) {
        sprintf(state.genericMsg, "%s casts Sleep! [2ND]", def->name);
        heroStats.isAsleep = true;
        heroStats.turnsAsleep = 0;
    } else if (castSpell == SPELL_STOPSPELL) {
        sprintf(state.genericMsg, "%s casts Stopspell! [2ND]", def->name);
        if (state.equippedArmor != ITEM_ERDRICKS_ARMOR) {
            heroStats.isSpellsBlocked = true;
        } else {
            // Effectively missing, but usually displays something else or just does nothing.
            // Let's just do nothing if blocked by Erdrick's Armor
        }
    } else if (castSpell == SPELL_BREATH_FIRE) {
        sprintf(state.genericMsg, "%s breathes fire! [2ND]", def->name);
        int dmg = 16 + (rand() % 8);
        if (state.equippedArmor == ITEM_MAGIC_ARMOR || state.equippedArmor == ITEM_ERDRICKS_ARMOR) {
            dmg = (dmg * 2) / 3;
        }
        heroStats.hp -= min(heroStats.hp, dmg);
    } else if (castSpell == SPELL_BREATH_STRONG_FIRE) {
        sprintf(state.genericMsg, "%s breathes intense fire! [2ND]", def->name);
        int dmg = 65 + (rand() % 8);
        if (state.equippedArmor == ITEM_MAGIC_ARMOR || state.equippedArmor == ITEM_ERDRICKS_ARMOR) {
            dmg = (dmg * 2) / 3;
        }
        heroStats.hp -= min(heroStats.hp, dmg);
    } else {
        // Physical Attack
        int min_d, max_d;
        uint16_t def_val = getHeroDefense();
        if (def_val < enemyStats.strength) {
            min_d = (enemyStats.strength - (def_val / 2)) / 4;
            max_d = (enemyStats.strength - (def_val / 2)) / 2;
        } else {
            min_d = 0;
            max_d = (enemyStats.strength + 4) / 6;
        }
        
        if (max_d < min_d) max_d = min_d;
        int dmg = min_d + (rand() % (max_d - min_d + 1));
        
        // Critical hits for monsters that allow them
        if (def->allowsCriticalHits && (rand() % 64) == 0) {
            dmg = enemyStats.strength;
            sprintf(state.genericMsg, "Excellent move! [2ND]"); // Could be handled by adding to a string
        }
        
        heroStats.hp -= min(heroStats.hp, dmg);
        sprintf(state.genericMsg, "%s attacks! %d damage! [2ND]", def->name, dmg);
    }
}
