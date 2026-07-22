#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "item_data.h"
#include "spell_data.h"
#include "monster_data.h"

// Application States
typedef enum {
    STATE_EXPLORING,
    STATE_MENU,
    STATE_COMBAT,
    STATE_INVENTORY,
    STATE_INVENTORY_ACTION,
    STATE_INVENTORY_DROP_CONFIRM,
    STATE_MESSAGE,
    STATE_SPELLS,
    STATE_COMBAT_SPELLS,
    STATE_STATS,
    STATE_SPLASH_MENU,
    STATE_GENERATING,
    STATE_DIALOG_MENU,
    STATE_VENDOR_BUY,
    STATE_VENDOR_SELL,
    STATE_LEVEL_UP,
    STATE_COMBAT_ITEM,
    STATE_SELECT_SAVE_SLOT,
    STATE_SELECT_LOAD_SLOT
} AppStateEnum;

#define TILE_SIZE 16

#define FLAG_PM_CREATED_RAINBOW_BRIDGE 214

#define EVENT_FLAG_COUNT 512

#define MAX_INVENTORY_ITEMS 16

// Items are now defined in item_data.h
#include "item_data.h"
// Monsters
// MonsterDef moved to monster_data.h


typedef struct {
    uint8_t level;
    uint24_t xp;
    uint16_t hp;
    uint16_t maxHp;
    uint16_t mp;
    uint16_t maxMp;
    uint16_t strength;
    uint16_t defense;
    uint16_t agility;
    uint32_t gold;
    bool isAsleep;
    uint8_t turnsAsleep;
    bool isSpellsBlocked;
} EntityStats;

// Directions
typedef enum {
    DIR_NORTH = 0,
    DIR_SOUTH,
    DIR_EAST,
    DIR_WEST
} DirectionEnum;

// Fixed State Structure
typedef struct {
    uint16_t playerX; // Absolute World Coordinates
    uint16_t playerY;
    DirectionEnum playerDirection;
    AppStateEnum currentState;
    bool exitFlag;
    char genericMsg[64];
    uint8_t inventory[NUM_ITEMS]; // Array of quantities for each ItemEnum
    ItemEnum equippedWeapon;
    ItemEnum equippedArmor;
    ItemEnum equippedShield;
    ItemEnum equippedAccessory;
    uint8_t repelSteps;
    uint8_t hpRegenStepCounter;
    uint8_t event_flags[64]; // 512 global boolean flags
    char currentMapName[8];
    char lastOutsideMap[8];
    uint16_t lastOutsideX;
    uint16_t lastOutsideY;
    uint8_t currentMonster;
    uint16_t combatVictoryActionId;
    
    // Light variables
    uint8_t lightDiameter;
    uint8_t lightDecaySteps;
    uint8_t lightDecayCounter;
    
    // Dialog Menu State
    uint8_t numMenuOptions;
    char menuOptions[4][32];
    uint24_t menuOffsets[4];
    
    // Vendor State
    uint8_t vendorNumItems;
    uint8_t vendorItemIds[8];
    
    // UI State
    bool showLevelUpDialog;
} GameState;

typedef struct {
    bool slotInUse;
    GameState state;
    EntityStats savedHero;
} SaveSlot;

typedef struct {
    SaveSlot slots[3];
} SaveFile;

extern GameState state;
extern EntityStats heroStats;
extern EntityStats enemyStats;

void giveItem(ItemEnum item);
void removeItem(ItemEnum item);
bool hasItem(ItemEnum item);
void triggerCombat(void);
void checkLevelUp(void);
void triggerScriptedCombat(uint8_t monster_id, uint16_t victory_action_id);
uint16_t getHeroAttack(void);
uint16_t getHeroDefense(void);
void markStatsDirty(void);
extern const uint8_t bit_mask[8];

static inline bool check_flag(uint16_t flag_id) {
    uint8_t byte_idx = flag_id >> 3;
    if (byte_idx >= 64) return false;
    return (state.event_flags[byte_idx] & bit_mask[flag_id & 7]) != 0;
}

static inline void set_flag(uint16_t flag_id) {
    uint8_t byte_idx = flag_id >> 3;
    if (byte_idx >= 64) return;
    state.event_flags[byte_idx] |= bit_mask[flag_id & 7];
}

static inline void clear_flag(uint16_t flag_id) {
    uint8_t byte_idx = flag_id >> 3;
    if (byte_idx >= 64) return;
    state.event_flags[byte_idx] &= ~bit_mask[flag_id & 7];
}

void rebuildInventoryList(void);

#endif
