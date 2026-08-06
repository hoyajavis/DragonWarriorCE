#ifndef HERO_H
#define HERO_H

#include <stdint.h>
#include <stdbool.h>
#include "game.h"
#include "item_data.h"

extern EntityStats heroStats;

extern uint8_t inventoryMenuIndex;
extern uint8_t inventoryTab;
extern uint8_t inventoryScrollOffset;
extern uint8_t inventoryActionIndex;
extern ItemEnum currentInventoryList[NUM_ITEMS];
extern uint8_t numInventoryItems;

void initNewGame(void);
void checkLevelUp(void);
void giveItem(ItemEnum item);
void removeItem(ItemEnum item);
bool hasItem(ItemEnum item);
bool useItem(ItemEnum item);
uint16_t getHeroAttack(void);
uint16_t getHeroDefense(void);
void markStatsDirty(void);
void rebuildInventoryList(void);

#endif // HERO_H
