#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "game.h"

// Draw a classic RPG-style menu window
void ui_DrawMenuWindow(uint16_t x, uint8_t y, uint16_t width, uint8_t height);

// Draw a selection pointer for menus
void ui_DrawSelectionPointer(int x, int y);

// Draw the global navigation hints footer
void ui_DrawNavigationFooter(void);

// Draw the main command menu
void ui_DrawCommandMenu(int selectedIndex);

// Draw the combat screen base overlay
void ui_DrawCombatScreen(void);

// Draw combat messages, stats, and command menu
void ui_DrawCombatStats(uint16_t heroHp, uint16_t heroMaxHp, uint16_t heroMp, uint16_t heroMaxMp, uint16_t enemyHp);
void ui_DrawCombatMessage(const char* message);
void ui_DrawCombatCommandMenu(int selectedIndex);

// Draw the splash title menu
void ui_DrawSplashMenu(int selectedIndex);

// Draw the inventory menu
void ui_DrawInventoryMenu(const ItemEnum* inventoryList, uint8_t numItems, int selectedIndex, int scrollOffset, int inventoryTab);
void ui_DrawInventoryActionMenu(bool canEquip, int selectedIndex);
void ui_DrawDropConfirm(ItemEnum item);

// Draw the spells menu
void ui_DrawSpellMenu(uint8_t heroLevel, int selectedIndex, int scrollOffset);

// Draw the dialog menu
void ui_DrawDialogMenu(int selectedIndex);

// Draw vendor buy menu
void ui_DrawVendorBuy(int selectedIndex, int scrollOffset);

// Draw vendor sell menu
void ui_DrawVendorSell(int selectedIndex, int scrollOffset);

// Draw the stats menu
void ui_DrawStatsMenu(const EntityStats* stats);

// Draw a generic message popup
void ui_DrawMessageWindow(const char* message);
void ui_DrawProgressBar(int x, int y, int width, int height, int percent);

// Draw level up dialog
void ui_DrawLevelUpDialog(void);

// Draw save/load menu
void ui_DrawSaveLoadMenu(int selectedIndex, bool isSaving);
void ui_DrawSelectSaveSlot(int selectedIndex);

// Draw randomizer UI menus
void ui_DrawSeedInputMenu(const char *seedBuffer);
void ui_DrawFlagSelectMenu(uint32_t seed, uint8_t presetIndex, uint16_t currentFlags, int selectedIndex);
void ui_DrawGeneratingLoader(uint32_t seed, uint16_t flags);

#endif
