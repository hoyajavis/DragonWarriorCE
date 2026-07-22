#ifndef ITEM_DATA_H
#define ITEM_DATA_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_ITEMS 39

typedef enum {
    ITEM_TYPE_NONE = 0,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_ARMOR,
    ITEM_TYPE_SHIELD,
    ITEM_TYPE_TOOL
} ItemCategory;

typedef enum {
    ITEM_NONE = 0,
    ITEM_HANDS = 1,
    ITEM_BAMBOO_POLE = 2,
    ITEM_CLUB = 3,
    ITEM_COPPER_SWORD = 4,
    ITEM_HAND_AXE = 5,
    ITEM_BROAD_SWORD = 6,
    ITEM_FLAME_SWORD = 7,
    ITEM_ERDRICKS_SWORD = 8,
    ITEM_CLOTHES = 9,
    ITEM_LEATHER_ARMOR = 10,
    ITEM_CHAIN_MAIL = 11,
    ITEM_HALF_PLATE = 12,
    ITEM_FULL_PLATE = 13,
    ITEM_MAGIC_ARMOR = 14,
    ITEM_ERDRICKS_ARMOR = 15,
    ITEM_LEATHER_SHIELD = 16,
    ITEM_IRON_SHIELD = 17,
    ITEM_SILVER_SHIELD = 18,
    ITEM_ERDRICKS_SHIELD = 19,
    ITEM_DRAGONS_SCALE = 20,
    ITEM_FAIRY_WATER = 21,
    ITEM_FIGHTERS_RING = 22,
    ITEM_HERB = 23,
    ITEM_KEY = 24,
    ITEM_KEY_RING = 25,
    ITEM_TORCH = 26,
    ITEM_LANTERN = 27,
    ITEM_WINGS = 28,
    ITEM_BALL_OF_LIGHT = 29,
    ITEM_FAIRY_FLUTE = 30,
    ITEM_ERDRICKS_TOKEN = 31,
    ITEM_STAFF_OF_RAIN = 32,
    ITEM_STONES_OF_SUNLIGHT = 33,
    ITEM_RAINBOW_DROP = 34,
    ITEM_SILVER_HARP = 35,
    ITEM_GWAELINS_LOVE = 36,
    ITEM_CURSED_BELT = 37,
    ITEM_DEATH_NECKLACE = 38,
} ItemEnum;

typedef struct {
    const char *name;
    ItemCategory type;
    uint8_t attackBonus;
    uint8_t defenseBonus;
    uint16_t price;
    uint16_t useActionId;
    bool isEquippable;
    bool isKeyItem;
} ItemDef;

extern const ItemDef itemTable[NUM_ITEMS];

#endif
