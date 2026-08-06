#include "hero.h"
#include "game.h"
#include "level_data.h"
#include "item_data.h"
#include "randomizer.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EntityStats heroStats;

uint8_t inventoryMenuIndex = 0;
uint8_t inventoryTab = 0; // 0=ITEMS, 1=EQUIP, 2=KEY
uint8_t inventoryScrollOffset = 0;
uint8_t inventoryActionIndex = 0;
ItemEnum currentInventoryList[NUM_ITEMS];
uint8_t numInventoryItems = 0;

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

bool hasItem(ItemEnum item) {
    if (item >= NUM_ITEMS) return false;
    return state.inventory[item] > 0;
}

bool useItem(ItemEnum item) {
    if (!hasItem(item)) return false;
    if (item == ITEM_HERB) {
        removeItem(ITEM_HERB);
        uint16_t heal = 23 + (rand() % 8);
        heroStats.hp += heal;
        if (heroStats.hp > heroStats.maxHp) heroStats.hp = heroStats.maxHp;
        sprintf(state.genericMsg, "Used Herb! Restored %d HP. [2ND]", heal);
        state.currentState = STATE_MESSAGE;
        return true;
    } else if (item == ITEM_TORCH) {
        removeItem(ITEM_TORCH);
        current_map_light_diameter = 3;
        sprintf(state.genericMsg, "Lighted Torch! [2ND]");
        state.currentState = STATE_MESSAGE;
        return true;
    } else if (item == ITEM_FAIRY_WATER) {
        removeItem(ITEM_FAIRY_WATER);
        state.repelSteps = 128;
        sprintf(state.genericMsg, "Used Fairy Water! [2ND]");
        state.currentState = STATE_MESSAGE;
        return true;
    } else if (item == ITEM_WINGS) {
        removeItem(ITEM_WINGS);
        strcpy(state.currentMapName, "PYDW037");
        state.playerX = 4 * TILE_SIZE;
        state.playerY = 5 * TILE_SIZE;
        map_Load("PYDW037");
        sprintf(state.genericMsg, "Returned to Tantegel! [2ND]");
        state.currentState = STATE_MESSAGE;
        return true;
    }
    return false;
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

void checkLevelUp(void) {
    bool leveled = false;
    uint8_t oldLevel = heroStats.level;
    
    for (uint8_t i = 0; i < NUM_LEVELS; i++) {
        const LevelDef *lvl = get_level_def(i);
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
        const LevelDef *curLvl = get_level_def(heroStats.level - 1);
        heroStats.maxHp = curLvl->maxHp;
        heroStats.maxMp = curLvl->maxMp;
        heroStats.strength = curLvl->strength;
        heroStats.agility = curLvl->agility;
        state.currentState = STATE_LEVEL_UP;
    }
}

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
    const LevelDef *startLvl = get_level_def(0);
    heroStats.hp = startLvl->maxHp;
    heroStats.maxHp = startLvl->maxHp;
    heroStats.mp = startLvl->maxMp;
    heroStats.maxMp = startLvl->maxMp;
    heroStats.strength = startLvl->strength;
    heroStats.agility = startLvl->agility;
}
