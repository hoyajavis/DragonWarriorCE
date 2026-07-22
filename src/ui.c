#include "ui.h"
#include "game.h"
#include <graphx.h>
#include <stdio.h>
#include <string.h>
#include "item_data.h"
#include "spell_data.h"
#include "level_data.h"
#include "spells.h"
#include "save.h"

void ui_DrawMenuWindow(uint16_t x, uint8_t y, uint16_t width, uint8_t height) {
    gfx_SetColor(0x00);
    gfx_FillRectangle(x, y, width, height);
    gfx_SetColor(0xFF);
    gfx_Rectangle(x, y, width, height);
    gfx_Rectangle(x + 2, y + 2, width - 4, height - 4);
}

void ui_DrawSelectionPointer(int x, int y) {
    gfx_SetColor(0xFF);
    gfx_FillTriangle(x, y + 1, x, y + 7, x + 4, y + 4);
}

void ui_DrawCommandMenu(int selectedIndex) {
    ui_DrawMenuWindow(10, 10, 80, 110);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    static const char * const options[] = {"TALK", "SPELL", "ITEM", "STATS", "SEARCH", "SAVE"};
    
    int y = 20;
    for (int i = 0; i < 6; i++, y += 15) {
        if (selectedIndex == i) {
            ui_DrawSelectionPointer(15, y);
        }
        gfx_PrintStringXY(options[i], 25, y);
    }
}

void ui_DrawInventoryMenu(const ItemEnum* inventoryList, uint8_t numItems, int selectedIndex, int scrollOffset, int inventoryTab) {
    ui_DrawMenuWindow(70, 10, 180, 140);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    // Tab bar
    const char* tabs[3] = {"ITEMS", "EQUIP", "KEY"};
    int tabX = 75;
    for (int t = 0; t < 3; t++) {
        if (t == inventoryTab) {
            ui_DrawSelectionPointer( tabX-10, 22);
        }
        gfx_PrintStringXY(tabs[t], tabX, 22);
        tabX += 50;
    }
    
    gfx_PrintStringXY("INVENTORY:", 80, 35);
    
    if (numItems == 0) {
        gfx_PrintStringXY("EMPTY", 90, 50);
        return;
    }
    
    int maxItems = (numItems > 7) ? 7 : numItems;
    
    int y = 50;
    for (int i = 0; i < maxItems; i++, y += 15) {
        int listIdx = scrollOffset + i;
        if (listIdx >= numItems) break;
        
        ItemEnum item = inventoryList[listIdx];
        uint8_t qty = state.inventory[item];
        
        char equipTag = ' ';
        if (item == state.equippedWeapon || item == state.equippedArmor || item == state.equippedShield || item == state.equippedAccessory) {
            equipTag = 'E';
        }
        
        if (selectedIndex == listIdx) {
            ui_DrawSelectionPointer(80, y);
        }
        
        gfx_SetTextXY(90, y);
        gfx_PrintChar(equipTag);
        gfx_PrintChar(' ');
        if (qty < 10) gfx_PrintChar(' ');
        gfx_PrintUInt(qty, 1);
        gfx_PrintString("x ");
        gfx_PrintString(itemTable[item].name);
    }
}

void ui_DrawInventoryActionMenu(int selectedIndex) {
    ui_DrawMenuWindow(250, 10, 60, 60);
    static const char * const options[] = {"USE", "EQUIP", "DROP"};
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    int y = 20;
    for (int i = 0; i < 3; i++, y += 15) {
        if (selectedIndex == i) {
            ui_DrawSelectionPointer(255, y);
        }
        gfx_PrintStringXY(options[i], 265, y);
    }
}

void ui_DrawSpellMenu(uint8_t level, int selectedIndex, int scrollOffset) {
    (void)level;
    ui_DrawMenuWindow(170, 10, 140, 150);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    gfx_PrintStringXY("SPELLS:", 180, 20);
    
    SpellEnum known[16];
    int count = getKnownSpells(known);
    
    if (count == 0) {
        gfx_PrintStringXY("EMPTY", 185, 35);
        return;
    }
    
    int maxSpells = (count > 8) ? 8 : count;
    
    int y = 35;
    for (int i = 0; i < maxSpells; i++, y += 15) {
        int listIdx = scrollOffset + i;
        if (listIdx >= count) break;
        
        if (selectedIndex == listIdx) ui_DrawSelectionPointer( 175, y);
        
        gfx_SetTextXY(185, y);
        gfx_PrintString(spellTable[known[listIdx]].name);
        gfx_PrintChar('(');
        gfx_PrintUInt(spellTable[known[listIdx]].mp, 1);
        gfx_PrintChar(')');
    }
}

void ui_DrawStatsMenu(const EntityStats* stats) {
    ui_DrawMenuWindow(40, 10, 240, 140);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    gfx_PrintStringXY("CHARACTER STATS", 50, 20);
    
    // Left column: Base stats
    int leftX = 50;
    gfx_SetTextXY(leftX, 35); gfx_PrintString("LEVEL: "); gfx_PrintUInt(stats->level, 1);
    gfx_SetTextXY(leftX, 50); gfx_PrintString("HP: "); gfx_PrintUInt(stats->hp, 1); gfx_PrintString("/"); gfx_PrintUInt(stats->maxHp, 1);
    gfx_SetTextXY(leftX, 65); gfx_PrintString("MP: "); gfx_PrintUInt(stats->mp, 1); gfx_PrintString("/"); gfx_PrintUInt(stats->maxMp, 1);
    gfx_SetTextXY(leftX, 80); gfx_PrintString("ATK: "); gfx_PrintUInt(getHeroAttack(), 1);
    gfx_SetTextXY(leftX, 95); gfx_PrintString("DEF: "); gfx_PrintUInt(getHeroDefense(), 1);
    gfx_SetTextXY(leftX, 110); gfx_PrintString("GOLD: "); gfx_PrintUInt(stats->gold, 1); gfx_PrintString(" G");
    
    // Right column: XP and Equipment
    int rightX = 140;
    gfx_SetTextXY(rightX, 35); gfx_PrintString("XP: "); gfx_PrintUInt(stats->xp, 1);

    gfx_SetTextXY(rightX, 50); gfx_PrintString("NEXT: ");
    if (stats->level < NUM_LEVELS) {
        int nextXp = levelTable[stats->level].xpRequired - stats->xp;
        gfx_PrintUInt(nextXp, 1);
    } else {
        gfx_PrintString("---");
    }

    gfx_PrintStringXY("EQUIPMENT:", rightX, 70);
    gfx_PrintStringXY(state.equippedWeapon ? itemTable[state.equippedWeapon].name : "W: None", rightX, 85);
    gfx_PrintStringXY(state.equippedArmor ? itemTable[state.equippedArmor].name : "A: None", rightX, 100);
    gfx_PrintStringXY(state.equippedShield ? itemTable[state.equippedShield].name : "S: None", rightX, 115);
    gfx_PrintStringXY(state.equippedAccessory ? itemTable[state.equippedAccessory].name : "Acc: None", rightX, 130);
}

void ui_DrawLevelUpDialog(void) {
    ui_DrawMenuWindow(80, 50, 160, 100);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    gfx_PrintStringXY("LEVEL UP!", 120, 60);
    
    gfx_SetTextXY(90, 80); gfx_PrintString("Level: "); gfx_PrintUInt(heroStats.level, 1);
    gfx_SetTextXY(90, 95); gfx_PrintString("Max HP: "); gfx_PrintUInt(heroStats.maxHp, 1);
    gfx_SetTextXY(90, 110); gfx_PrintString("Max MP: "); gfx_PrintUInt(heroStats.maxMp, 1);
}

void ui_DrawCombatScreen(void) {
    ui_DrawMenuWindow(10, 10, 100, 80); // Stats Window
    ui_DrawMenuWindow(110, 10, 100, 100); // Monster Window
    ui_DrawMenuWindow(10, 170, 300, 60); // Message Window
}

void ui_DrawCombatStats(uint16_t heroHp, uint16_t heroMaxHp, uint16_t heroMp, uint16_t heroMaxMp, uint16_t enemyHp) {
    gfx_SetTextFGColor(0xFF); 
    gfx_SetTextBGColor(0x00); 
    gfx_SetTextTransparentColor(0x00);
    
    gfx_PrintStringXY("HERO", 20, 15);
    
    gfx_SetTextXY(20, 27); gfx_PrintString("HP: "); gfx_PrintUInt(heroHp, 1); gfx_PrintString("/"); gfx_PrintUInt(heroMaxHp, 1);
    
    int hpPercent = (heroMaxHp > 0) ? (heroHp * 100) / heroMaxHp : 0;
    ui_DrawProgressBar(20, 37, 80, 6, hpPercent);
    
    gfx_SetTextXY(20, 48); gfx_PrintString("MP: "); gfx_PrintUInt(heroMp, 1); gfx_PrintString("/"); gfx_PrintUInt(heroMaxMp, 1);
    
    int mpPercent = (heroMaxMp > 0) ? (heroMp * 100) / heroMaxMp : 0;
    ui_DrawProgressBar(20, 58, 80, 6, mpPercent);
    
    gfx_SetTextXY(20, 70); gfx_PrintString("E: "); gfx_PrintUInt(enemyHp, 1);
}

void ui_DrawCombatMessage(const char* message) {
    gfx_SetTextFGColor(0xFF); 
    gfx_SetTextBGColor(0x00); 
    gfx_SetTextTransparentColor(0x00);
    gfx_PrintStringXY(message, 20, 185);
}

void ui_DrawCombatCommandMenu(int selectedIndex) {
    ui_DrawMenuWindow(10, 95, 90, 80);
    static const char * const options[] = {"FIGHT", "SPELL", "ITEM", "RUN"};
    gfx_SetTextFGColor(0xFF); 
    gfx_SetTextBGColor(0x00); 
    gfx_SetTextTransparentColor(0x00);
    
    int y = 110;
    for (int i = 0; i < 4; i++, y += 15) {
        if (selectedIndex == i) {
            ui_DrawSelectionPointer(20, y);
        }
        gfx_PrintStringXY(options[i], 30, y);
    }
}

void ui_DrawMessageWindow(const char* message) {
    ui_DrawMenuWindow(10, 150, 300, 70);
    gfx_SetTextFGColor(0xFF); 
    gfx_SetTextBGColor(0x00); 
    gfx_SetTextTransparentColor(0x00);
    
    int msg_len = strlen(message);
    
    // Simple line wrapping
    int max_chars_per_line = 35;
    int current_char = 0;
    
    int drawY = 160;
    while (current_char < msg_len) {
        char line_buffer[64];
        int chars_to_copy = msg_len - current_char;
        if (chars_to_copy > max_chars_per_line) chars_to_copy = max_chars_per_line;
        
        strncpy(line_buffer, &message[current_char], chars_to_copy);
        line_buffer[chars_to_copy] = '\0';
        
        gfx_PrintStringXY(line_buffer, 20, drawY);
        drawY += 15;
        
        current_char += chars_to_copy;
    }
}

void ui_DrawProgressBar(int x, int y, int width, int height, int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    // Border
    gfx_SetColor(0xFF); // White
    gfx_Rectangle(x, y, width, height);
    gfx_Rectangle(x+1, y+1, width-2, height-2);
    
    // Fill
    int fillWidth = (width - 4) * percent / 100;
    gfx_SetColor(0xFF); // High contrast solid white fill
    if (fillWidth > 0) {
        gfx_FillRectangle(x+2, y+2, fillWidth, height-4);
    }
    
    // Erase rest
    gfx_SetColor(0x00);
    int emptyWidth = (width - 4) - fillWidth;
    if (emptyWidth > 0) {
        gfx_FillRectangle(x+2+fillWidth, y+2, emptyWidth, height-4);
    }
}

void ui_DrawDialogMenu(int selectedIndex) {
    ui_DrawMessageWindow(""); 
    
    gfx_SetTextFGColor(0xFF);
    
    int start_x = 40;
    int start_y = 180;
    
    int y = start_y;
    for (int i = 0; i < state.numMenuOptions; i++) {
        int x = (i & 1) ? start_x + 120 : start_x;
        if (i == 2) y += 15;
        
        if (i == selectedIndex) {
            ui_DrawSelectionPointer(x - 10, y);
        }
        gfx_PrintStringXY(state.menuOptions[i], x, y);
    }
}

void ui_DrawVendorBuy(int selectedIndex, int scrollOffset) {
    ui_DrawMenuWindow(10, 140, 300, 90);
    gfx_SetTextFGColor(0xFF);
    gfx_PrintStringXY("What dost thou wish to buy?", 20, 150);
    
    int start_x = 40;
    int start_y = 170;
    
    int maxItems = (state.vendorNumItems > 6) ? 6 : state.vendorNumItems;
    
    int y = start_y;
    for (int i = 0; i < maxItems; i++) {
        int listIdx = scrollOffset + i;
        if (listIdx >= state.vendorNumItems) break;
        
        int x = (i & 1) ? start_x + 140 : start_x;
        
        if (listIdx == selectedIndex) {
            ui_DrawSelectionPointer( x - 10, y);
        }
        
        ItemEnum item = state.vendorItemIds[listIdx];
        const char *name = itemTable[item].name;
        int price = itemTable[item].price;
        gfx_PrintStringXY(name, x, y);
        
        gfx_SetTextXY(x + 80, y);
        gfx_PrintUInt(price, 1);
        gfx_PrintString(" G");
        
        if (i & 1) y += 15;
    }
}

void ui_DrawVendorSell(int selectedIndex, int scrollOffset) {
    ui_DrawMenuWindow(10, 140, 300, 90);
    gfx_SetTextFGColor(0xFF);
    gfx_PrintStringXY("What dost thou wish to sell?", 20, 150);
    
    if (state.vendorNumItems == 0) {
        gfx_PrintStringXY("Thou hast nothing to sell.", 40, 170);
        return;
    }
    
    int start_x = 40;
    int start_y = 170;
    
    int maxItems = (state.vendorNumItems > 6) ? 6 : state.vendorNumItems;
    
    int y = start_y;
    for (int i = 0; i < maxItems; i++) {
        int listIdx = scrollOffset + i;
        if (listIdx >= state.vendorNumItems) break;
        
        int x = (i & 1) ? start_x + 140 : start_x;
        
        if (listIdx == selectedIndex) {
            ui_DrawSelectionPointer( x - 10, y);
        }
        
        ItemEnum item = state.vendorItemIds[listIdx];
        const char *name = itemTable[item].name;
        gfx_PrintStringXY(name, x, y);
        
        gfx_SetTextXY(x + 80, y);
        gfx_PrintUInt(itemTable[item].price / 2, 1);
        gfx_PrintString(" G");
        
        if (i & 1) y += 15;
    }
}

void ui_DrawNavigationFooter(void) {
    gfx_SetColor(0x00);
    gfx_FillRectangle(0, 225, 320, 15);
    gfx_SetColor(0xFF);
    gfx_HorizLine(0, 225, 320);
    
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    gfx_PrintStringXY("[2nd]:Ok   [ALPHA]:Back   [Arrows]:Select", 10, 228);
}

void ui_DrawDropConfirm(ItemEnum item) {
    ui_DrawMenuWindow(60, 90, 200, 60);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    gfx_SetTextXY(60 + (200 - 80) / 2, 105);
    gfx_PrintString("Drop ");
    gfx_PrintString(itemTable[item].name);
    gfx_PrintString("?");
    
    int w2 = gfx_GetStringWidth("[2nd] Yes   [ALPHA] No");
    gfx_PrintStringXY("[2nd] Yes   [ALPHA] No", 60 + (200 - w2) / 2, 125);
}

void ui_DrawSaveLoadMenu(int selectedIndex, bool isSaving) {
    ui_DrawMenuWindow(80, 50, 160, 100);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    
    gfx_PrintStringXY(isSaving ? "Select Save Slot:" : "Select Load Slot:", 95, 65);
    
    for (int i = 0; i < 3; i++) {
        if (selectedIndex == i) {
            ui_DrawSelectionPointer(90, 85 + i * 20);
        }
        gfx_SetTextXY(105, 85 + i * 20);
        gfx_PrintString("Slot ");
        gfx_PrintUInt(i + 1, 1);
        
        if (save_Exists((uint8_t)i)) {
            gfx_PrintString(": Saved");
        } else {
            gfx_PrintString(": Empty");
        }
    }
}
