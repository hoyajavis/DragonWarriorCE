#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t splashMenuIndex;
extern uint8_t commandMenuIndex;
extern uint8_t spellMenuIndex;
extern uint8_t spellScrollOffset;
extern uint8_t dialogMenuIndex;
extern uint8_t vendorMenuIndex;
extern uint8_t vendorScrollOffset;

extern uint8_t saveSlotIndex;
extern char seedInputBuf[16];
extern uint8_t presetIndex;
extern int flagMenuIndex;

void updateMenu(void);
void renderMenu(void);

#endif // MENU_H
