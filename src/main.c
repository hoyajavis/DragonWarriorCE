#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

/* This includes the generated graphics headers */
#include "gfx/gfx.h"
#include "ui.h"
#include "map.h"
#include "game.h"
#include "level_data.h"
#include "save.h"
#include "action_ids.h"
#include "interactables.h"
#include "spells.h"
#include "monster_data.h"

// Standardized Keypad Macros
#define BTN_CONFIRM (((kb_Data[1] & kb_2nd) && !(prev_key1 & kb_2nd)) || ((kb_Data[6] & kb_Enter) && !(prev_key6 & kb_Enter)))
#define BTN_CANCEL (((kb_Data[2] & kb_Alpha) && !(prev_key2 & kb_Alpha)) || ((kb_Data[6] & kb_Clear) && !(prev_key6 & kb_Clear)))
#define BTN_UP ((kb_Data[7] & kb_Up) && !(prev_key7 & kb_Up))
#define BTN_DOWN ((kb_Data[7] & kb_Down) && !(prev_key7 & kb_Down))
#define BTN_LEFT ((kb_Data[7] & kb_Left) && !(prev_key7 & kb_Left))
#define BTN_RIGHT ((kb_Data[7] & kb_Right) && !(prev_key7 & kb_Right))
#define BTN_UP_HELD (kb_Data[7] & kb_Up)
#define BTN_DOWN_HELD (kb_Data[7] & kb_Down)
#define BTN_LEFT_HELD (kb_Data[7] & kb_Left)
#define BTN_RIGHT_HELD (kb_Data[7] & kb_Right)

#define FLAG_PM_CARRYING_PRINCESS 70

// Combat Phases
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

// Define globals
GameState state;
EntityStats heroStats;
EntityStats enemyStats;
static CombatPhase combatPhase;
static int lastHeroDmg;
static int lastEnemyDmg;

static uint8_t splashMenuIndex = 0;
static uint8_t commandMenuIndex = 0;
static uint8_t inventoryMenuIndex = 0;
static uint8_t inventoryTab = 0; // 0=ITEMS, 1=EQUIP, 2=KEY
static uint8_t inventoryScrollOffset = 0;
static uint8_t inventoryActionIndex = 0;
static ItemEnum currentInventoryList[NUM_ITEMS];
static uint8_t numInventoryItems = 0;
static uint8_t spellMenuIndex = 0;
static uint8_t spellScrollOffset = 0;
static uint8_t combatMenuIndex = 0;
static uint8_t dialogMenuIndex = 0;
static uint8_t vendorMenuIndex = 0;
static uint8_t vendorScrollOffset = 0;

static bool isMoving = false;
static uint8_t moveFrames = 0;
static int8_t moveStepX = 0;
static int8_t moveStepY = 0;
static bool needsFadeIn = false;
static bool isScriptedCombat = false;
static uint8_t inputDelay = 0;

void rebuildInventoryList(void) {
    numInventoryItems = 0;
    const ItemDef *item = &itemTable[1];
    for (uint8_t i = 1; i < NUM_ITEMS; i++, item++) {
        if (state.inventory[i] > 0) {
            ItemCategory t = item->type;
            if (inventoryTab == 0 && t == ITEM_TYPE_TOOL && !item->isKeyItem) {
                currentInventoryList[numInventoryItems++] = (ItemEnum)i;
            } else if (inventoryTab == 1 && (t == ITEM_TYPE_WEAPON || t == ITEM_TYPE_ARMOR || t == ITEM_TYPE_SHIELD)) {
                currentInventoryList[numInventoryItems++] = (ItemEnum)i;
            } else if (inventoryTab == 2 && item->isKeyItem) {
                currentInventoryList[numInventoryItems++] = (ItemEnum)i;
            }
        }
    }
}

// monsterTable moved to monster_data.c
static const MonsterDef *currentMonsterDef = NULL;

static uint16_t fade_palettes[9][256];

void initFadePalettes(void) {
    for (int step = 0; step <= 8; step++) {
        for (int i = 0; i < 256; i++) {
            uint16_t c = global_palette[i];
            uint8_t r = (c >> 10) & 31;
            uint8_t g = (c >> 5) & 31;
            uint8_t b = c & 31;
            r = (r * step) / 8;
            g = (g * step) / 8;
            b = (b * step) / 8;
            fade_palettes[step][i] = (r << 10) | (g << 5) | b;
        }
    }
}

void triggerCombat(void) {
    state.currentState = STATE_COMBAT;
    
    int px = state.playerX / TILE_SIZE;
    int py = state.playerY / TILE_SIZE;
    
    uint8_t setId = current_map_global_monster_set;
    for (int i = 0; i < current_map_num_monster_zones; i++) {
        uint8_t zx = current_map_monster_zones[i*5 + 0];
        uint8_t zy = current_map_monster_zones[i*5 + 1];
        uint8_t zw = current_map_monster_zones[i*5 + 2];
        uint8_t zh = current_map_monster_zones[i*5 + 3];
        uint8_t zset = current_map_monster_zones[i*5 + 4];
        
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
        if (monsterSets[setId][i] != 255) count++;
        else break;
    }
    
    if (count == 0) {
        state.currentState = STATE_EXPLORING;
        return;
    }
    
    state.currentMonster = monsterSets[setId][rand() % count];
    
    const MonsterDef *def = &monsterTable[state.currentMonster];
    currentMonsterDef = def;
    
    if (state.repelSteps > 0 && current_map_is_outside) {
        if ((heroStats.defense / 2) > (def->strength / 2)) {
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
    
    // Screen Transition: Fade to Black
    for (int step = 0; step <= 8; step++) {
        gfx_SetPalette(fade_palettes[8 - step], sizeof_global_palette, 0);
        delay(20); // Delay ~20ms per step
    }
    
    needsFadeIn = true;
}

void triggerScriptedCombat(uint8_t monster_id, uint16_t victory_action_id) {
    state.currentState = STATE_COMBAT;
    isScriptedCombat = true;
    
    state.currentMonster = monster_id;
    state.combatVictoryActionId = victory_action_id;
    
    const MonsterDef *def = &monsterTable[state.currentMonster];
    currentMonsterDef = def;
    
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
    
    // We do not set genericMsg here because the ApproachDialog has already set it,
    // and wait, PHASE_ENCOUNTER waits for BTN_CONFIRM. If we don't set genericMsg,
    // the text from ApproachDialog will still be there! Wait! 
    // The dialog VM uses `genericMsg` to store text and state.currentState is STATE_MESSAGE.
    // When we call `triggerScriptedCombat` inside `interactables.c`, `state.genericMsg` currently holds the last dialog line.
    // If the ApproachDialog said "The Dragonlord draws near!", `genericMsg` already has that.
    
    // Screen Transition: Fade to Black
    for (int step = 0; step <= 8; step++) {
        gfx_SetPalette(fade_palettes[8 - step], sizeof_global_palette, 0);
        delay(20);
    }
    
    needsFadeIn = true;
}

bool isPassable(uint16_t px, uint16_t py) {
    uint16_t tx = px / TILE_SIZE;
    uint16_t ty = py / TILE_SIZE;
    if (tx >= current_map_width || ty >= current_map_height) return false;
    
    // Check objects first
    uint8_t *obj = current_map_interactables_raw;
    for (uint8_t i = 0; i < current_map_num_interactables; i++, obj += 6) {
        uint16_t objX = obj[0];
        uint16_t objY = obj[1];
        if (objX == tx && objY == ty) {
            uint8_t objType = obj[2];
            // Can't walk on NPCs, Chests, or Doors
            if ((objType >= OBJ_NPC_KING && objType <= OBJ_NPC_TRUMPETER) || 
                objType == OBJ_CHEST || 
                objType == OBJ_DOOR) {
                return false;
            }
        }
    }
    
    uint8_t tile = current_map_data[ty * current_map_width + tx];
    return (tile != TILE_WATER && tile != TILE_MOUNTAIN && tile != TILE_STONE && tile != TILE_COUNTER && tile != TILE_ROOF && tile != TILE_INN && tile != TILE_ARMOR && tile != TILE_DARKNESS);
}

bool hasItem(ItemEnum item) {
    if (item >= NUM_ITEMS) return false;
    return state.inventory[item] > 0;
}

static uint16_t cachedHeroAttack = 0;
static uint16_t cachedHeroDefense = 0;
static bool statsDirty = true;

void markStatsDirty(void) {
    statsDirty = true;
}

static void updateCachedStats(void) {
    uint16_t totalAtk = heroStats.strength;
    if (state.equippedWeapon != ITEM_NONE) totalAtk += itemTable[state.equippedWeapon].attackBonus;
    if (state.equippedArmor != ITEM_NONE) totalAtk += itemTable[state.equippedArmor].attackBonus;
    if (state.equippedShield != ITEM_NONE) totalAtk += itemTable[state.equippedShield].attackBonus;
    if (state.equippedAccessory != ITEM_NONE) totalAtk += itemTable[state.equippedAccessory].attackBonus;
    cachedHeroAttack = totalAtk;

    uint16_t totalDef = heroStats.agility / 2;
    if (state.equippedWeapon != ITEM_NONE) totalDef += itemTable[state.equippedWeapon].defenseBonus;
    if (state.equippedArmor != ITEM_NONE) totalDef += itemTable[state.equippedArmor].defenseBonus;
    if (state.equippedShield != ITEM_NONE) totalDef += itemTable[state.equippedShield].defenseBonus;
    if (state.equippedAccessory != ITEM_NONE) totalDef += itemTable[state.equippedAccessory].defenseBonus;
    cachedHeroDefense = totalDef;

    statsDirty = false;
}

uint16_t getHeroAttack(void) {
    if (statsDirty) updateCachedStats();
    return cachedHeroAttack;
}

uint16_t getHeroDefense(void) {
    if (statsDirty) updateCachedStats();
    return cachedHeroDefense;
}

void giveItem(ItemEnum item) {
    if (item >= NUM_ITEMS) return;
    if (state.inventory[item] < 255) {
        state.inventory[item]++;
    }
}

void removeItem(ItemEnum item) {
    if (item >= NUM_ITEMS) return;
    if (state.inventory[item] > 0) {
        state.inventory[item]--;
        if (state.inventory[item] == 0) {
            if (state.equippedWeapon == item) state.equippedWeapon = ITEM_NONE;
            if (state.equippedArmor == item) state.equippedArmor = ITEM_NONE;
            if (state.equippedShield == item) state.equippedShield = ITEM_NONE;
            if (state.equippedAccessory == item) state.equippedAccessory = ITEM_NONE;
            markStatsDirty();
        }
    }
}

void checkLevelUp(void) {
    bool leveled = false;
    uint8_t oldLevel = heroStats.level;
    
    const LevelDef *lvl = levelTable;
    for (uint8_t i = 0; i < NUM_LEVELS; i++, lvl++) {
        if (heroStats.xp >= lvl->xpRequired) {
            heroStats.level = i + 1;
        } else {
            break;
        }
    }
    if (heroStats.level != oldLevel) {
        leveled = true;
    }
    if (leveled) {
        markStatsDirty();
        heroStats.maxHp = levelTable[heroStats.level - 1].maxHp;
        heroStats.maxMp = levelTable[heroStats.level - 1].maxMp;
        heroStats.strength = levelTable[heroStats.level - 1].strength;
        heroStats.agility = levelTable[heroStats.level - 1].agility;
        state.currentState = STATE_LEVEL_UP;
    }
}

// enemyTurn is now handled inline by the combat phase system

// enemyTurn is now handled inline by the combat phase system

void initNewGame(void) {
    strcpy(state.currentMapName, "PYDW037");
    state.playerX = 4 * TILE_SIZE;
    state.playerY = 5 * TILE_SIZE;
    state.currentState = STATE_EXPLORING;
    state.exitFlag = false;
    
    for (int i = 0; i < NUM_ITEMS; i++) state.inventory[i] = 0;
    state.equippedWeapon = ITEM_NONE;
    state.equippedArmor = ITEM_NONE;
    state.equippedShield = ITEM_NONE;
    markStatsDirty();
    
    giveItem(ITEM_HERB);
    giveItem(ITEM_HERB);
    
    heroStats.level = 1;
    heroStats.xp = 0;
    heroStats.hp = levelTable[0].maxHp;
    heroStats.maxHp = levelTable[0].maxHp;
    heroStats.mp = levelTable[0].maxMp;
    heroStats.maxMp = levelTable[0].maxMp;
    heroStats.strength = levelTable[0].strength;
    heroStats.agility = levelTable[0].agility;
}

uint8_t saveSlotIndex = 0;

int main(void) {
    kb_key_t prev_key1 = 0;
    kb_key_t prev_key2 = 0;
    kb_key_t prev_key6 = 0;
    kb_key_t prev_key7 = 0;
    
    gfx_Begin();
    gfx_SetDrawBuffer(); 
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    map_InitLUT();
    initFadePalettes();
    
    srand(rtc_Time());
    
    // Wait for all keys to be released before starting the game loop
    // This prevents the 'Enter' key used to launch the app from instantly starting a new game
    do {
        kb_Scan();
    } while (kb_AnyKey());
    
    // Set safe defaults for RETURN/OUTSIDE teleporting
    strcpy(state.lastOutsideMap, "PYDW001");
    state.lastOutsideX = 43 * TILE_SIZE;
    state.lastOutsideY = 44 * TILE_SIZE;

    state.currentState = STATE_SPLASH_MENU;
    state.exitFlag = false;
    splashMenuIndex = 0;

    while (!state.exitFlag) {
        kb_Scan(); 
        
        switch(state.currentState) {
            case STATE_GENERATING:
                // Generation is handled synchronously, so we shouldn't be in this state during the input loop
                break;
                
            case STATE_SPLASH_MENU:
                if (BTN_UP) {
                    splashMenuIndex = (splashMenuIndex > 0) ? splashMenuIndex - 1 : 3;
                }
                if (BTN_DOWN) {
                    splashMenuIndex = (splashMenuIndex < 3) ? splashMenuIndex + 1 : 0;
                }
                if BTN_CONFIRM {
                    if (splashMenuIndex == 0) {
                        if (!map_Load("PYDW037")) {
                            strcpy(state.genericMsg, "ERROR: Missing PYDW037.8xv!");
                            // Just draw it directly to the splash screen and stay in menu
                            gfx_SetTextFGColor(0xE0); // Red
                            gfx_SetTextBGColor(0x00);
                            gfx_PrintStringXY(state.genericMsg, 10, 10);
                            gfx_SwapDraw();
                            delay(2000);
                        } else {
                            state.currentState = STATE_EXPLORING;
                            initNewGame();
                            needsFadeIn = true;
                        }
                    } else if (splashMenuIndex == 1) {
                        state.currentState = STATE_SELECT_LOAD_SLOT;
                        saveSlotIndex = 0;
                    } else if (splashMenuIndex == 3) {
                        state.exitFlag = true;
                    }
                }
                break;
                
            case STATE_SELECT_LOAD_SLOT:
                if (BTN_UP) {
                    saveSlotIndex = (saveSlotIndex > 0) ? saveSlotIndex - 1 : 2;
                }
                if (BTN_DOWN) {
                    saveSlotIndex = (saveSlotIndex < 2) ? saveSlotIndex + 1 : 0;
                }
                if (BTN_CANCEL) {
                    state.currentState = STATE_SPLASH_MENU;
                }
                if (BTN_CONFIRM) {
                    if (save_Exists(saveSlotIndex)) {
                        load_Game(saveSlotIndex);
                        needsFadeIn = true;
                    }
                }
                break;
                
            case STATE_SELECT_SAVE_SLOT:
                if (BTN_UP) {
                    saveSlotIndex = (saveSlotIndex > 0) ? saveSlotIndex - 1 : 2;
                }
                if (BTN_DOWN) {
                    saveSlotIndex = (saveSlotIndex < 2) ? saveSlotIndex + 1 : 0;
                }
                if (BTN_CANCEL) {
                    state.currentState = STATE_EXPLORING;
                }
                if (BTN_CONFIRM) {
                    save_Game(saveSlotIndex);
                    state.currentState = STATE_EXPLORING;
                }
                break;
                
            case STATE_EXPLORING:
                {
                    if (!isMoving) {
                        if (inputDelay > 0) inputDelay--;
                        
                        if (inputDelay == 0) {
                            bool moved = false;
                            bool tryOutBounds = false;
                            if (BTN_LEFT_HELD) {
                                state.playerDirection = DIR_WEST;
                                if (state.playerX >= TILE_SIZE) {
                                    if (isPassable(state.playerX - 16, state.playerY)) {
                                        isMoving = true; moveStepX = -2; moveStepY = 0; moveFrames = 8; moved = true;
                                    }
                                } else tryOutBounds = true;
                            } else if (BTN_RIGHT_HELD) {
                                state.playerDirection = DIR_EAST;
                                if (state.playerX < (current_map_width * TILE_SIZE - TILE_SIZE)) {
                                    if (isPassable(state.playerX + 16, state.playerY)) {
                                        isMoving = true; moveStepX = 2; moveStepY = 0; moveFrames = 8; moved = true;
                                    }
                                } else tryOutBounds = true;
                            } else if (BTN_UP_HELD) {
                                state.playerDirection = DIR_NORTH;
                                if (state.playerY >= TILE_SIZE) {
                                    if (isPassable(state.playerX, state.playerY - 16)) {
                                        isMoving = true; moveStepX = 0; moveStepY = -2; moveFrames = 8; moved = true;
                                    }
                                } else tryOutBounds = true;
                            } else if (BTN_DOWN_HELD) {
                                state.playerDirection = DIR_SOUTH;
                                if (state.playerY < (current_map_height * TILE_SIZE - TILE_SIZE)) {
                                    if (isPassable(state.playerX, state.playerY + 16)) {
                                        isMoving = true; moveStepX = 0; moveStepY = 2; moveFrames = 8; moved = true;
                                    }
                                } else tryOutBounds = true;
                            }
                            
                            if (tryOutBounds && current_map_transitions != NULL) {
                                MapTransition *t = current_map_transitions;
                                for (uint8_t i = 0; i < current_map_num_transitions; i++, t++) {
                                    if (t->trigger_x == 255 && t->trigger_y == 255) {
                                        if (strncmp(t->target_map, "UNKNOWN", 7) == 0) break;
                                        
                                        char appvarName[9];
                                        memcpy(appvarName, t->target_map, 8);
                                        appvarName[8] = '\0';
                                        
                                        bool was_outside = current_map_is_outside;
                                        char old_map[9];
                                        strcpy(old_map, state.currentMapName);
                                        uint16_t old_x = state.playerX;
                                        uint16_t old_y = state.playerY;
                                        uint8_t sx = t->spawn_x;
                                        uint8_t sy = t->spawn_y;
                                        
                                        if (map_Load(appvarName)) {
                                            if (was_outside && !current_map_is_outside) {
                                                strcpy(state.lastOutsideMap, old_map);
                                                state.lastOutsideX = old_x;
                                                state.lastOutsideY = old_y;
                                            }
                                            strcpy(state.currentMapName, appvarName);
                                            state.playerX = sx * TILE_SIZE;
                                            state.playerY = sy * TILE_SIZE;
                                            needsFadeIn = true;
                                        }
                                        break;
                                    }
                                }
                            }
                            
                            if (moved) inputDelay = 5; // 5 frame pause between tiles
                        }
                        
                        if (!isMoving && BTN_CANCEL) {
                            state.currentState = STATE_MENU;
                        }
                        
                        if (!isMoving && BTN_CONFIRM) {
                            // Calculate tile in front of player
                            uint16_t targetX = state.playerX;
                            uint16_t targetY = state.playerY;
                            
                            if (state.playerDirection == DIR_NORTH) targetY -= TILE_SIZE;
                            else if (state.playerDirection == DIR_SOUTH) targetY += TILE_SIZE;
                            else if (state.playerDirection == DIR_EAST) targetX += TILE_SIZE;
                            else if (state.playerDirection == DIR_WEST) targetX -= TILE_SIZE;
                            
                            if (targetX < current_map_width * TILE_SIZE && targetY < current_map_height * TILE_SIZE) {
                                uint8_t targetTile = current_map_data[(targetY / TILE_SIZE) * current_map_width + (targetX / TILE_SIZE)];
                                if (targetTile == TILE_COUNTER) {
                                    if (state.playerDirection == DIR_NORTH) targetY -= TILE_SIZE;
                                    else if (state.playerDirection == DIR_SOUTH) targetY += TILE_SIZE;
                                    else if (state.playerDirection == DIR_EAST) targetX += TILE_SIZE;
                                    else if (state.playerDirection == DIR_WEST) targetX -= TILE_SIZE;
                                }
                            }
                            
                            bool found = false;
                            uint8_t *obj = current_map_interactables_raw;
                            for (uint8_t i = 0; i < current_map_num_interactables; i++, obj += 6) {
                                uint16_t objX = obj[0] * TILE_SIZE;
                                uint16_t objY = obj[1] * TILE_SIZE;
                                
                                if (targetX == objX && targetY == objY) {
                                    uint8_t objType = obj[2];
                                    uint16_t actionId = obj[4] | (obj[5] << 8);
                                    
                                    if (objType == OBJ_DOOR) {
                                        // Check if we have a key
                                        bool hasKey = hasItem(ITEM_KEY);
                                        if (hasKey) {
                                            removeItem(ITEM_KEY);
                                            // Remove object from array
                                            uint8_t remaining = current_map_num_interactables - 1 - i;
                                            if (remaining > 0) {
                                                memmove(obj, obj + 6, remaining * 6);
                                            }
                                            current_map_num_interactables--;
                                            if (actionId < 512) {
                                                state.event_flags[actionId / 8] |= (1 << (actionId % 8));
                                            }
                                            strcpy(state.genericMsg, "Unlocked the door.");
                                        } else {
                                            strcpy(state.genericMsg, "The door is locked.");
                                        }
                                        state.currentState = STATE_MESSAGE;
                                        found = true;
                                        break;
                                    } else if (objType == OBJ_CHEST) {
                                        start_action(actionId);
                                        
                                        // Remove object from array
                                        uint8_t remaining = current_map_num_interactables - 1 - i;
                                        if (remaining > 0) {
                                            memmove(obj, obj + 6, remaining * 6);
                                        }
                                        current_map_num_interactables--;
                                        if (actionId < 512) {
                                            state.event_flags[actionId / 8] |= (1 << (actionId % 8));
                                        }
                                        
                                        found = true;
                                        break;
                                    } else {
                                        // NPC dialogue execution
                                        start_action(actionId);
                                        found = true;
                                        break;
                                    }
                                }
                            }
                            if (!found) {
                                state.currentState = STATE_MENU;
                            }
                        }
                    }
                    
                    if (isMoving) {
                        state.playerX += moveStepX;
                        state.playerY += moveStepY;
                        moveFrames--;
                        
                        if (moveFrames == 0) {
                            isMoving = false;
                            
                            if (current_map_transitions != NULL) {
                                MapTransition *t = current_map_transitions;
                                for (uint8_t i = 0; i < current_map_num_transitions; i++, t++) {
                                    if ((state.playerX / TILE_SIZE) == t->trigger_x && (state.playerY / TILE_SIZE) == t->trigger_y) {
                                        if (strncmp(t->target_map, "UNKNOWN", 7) == 0) break;
                                        
                                        char appvarName[9];
                                        memcpy(appvarName, t->target_map, 8);
                                        appvarName[8] = '\0';
                                        
                                        bool was_outside = current_map_is_outside;
                                        char old_map[9];
                                        strcpy(old_map, state.currentMapName);
                                        uint16_t old_x = state.playerX;
                                        uint16_t old_y = state.playerY;
                                        uint8_t sx = t->spawn_x;
                                        uint8_t sy = t->spawn_y;
                                        
                                        if (map_Load(appvarName)) {
                                            if (was_outside && !current_map_is_outside) {
                                                strcpy(state.lastOutsideMap, old_map);
                                                state.lastOutsideX = old_x;
                                                state.lastOutsideY = old_y;
                                            }
                                            strcpy(state.currentMapName, appvarName);
                                            state.playerX = sx * TILE_SIZE;
                                            state.playerY = sy * TILE_SIZE;
                                        }
                                        break;
                                    }
                                }
                            }
                            
                            uint8_t currentTile = current_map_data[(state.playerY / TILE_SIZE) * current_map_width + (state.playerX / TILE_SIZE)];
                            
                            bool ignoreTileDamage = (state.equippedArmor == ITEM_ERDRICKS_ARMOR);
                            uint8_t hpRegenTiles = 0;
                            if (state.equippedArmor == ITEM_ERDRICKS_ARMOR) hpRegenTiles = 1;
                            else if (state.equippedArmor == ITEM_MAGIC_ARMOR) hpRegenTiles = 3;

                            if (hpRegenTiles > 0) {
                                state.hpRegenStepCounter++;
                                if (state.hpRegenStepCounter >= hpRegenTiles) {
                                    state.hpRegenStepCounter = 0;
                                    if (heroStats.hp < heroStats.maxHp) heroStats.hp++;
                                }
                            }

                            if (!ignoreTileDamage) {
                                if (currentTile == TILE_SWAMP) {
                                    if (heroStats.hp > 2) heroStats.hp -= 2; else heroStats.hp = 1;
                                } else if (currentTile == TILE_BARRIER) {
                                    if (heroStats.hp > 15) heroStats.hp -= 15; else heroStats.hp = 1;
                                }
                            }
                            
                            // Light Decay
                            if (state.lightDiameter > 0 && state.lightDecaySteps > 0) {
                                if (state.lightDecayCounter > 0) {
                                    state.lightDecayCounter--;
                                    if (state.lightDecayCounter == 0) {
                                        state.lightDiameter = (state.lightDiameter > 2) ? state.lightDiameter - 2 : 1;
                                        state.lightDecayCounter = state.lightDecaySteps;
                                    }
                                }
                            }

                            if (state.repelSteps > 0) {
                                state.repelSteps--;
                                if (state.repelSteps == 0) {
                                    sprintf(state.genericMsg, "The repel effect has worn off. [2ND]");
                                    state.currentState = STATE_MESSAGE;
                                }
                            }
                            
                            int chance = 0;
                            if (currentTile == TILE_PLAIN || currentTile == TILE_HILL) chance = 16;
                            else if (currentTile == TILE_FOREST || currentTile == TILE_SWAMP || currentTile == TILE_DESERT) chance = 8;
                            
                            if (chance > 0 && rand() % chance == 0) triggerCombat();
                        }
                    }
                }
                break;
                
            case STATE_MENU:
                if (BTN_CANCEL) {
                    state.currentState = STATE_EXPLORING;
                }
                if (BTN_UP) {
                    commandMenuIndex = (commandMenuIndex > 0) ? commandMenuIndex - 1 : 5;
                }
                if (BTN_DOWN) {
                    commandMenuIndex = (commandMenuIndex < 5) ? commandMenuIndex + 1 : 0;
                }
                if (BTN_CONFIRM) {
                    if (commandMenuIndex == 1) {
                        state.currentState = STATE_SPELLS;
                        spellMenuIndex = 0;
                        spellScrollOffset = 0;
                    }
                    else if (commandMenuIndex == 2) {
                        state.currentState = STATE_INVENTORY;
                        inventoryMenuIndex = 0;
                        inventoryScrollOffset = 0;
                        rebuildInventoryList();
                    }
                    else if (commandMenuIndex == 3) state.currentState = STATE_STATS;
                    else if (commandMenuIndex == 4) {
                        // SEARCH
                        uint16_t searchX = state.playerX;
                        uint16_t searchY = state.playerY;
                        bool found = false;
                        
                        uint8_t *obj = current_map_interactables_raw;
                        for (uint8_t i = 0; i < current_map_num_interactables; i++, obj += 6) {
                            uint16_t objX = obj[0] * TILE_SIZE;
                            uint16_t objY = obj[1] * TILE_SIZE;
                            
                            if (searchX == objX && searchY == objY) {
                                uint8_t objType = obj[2];
                                uint16_t actionId = obj[4] | (obj[5] << 8);
                                
                                if (objType == 0) { // MapDecoration type="none"
                                    start_action(actionId);
                                    extern uint24_t current_action_offset;
                                    if (current_action_offset != 0) {
                                        continue_action();
                                    }
                                    found = true;
                                    break;
                                }
                            }
                        }
                        
                        if (!found) {
                            strcpy(state.genericMsg, "Nothing found here.");
                            state.currentState = STATE_MESSAGE;
                        }
                    }
                    else if (commandMenuIndex == 5) {
                        state.currentState = STATE_SELECT_SAVE_SLOT;
                        saveSlotIndex = 0;
                    }
                }
                break;
                
            case STATE_INVENTORY:
                if (BTN_CANCEL) {
                    state.currentState = STATE_MENU;
                }
                if (BTN_LEFT) {
                    if (inventoryTab > 0) inventoryTab--;
                    else inventoryTab = 2;
                    inventoryMenuIndex = 0;
                    inventoryScrollOffset = 0;
                    rebuildInventoryList();
                }
                if (BTN_RIGHT) {
                    if (inventoryTab < 2) inventoryTab++;
                    else inventoryTab = 0;
                    inventoryMenuIndex = 0;
                    inventoryScrollOffset = 0;
                    rebuildInventoryList();
                }
                if (numInventoryItems > 0) {
                    if (BTN_UP) {
                        if (inventoryMenuIndex > 0) {
                            inventoryMenuIndex--;
                            if (inventoryMenuIndex < inventoryScrollOffset) {
                                inventoryScrollOffset = inventoryMenuIndex;
                            }
                        }
                    }
                    if (BTN_DOWN) {
                        if (inventoryMenuIndex < numInventoryItems - 1) {
                            inventoryMenuIndex++;
                            if (inventoryMenuIndex >= inventoryScrollOffset + 7) {
                                inventoryScrollOffset = inventoryMenuIndex - 6;
                            }
                        }
                    }
                    if (BTN_CONFIRM) {
                        state.currentState = STATE_INVENTORY_ACTION; // Need to define this in game.h
                        inventoryActionIndex = 0;
                    }
                }
                break;
                
            case STATE_INVENTORY_ACTION:
                if (BTN_CANCEL) {
                    state.currentState = STATE_INVENTORY;
                }
                if (BTN_UP) {
                    inventoryActionIndex = (inventoryActionIndex > 0) ? inventoryActionIndex - 1 : 2;
                }
                if (BTN_DOWN) {
                    inventoryActionIndex = (inventoryActionIndex < 2) ? inventoryActionIndex + 1 : 0;
                }
                if (BTN_CONFIRM) {
                    ItemEnum selectedItem = currentInventoryList[inventoryMenuIndex];
                    if (inventoryActionIndex == 0) { // USE
                        // Start action script if the item has one
                        uint16_t actionId = itemTable[selectedItem].useActionId;
                        if (actionId > 0) {
                            start_action(actionId);
                        } else {
                            strcpy(state.genericMsg, "Nothing happened.");
                            state.currentState = STATE_MESSAGE;
                        }
                    } else if (inventoryActionIndex == 1) { // EQUIP
                        if (itemTable[selectedItem].type == ITEM_TYPE_WEAPON) state.equippedWeapon = selectedItem;
                        else if (itemTable[selectedItem].type == ITEM_TYPE_ARMOR) state.equippedArmor = selectedItem;
                        else if (itemTable[selectedItem].type == ITEM_TYPE_SHIELD) state.equippedShield = selectedItem;
                        else if (itemTable[selectedItem].isEquippable) state.equippedAccessory = selectedItem;
                        else {
                            strcpy(state.genericMsg, "Cannot equip this.");
                            state.currentState = STATE_MESSAGE;
                        }
                        markStatsDirty();
                        if (state.currentState == STATE_INVENTORY_ACTION) {
                            state.currentState = STATE_INVENTORY; // go back if equipped
                        }
                    } else if (inventoryActionIndex == 2) { // DROP
                        state.currentState = STATE_INVENTORY_DROP_CONFIRM;
                    }
                }
                break;
                
            case STATE_INVENTORY_DROP_CONFIRM:
                if (BTN_CONFIRM) {
                    ItemEnum selectedItem = currentInventoryList[inventoryMenuIndex];
                    removeItem(selectedItem);
                    // Rebuild list
                    rebuildInventoryList();
                    if (inventoryMenuIndex >= numInventoryItems && numInventoryItems > 0) {
                        inventoryMenuIndex = numInventoryItems - 1;
                    }
                    if (inventoryScrollOffset > inventoryMenuIndex) inventoryScrollOffset = inventoryMenuIndex;
                    state.currentState = STATE_INVENTORY;
                } else if (BTN_CANCEL) {
                    state.currentState = STATE_INVENTORY_ACTION;
                }
                break;

            case STATE_SPELLS:
                {
                    SpellEnum known[16];
                    int count = getKnownSpells(known);
                    
                    if (BTN_CANCEL) {
                        state.currentState = STATE_MENU;
                    }
                    
                    if (count > 0) {
                        if (BTN_UP) {
                            if (spellMenuIndex > 0) {
                                spellMenuIndex--;
                                if (spellMenuIndex < spellScrollOffset) {
                                    spellScrollOffset = spellMenuIndex;
                                }
                            }
                        }
                        if (BTN_DOWN) {
                            if (spellMenuIndex < count - 1) {
                                spellMenuIndex++;
                                if (spellMenuIndex >= spellScrollOffset + 8) {
                                    spellScrollOffset = spellMenuIndex - 7;
                                }
                            }
                        }
                        if (BTN_CONFIRM) {
                            castHeroSpell(known[spellMenuIndex]);
                        }
                    }
                }
                break;
                
            case STATE_STATS:
                if (BTN_CANCEL) {
                    state.currentState = STATE_MENU;
                }
                break;

            case STATE_MESSAGE:
                if (BTN_CONFIRM) {
                    if (!continue_action()) {
                        state.currentState = STATE_EXPLORING;
                    }
                }
                break;
                
            case STATE_LEVEL_UP:
                if (BTN_CONFIRM) {
                    state.currentState = STATE_EXPLORING;
                    needsFadeIn = true;
                    gfx_SetPalette(fade_palettes[0], sizeof_global_palette, 0);
                }
                break;
                
            case STATE_DIALOG_MENU:
                if (BTN_UP) {
                    dialogMenuIndex = (dialogMenuIndex > 0) ? dialogMenuIndex - 1 : state.numMenuOptions - 1;
                }
                if (BTN_DOWN) {
                    dialogMenuIndex = (dialogMenuIndex < state.numMenuOptions - 1) ? dialogMenuIndex + 1 : 0;
                }
                if (BTN_CONFIRM) {
                    // Set current_action_offset to the selected branch and continue
                    extern uint24_t current_action_offset;
                    current_action_offset = state.menuOffsets[dialogMenuIndex];
                    if (!continue_action()) {
                        state.currentState = STATE_EXPLORING;
                    }
                }
                break;
                
            case STATE_VENDOR_BUY:
                if (BTN_UP) {
                    if (vendorMenuIndex > 0) {
                        vendorMenuIndex--;
                        if (vendorMenuIndex < vendorScrollOffset) {
                            vendorScrollOffset = vendorMenuIndex;
                        }
                    }
                }
                if (BTN_DOWN) {
                    if (vendorMenuIndex < state.vendorNumItems - 1) {
                        vendorMenuIndex++;
                        if (vendorMenuIndex >= vendorScrollOffset + 6) {
                            vendorScrollOffset = vendorMenuIndex - 5;
                        }
                    }
                }
                if (BTN_CANCEL) {
                    // Just return from vendor by continuing the action (which should go to end or next prompt)
                    if (!continue_action()) state.currentState = STATE_EXPLORING;
                }
                if (BTN_CONFIRM) {
                    ItemEnum selectedItem = state.vendorItemIds[vendorMenuIndex];
                    uint16_t price = itemTable[selectedItem].price;
                    if (heroStats.gold >= price) {
                        heroStats.gold -= price;
                        giveItem(selectedItem);
                    }
                    if (!continue_action()) state.currentState = STATE_EXPLORING;
                }
                break;

            case STATE_VENDOR_SELL:
                if (state.vendorNumItems == 0) {
                    // Nothing to sell
                    if (BTN_CONFIRM || BTN_CANCEL) {
                        if (!continue_action()) state.currentState = STATE_EXPLORING;
                    }
                } else {
                    if (BTN_UP) {
                        if (vendorMenuIndex > 0) {
                            vendorMenuIndex--;
                            if (vendorMenuIndex < vendorScrollOffset) {
                                vendorScrollOffset = vendorMenuIndex;
                            }
                        }
                    }
                    if (BTN_DOWN) {
                        if (vendorMenuIndex < state.vendorNumItems - 1) {
                            vendorMenuIndex++;
                            if (vendorMenuIndex >= vendorScrollOffset + 6) {
                                vendorScrollOffset = vendorMenuIndex - 5;
                            }
                        }
                    }
                    if (BTN_CANCEL) {
                        if (!continue_action()) state.currentState = STATE_EXPLORING;
                    }
                    if (BTN_CONFIRM) {
                        ItemEnum selectedItem = state.vendorItemIds[vendorMenuIndex];
                        removeItem(selectedItem);
                        heroStats.gold += itemTable[selectedItem].price / 2;
                        if (!continue_action()) state.currentState = STATE_EXPLORING;
                    }
                }
                break;
                
            case STATE_COMBAT:
                switch (combatPhase) {
                    case PHASE_ENCOUNTER:
                        // "A Slime appears! [2ND]" -> press Alpha to proceed
                        if (isScriptedCombat || BTN_CONFIRM) {
                            combatPhase = PHASE_COMMAND;
                            isScriptedCombat = false;
                            // genericMsg no longer needs to store the prompt since ui_DrawCombatCommandMenu handles it
                        }
                        break;
                        
                    case PHASE_COMMAND:
                        // Player chooses action
                        if (BTN_UP) {
                            combatMenuIndex = (combatMenuIndex > 0) ? combatMenuIndex - 1 : 3;
                        }
                        if (BTN_DOWN) {
                            combatMenuIndex = (combatMenuIndex < 3) ? combatMenuIndex + 1 : 0;
                        }
                        
                        if BTN_CONFIRM {
                            if (combatMenuIndex == 0) { // FIGHT
                                if (heroStats.isAsleep) {
                                    combatPhase = PHASE_PLAYER_ATK;
                                    sprintf(state.genericMsg, "Hero is asleep! [2ND]");
                                } else {
                                    uint8_t atk = getHeroAttack();
                                    int min_d = (atk - (enemyStats.defense / 2)) / 4;
                                    int max_d = (atk - (enemyStats.defense / 2)) / 2;
                                    if (min_d <= 0) {
                                        min_d = 0; max_d = 1;
                                    }
                                    if (heroStats.level >= 20 || atk > enemyStats.defense) {
                                        min_d = atk / 2;
                                        max_d = atk;
                                    }
                                    bool crit = false;
                                    
                                    const MonsterDef *def = currentMonsterDef;
                                    if (def->allowsCriticalHits && (rand() % 32 == 0)) {
                                        min_d = getHeroAttack() / 2;
                                        max_d = getHeroAttack();
                                        crit = true;
                                    }
                                    
                                    if (max_d < min_d) max_d = min_d;
                                    lastHeroDmg = min_d;
                                    if (max_d > min_d) lastHeroDmg += rand() % (max_d - min_d + 1);
                                    if (lastHeroDmg < 1) lastHeroDmg = rand() % 2;
                                    
                                    enemyStats.hp -= (lastHeroDmg > enemyStats.hp) ? enemyStats.hp : lastHeroDmg;
                                    
                                    if (crit) {
                                        sprintf(state.genericMsg, "Excellent move! %d damage! [2ND]", lastHeroDmg);
                                    } else {
                                        sprintf(state.genericMsg, "Hero attacks! %d damage! [2ND]", lastHeroDmg);
                                    }
                                    combatPhase = PHASE_PLAYER_ATK;
                                }
                            } else if (combatMenuIndex == 1) { // SPELL
                                state.currentState = STATE_COMBAT_SPELLS;
                                spellMenuIndex = 0; // reset spell cursor on entry
                            } else if (combatMenuIndex == 2) { // ITEM
                                state.currentState = STATE_COMBAT_ITEM;
                                inventoryTab = 0; // Tools only
                                rebuildInventoryList();
                                inventoryMenuIndex = 0;
                                inventoryScrollOffset = 0;
                            } else if (combatMenuIndex == 3) { // RUN
                                bool fleeSuccess = true;
                                if (!enemyStats.isAsleep) {
                                    uint24_t heroRoll = (uint24_t)heroStats.agility * (rand() % 256);
                                    uint24_t enemyRoll = ((uint24_t)enemyStats.agility * (rand() % 256) * currentMonsterDef->blockFactor64) / 64;
                                    if (heroRoll < enemyRoll) {
                                        fleeSuccess = false;
                                    }
                                }
                                
                                if (fleeSuccess) {
                                    combatPhase = PHASE_FLED;
                                    strcpy(state.genericMsg, "Escaped! [2ND]");
                                } else {
                                    combatPhase = PHASE_FLEE_FAIL;
                                    strcpy(state.genericMsg, "Can't escape! [2ND]");
                                }
                            }
                        }
                        break;
                        
                    case PHASE_PLAYER_ATK:
                        // "Hero attacks! X damage! [2ND]" -> press Alpha
                        if (BTN_CONFIRM) {
                            if (enemyStats.hp == 0) {
                                // Enemy died
                                heroStats.xp += enemyStats.xp;
                                heroStats.gold += enemyStats.gold;
                                if (heroStats.gold > 99999) heroStats.gold = 99999;
                                combatPhase = PHASE_VICTORY;
                                sprintf(state.genericMsg, "Won! +%d XP, +%d G! [2ND]", enemyStats.xp, (int)enemyStats.gold);
                            } else {
                                // Enemy attacks
                                if (enemyStats.isAsleep) {
                                    if (enemyStats.turnsAsleep == 0) {
                                        enemyStats.turnsAsleep = 1;
                                        combatPhase = PHASE_ENEMY_ATK;
                                        sprintf(state.genericMsg, "%s is asleep. [2ND]", currentMonsterDef->name);
                                    } else {
                                        enemyStats.turnsAsleep++;
                                        // 33% chance to wake up
                                        if (rand() % 100 < 33) {
                                            enemyStats.isAsleep = false;
                                            enemyStats.turnsAsleep = 0;
                                            combatPhase = PHASE_ENEMY_WAKES;
                                            sprintf(state.genericMsg, "%s wakes up! [2ND]", currentMonsterDef->name);
                                        } else {
                                            combatPhase = PHASE_ENEMY_ATK;
                                            sprintf(state.genericMsg, "%s is asleep. [2ND]", currentMonsterDef->name);
                                        }
                                    }
                                } else {
                                    executeMonsterAction();
                                    if (heroStats.hp > 0) {
                                        combatPhase = PHASE_ENEMY_ATK;
                                    } else {
                                        strcpy(state.genericMsg, "Thou art dead. [2ND]");
                                        combatMenuIndex = 0;
                                        combatPhase = PHASE_DEFEAT;
                                    }
                                }
                            }
                        }
                        break;
                        
                    case PHASE_ENEMY_ATK:
                        // "Slime attacks! X damage! [2ND]" -> back to command
                        if (BTN_CONFIRM) {
                            if (heroStats.isAsleep) {
                                if (heroStats.turnsAsleep == 0) {
                                    heroStats.turnsAsleep = 1;
                                    combatPhase = PHASE_PLAYER_ATK; // Skip command phase
                                    sprintf(state.genericMsg, "Hero is asleep! [2ND]");
                                } else {
                                    heroStats.turnsAsleep++;
                                    if (rand() % 100 < 50) {
                                        heroStats.isAsleep = false;
                                        heroStats.turnsAsleep = 0;
                                        combatPhase = PHASE_HERO_WAKES;
                                        sprintf(state.genericMsg, "Hero wakes up! [2ND]");
                                    } else {
                                        sprintf(state.genericMsg, "Hero is asleep! [2ND]");
                                        combatPhase = PHASE_PLAYER_ATK; // Skip command phase
                                    }
                                }
                            } else {
                                combatPhase = PHASE_COMMAND;
                                sprintf(state.genericMsg, "[1]FIGHT [2]SPELL [2ND]RUN");
                            }
                        }
                        break;
                        
                    case PHASE_VICTORY:
                        // "Monster defeated! +X XP [2ND]" -> return to map
                        if (BTN_CONFIRM) {
                            state.currentState = STATE_EXPLORING;
                            checkLevelUp();
                            if (state.currentState == STATE_EXPLORING) {
                                needsFadeIn = true;
                                gfx_SetPalette(fade_palettes[0], sizeof_global_palette, 0);
                            }
                            if (state.combatVictoryActionId > 0) {
                                start_action(state.combatVictoryActionId);
                                state.combatVictoryActionId = 0;
                            }
                        }
                        break;
                        
                    case PHASE_DEFEAT:
                        if (BTN_CONFIRM) {
                            if (combatMenuIndex == 0) {
                                strcpy(state.genericMsg, "I shall give thee\nanother chance. [2ND]");
                                combatMenuIndex++;
                            } else if (combatMenuIndex == 1) {
                                strcpy(state.genericMsg, "Thy gold is\nhalved! [2ND]");
                                combatMenuIndex++;
                            } else {
                                // Cut gold in half
                                heroStats.gold /= 2;
                                
                                // Restore 100% HP/MP
                                heroStats.hp = heroStats.maxHp;
                                heroStats.mp = heroStats.maxMp;
                                
                                // Teleport to Tantegel Castle King Lorik
                                strncpy(state.currentMapName, "PYDW036", 8);
                                state.currentMapName[7] = '\0';
                                
                                state.playerX = 12 * TILE_SIZE;
                                state.playerY = 8 * TILE_SIZE;
                                
                                map_Load(state.currentMapName);
                                state.currentState = STATE_EXPLORING;
                            }
                        }
                        break;
                        
                    case PHASE_FLED:
                        // "Escaped! [2ND]" -> return to map
                        if (BTN_CONFIRM) {
                            state.currentState = STATE_EXPLORING;
                            needsFadeIn = true;
                            gfx_SetPalette(fade_palettes[0], sizeof_global_palette, 0);
                        }
                        break;
                        
                    case PHASE_FLEE_FAIL:
                        // "Can't escape! [2ND]" -> enemy gets free hit
                        if (BTN_CONFIRM) {
                            executeMonsterAction();
                            if (heroStats.hp > 0) {
                                combatPhase = PHASE_ENEMY_ATK;
                            } else {
                                strcpy(state.genericMsg, "Thou art dead. [2ND]");
                                combatMenuIndex = 0;
                                combatPhase = PHASE_DEFEAT;
                            }
                        }
                        break;
                        
                    case PHASE_ENEMY_WAKES:
                        if (BTN_CONFIRM) {
                            executeMonsterAction();
                            if (heroStats.hp > 0) {
                                combatPhase = PHASE_ENEMY_ATK;
                            } else {
                                strcpy(state.genericMsg, "Thou art dead. [2ND]");
                                combatMenuIndex = 0;
                                combatPhase = PHASE_DEFEAT;
                            }
                        }
                        break;
                        
                    case PHASE_HERO_WAKES:
                        if (BTN_CONFIRM) {
                            combatPhase = PHASE_COMMAND;
                            sprintf(state.genericMsg, "[1]FIGHT [2]SPELL [2ND]RUN");
                        }
                        break;
                }
                break;
                
            case STATE_COMBAT_SPELLS:
                {
                    SpellEnum known[16];
                    int count = getKnownSpells(known);
                    
                    if (BTN_CANCEL) {
                        combatPhase = PHASE_COMMAND;
                        state.currentState = STATE_COMBAT;
                    }
                    
                    if (count > 0) {
                        if (BTN_UP) {
                            spellMenuIndex = (spellMenuIndex > 0) ? spellMenuIndex - 1 : count - 1;
                        }
                        if (BTN_DOWN) {
                            spellMenuIndex = (spellMenuIndex < count - 1) ? spellMenuIndex + 1 : 0;
                        }
                        if (BTN_CONFIRM) {
                            if (castHeroSpell(known[spellMenuIndex])) {
                                combatPhase = PHASE_PLAYER_ATK;
                            } else {
                                combatPhase = PHASE_COMMAND;
                            }
                            state.currentState = STATE_COMBAT;
                        }
                    }
                }
                break;
                
            case STATE_COMBAT_ITEM:
                {
                    if (BTN_CANCEL) {
                        combatPhase = PHASE_COMMAND;
                        state.currentState = STATE_COMBAT;
                    }
                    
                    if (numInventoryItems > 0) {
                        if (BTN_UP) {
                            if (inventoryMenuIndex > 0) {
                                inventoryMenuIndex--;
                                if (inventoryMenuIndex < inventoryScrollOffset) {
                                    inventoryScrollOffset = inventoryMenuIndex;
                                }
                            }
                        }
                        if (BTN_DOWN) {
                            if (inventoryMenuIndex < numInventoryItems - 1) {
                                inventoryMenuIndex++;
                                if (inventoryMenuIndex >= inventoryScrollOffset + 7) {
                                    inventoryScrollOffset = inventoryMenuIndex - 6;
                                }
                            }
                        }
                        
                        if (BTN_CONFIRM) {
                            ItemEnum selectedItem = currentInventoryList[inventoryMenuIndex];
                            
                            // EXPLANATION: We cannot safely use the dialogue VM (start_action) 
                            // to trigger item effects while inside the combat loop, because the 
                            // VM overrides state.currentState and enters its own message loops.
                            // Therefore, combat item usage is hardcoded here.
                            if (selectedItem == ITEM_HERB) {
                                removeItem(ITEM_HERB);
                                int heal = 20 + (rand() % 16);
                                heroStats.hp = (heroStats.hp + heal > heroStats.maxHp) ? heroStats.maxHp : heroStats.hp + heal;
                                sprintf(state.genericMsg, "Hero uses an Herb! Recovered %d HP! [2ND]", heal);
                                combatPhase = PHASE_PLAYER_ATK;
                                state.currentState = STATE_COMBAT;
                            } else if (selectedItem == ITEM_FAIRY_FLUTE) {
                                if (rand() % 100 >= currentMonsterDef->sleepResist) {
                                    enemyStats.isAsleep = true;
                                    enemyStats.turnsAsleep = 0;
                                    sprintf(state.genericMsg, "Hero plays the Fairy Flute...\nThe monster falls asleep! [2ND]");
                                } else {
                                    sprintf(state.genericMsg, "Hero plays the Fairy Flute...\nBut nothing happened. [2ND]");
                                }
                                combatPhase = PHASE_PLAYER_ATK;
                                state.currentState = STATE_COMBAT;
                            } else if (selectedItem == ITEM_FAIRY_WATER) {
                                sprintf(state.genericMsg, "Cannot use that here! [2ND]");
                            } else {
                                sprintf(state.genericMsg, "Cannot use that here! [2ND]");
                            }
                            
                            // Rebuild in case item was consumed
                            rebuildInventoryList();
                        }
                    }
                }
                break;
        }
        
        prev_key1 = kb_Data[1];
        prev_key2 = kb_Data[2];
        prev_key6 = kb_Data[6];
        prev_key7 = kb_Data[7];

        if (kb_Data[6] & kb_Clear) {
            state.exitFlag = true;
        }

        // 4. Graphics Render Phase
        if (state.currentState == STATE_SPLASH_MENU || state.currentState == STATE_SELECT_LOAD_SLOT) {
            gfx_ZeroScreen();
            gfx_TransparentSprite(title, 30, 20);
            
            if (state.currentState == STATE_SPLASH_MENU) {
                ui_DrawMenuWindow(20, 120, 280, 100);
                gfx_SetTextFGColor(0xFF);
                gfx_SetTextBGColor(0x00);
                gfx_SetTextTransparentColor(0x00);
                
                static const char * const options[] = {
                    "Start New Game",
                    "Load Saved Game",
                    "Settings",
                    "Exit to OS"
                };
                
                for (int i = 0; i < 4; i++) {
                    if (splashMenuIndex == i) {
                        ui_DrawSelectionPointer(30, 135 + i * 20);
                    }
                    gfx_PrintStringXY(options[i], 45, 135 + i * 20);
                }
            } else {
                ui_DrawSaveLoadMenu(saveSlotIndex, false);
            }
        } else {
            if (current_map_width * TILE_SIZE < 320 || current_map_height * TILE_SIZE < 240) {
                gfx_ZeroScreen(); // Clear screen to prevent title screen from bleeding through on small maps
            }
            
            // 1. Draw Overworld Background
            int24_t cameraX = (int24_t)state.playerX - 152; // 320/2 - 16/2
            int24_t cameraY = (int24_t)state.playerY - 112; // 240/2 - 16/2
            
            int24_t maxCamX = (int24_t)(current_map_width * TILE_SIZE) - 320;
            int24_t maxCamY = (int24_t)(current_map_height * TILE_SIZE) - 240;
            if (maxCamX < 0) maxCamX = 0; // Map is smaller than screen width
            if (maxCamY < 0) maxCamY = 0; // Map is smaller than screen height
            
            if (cameraX < 0) cameraX = 0;
            if (cameraY < 0) cameraY = 0;
            if (cameraX > maxCamX) cameraX = maxCamX;
            if (cameraY > maxCamY) cameraY = maxCamY;
            
            map_Draw((uint16_t)cameraX, (uint16_t)cameraY); 
            
            static gfx_sprite_t *obj_sprite_lut[14] = {NULL};
            if (!obj_sprite_lut[OBJ_NPC_KING]) { // Initialize once
                obj_sprite_lut[OBJ_NONE] = NULL;
                obj_sprite_lut[OBJ_NPC_KING] = king;
                obj_sprite_lut[OBJ_NPC_PRINCESS] = princess;
                obj_sprite_lut[OBJ_NPC_GUARD] = guard;
                obj_sprite_lut[OBJ_NPC_SAGE] = sage;
                obj_sprite_lut[OBJ_NPC_MERCHANT] = merchant;
                obj_sprite_lut[OBJ_NPC_WARRIOR] = warrior;
                obj_sprite_lut[OBJ_NPC_BOY] = boy;
                obj_sprite_lut[OBJ_NPC_GIRL] = girl;
                obj_sprite_lut[OBJ_NPC_TRUMPETER] = trumpeter;
                obj_sprite_lut[OBJ_CHEST] = chest;
                obj_sprite_lut[OBJ_DOOR] = door;
                obj_sprite_lut[OBJ_STAIRS_UP] = stairs_up;
                obj_sprite_lut[OBJ_STAIRS_DOWN] = stairs_down;
            }
            
            uint8_t cam_tx = cameraX / TILE_SIZE;
            uint8_t cam_ty = cameraY / TILE_SIZE;

            uint8_t *obj_data = current_map_interactables_raw;
            for (uint8_t i = 0; i < current_map_num_interactables; i++, obj_data += 6) {
                uint8_t obj_x = obj_data[0];
                uint8_t obj_y = obj_data[1];
                
                // Bounds Check: Only process objects roughly within the 21x16 tile camera view
                if (obj_x + 1 < cam_tx || obj_x > cam_tx + 21 || obj_y + 1 < cam_ty || obj_y > cam_ty + 16) {
                    continue;
                }
                
                uint8_t obj_type = obj_data[2];
                // uint8_t obj_dir = obj_data[3];
                // uint16_t obj_data_id = (obj_data[5] << 8) | obj_data[4];
                
                gfx_sprite_t *obj_sprite = NULL;
                if (obj_type < 14) {
                    obj_sprite = obj_sprite_lut[obj_type];
                }
                
                if (obj_sprite) {
                    int24_t obj_px = (int24_t)(obj_x * TILE_SIZE) - cameraX;
                    int24_t obj_py = (int24_t)(obj_y * TILE_SIZE) - cameraY;
                    
                    uint8_t active_diameter = state.lightDiameter > current_map_light_diameter ? state.lightDiameter : current_map_light_diameter;
                    if (active_diameter != 255) {
                        int radius = (active_diameter * 16) / 2;
                        int heroScreenX = state.playerX - cameraX + 8;
                        int heroScreenY = state.playerY - cameraY + 8;
                        int objScreenX = obj_px + 8;
                        int objScreenY = obj_py + 8;
                        int dx = objScreenX - heroScreenX;
                        int dy = objScreenY - heroScreenY;
                        if (dx < 0) dx = -dx;
                        if (dy < 0) dy = -dy;
                        if (dx > radius || dy > radius) {
                            continue; // In darkness, skip drawing
                        }
                    }
                    
                    if (obj_px >= -16 && obj_px < 320 && obj_py >= -16 && obj_py < 240) {
                        if (obj_type >= OBJ_CHEST) {
                            gfx_Sprite(obj_sprite, obj_px, obj_py);
                        } else {
                            gfx_TransparentSprite(obj_sprite, obj_px, obj_py);
                        }
                    }
                }
            }
            
            gfx_sprite_t *current_hero_sprite = hero_down;
            if (state.playerDirection == DIR_NORTH) current_hero_sprite = hero_up;
            else if (state.playerDirection == DIR_WEST) current_hero_sprite = hero_left;
            else if (state.playerDirection == DIR_EAST) current_hero_sprite = hero_right;
            
            bool carry = check_flag(FLAG_PM_CARRYING_PRINCESS);
            int px = state.playerX - cameraX;
            int py = state.playerY - cameraY;
            
            if (carry && (state.playerDirection == DIR_SOUTH || state.playerDirection == DIR_EAST)) {
                // Draw princess trailing behind/above/left
                if (state.playerDirection == DIR_SOUTH) gfx_TransparentSprite_NoClip(princess, px, py - 12);
                if (state.playerDirection == DIR_EAST) gfx_TransparentSprite_NoClip(princess, px - 12, py);
            }
            
            gfx_TransparentSprite_NoClip(current_hero_sprite, px, py);
            
            if (carry && (state.playerDirection == DIR_NORTH || state.playerDirection == DIR_WEST)) {
                // Draw princess trailing in front/below/right (drawn last to overlay hero properly)
                if (state.playerDirection == DIR_NORTH) gfx_TransparentSprite_NoClip(princess, px, py + 12);
                if (state.playerDirection == DIR_WEST) gfx_TransparentSprite_NoClip(princess, px + 12, py);
            }
            
            // 2. Draw State-Specific UI Overlays
            if (state.currentState == STATE_COMBAT || state.currentState == STATE_COMBAT_SPELLS || state.currentState == STATE_COMBAT_ITEM) {
                ui_DrawCombatScreen();
                
                gfx_sprite_t *mSprite = (gfx_sprite_t *)currentMonsterDef->sprite;
                
                // Center monster sprite in the Monster Window (110, 10, width 100, height 100)
                gfx_TransparentSprite_NoClip(mSprite, 160 - (mSprite->width / 2), 60 - (mSprite->height / 2)); 
                
                ui_DrawCombatStats(heroStats.hp, heroStats.maxHp, heroStats.mp, heroStats.maxMp, enemyStats.hp);
                
                // Hide message if we are actively using the command menu, keep UI clean
                if (combatPhase != PHASE_COMMAND || state.currentState == STATE_COMBAT_SPELLS || state.currentState == STATE_COMBAT_ITEM) {
                    ui_DrawCombatMessage(state.genericMsg);
                }
                
                if (state.currentState == STATE_COMBAT_SPELLS) {
                    ui_DrawSpellMenu(heroStats.level, spellMenuIndex, spellScrollOffset);
                } else if (state.currentState == STATE_COMBAT_ITEM) {
                    ui_DrawInventoryMenu(currentInventoryList, numInventoryItems, inventoryMenuIndex, inventoryScrollOffset, inventoryTab);
                } else if (combatPhase == PHASE_COMMAND) {
                    ui_DrawCombatCommandMenu(combatMenuIndex);
                }
            } else if (state.currentState == STATE_MENU) {
                ui_DrawCommandMenu(commandMenuIndex);
            } else if (state.currentState == STATE_INVENTORY || state.currentState == STATE_INVENTORY_ACTION || state.currentState == STATE_INVENTORY_DROP_CONFIRM) {
                ui_DrawInventoryMenu(currentInventoryList, numInventoryItems, inventoryMenuIndex, inventoryScrollOffset, inventoryTab);
                if (state.currentState == STATE_INVENTORY_ACTION || state.currentState == STATE_INVENTORY_DROP_CONFIRM) {
                    ui_DrawInventoryActionMenu(inventoryActionIndex);
                }
                if (state.currentState == STATE_INVENTORY_DROP_CONFIRM) {
                    ui_DrawDropConfirm(currentInventoryList[inventoryMenuIndex]);
                }
            } else if (state.currentState == STATE_SPELLS) {
                ui_DrawSpellMenu(heroStats.level, spellMenuIndex, spellScrollOffset);
            } else if (state.currentState == STATE_STATS) {
                ui_DrawStatsMenu(&heroStats);
            } else if (state.currentState == STATE_LEVEL_UP) {
                ui_DrawLevelUpDialog();
            } else if (state.currentState == STATE_MESSAGE) {
                ui_DrawMessageWindow(state.genericMsg);
            } else if (state.currentState == STATE_DIALOG_MENU) {
                ui_DrawDialogMenu(dialogMenuIndex);
            } else if (state.currentState == STATE_VENDOR_BUY) {
                ui_DrawVendorBuy(vendorMenuIndex, vendorScrollOffset);
            } else if (state.currentState == STATE_VENDOR_SELL) {
                ui_DrawVendorSell(vendorMenuIndex, vendorScrollOffset);
            } else if (state.currentState == STATE_SELECT_SAVE_SLOT) {
                ui_DrawSaveLoadMenu(saveSlotIndex, true);
            }
            
            if (state.currentState == STATE_MENU || state.currentState == STATE_INVENTORY || state.currentState == STATE_INVENTORY_ACTION || state.currentState == STATE_INVENTORY_DROP_CONFIRM || state.currentState == STATE_SPELLS || state.currentState == STATE_STATS || state.currentState == STATE_DIALOG_MENU || state.currentState == STATE_VENDOR_BUY || state.currentState == STATE_VENDOR_SELL || state.currentState == STATE_MESSAGE || state.currentState == STATE_LEVEL_UP || state.currentState == STATE_SELECT_SAVE_SLOT) {
                ui_DrawNavigationFooter();
            }
        }
        gfx_SwapDraw();
        
        if (needsFadeIn) {
            needsFadeIn = false;
            // Screen Transition: Fade In
            for (int step = 0; step <= 8; step++) {
                gfx_SetPalette(fade_palettes[step], sizeof_global_palette, 0);
                delay(20); // Delay ~20ms per step
            }
            gfx_SetPalette(global_palette, sizeof_global_palette, 0);
        }
    }

    gfx_End();
}
