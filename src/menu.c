#include "menu.h"
#include "game.h"
#include "hero.h"
#include "save.h"
#include "spells.h"
#include "randomizer.h"
#include "effects.h"
#include "input.h"
#include "ui.h"
#include "map.h"
#include "interactables.h"
#include "gfx/gfx.h"
#include <graphx.h>
#include <tice.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t splashMenuIndex = 0;
uint8_t commandMenuIndex = 0;
uint8_t spellMenuIndex = 0;
uint8_t spellScrollOffset = 0;
uint8_t dialogMenuIndex = 0;
uint8_t vendorMenuIndex = 0;
uint8_t vendorScrollOffset = 0;

uint8_t saveSlotIndex = 0;
char seedInputBuf[16] = "";
uint8_t presetIndex = 0;
int flagMenuIndex = 0;

void updateMenu(void) {
    switch (state.currentState) {
        case STATE_SPLASH_MENU:
            if (BTN_UP) {
                splashMenuIndex = (splashMenuIndex > 0) ? splashMenuIndex - 1 : 3;
            }
            if (BTN_DOWN) {
                splashMenuIndex = (splashMenuIndex < 3) ? splashMenuIndex + 1 : 0;
            }
            if (BTN_CONFIRM) {
                if (splashMenuIndex == 0) {
                    uint32_t defSeed = (uint32_t)rtc_Time();
                    if (defSeed == 0) defSeed = 123456789;
                    sprintf(seedInputBuf, "%lu", (unsigned long)defSeed);
                    state.currentState = STATE_SEED_INPUT;
                } else if (splashMenuIndex == 1) {
                    state.currentState = STATE_SELECT_LOAD_SLOT;
                    saveSlotIndex = 0;
                } else if (splashMenuIndex == 3) {
                    state.exitFlag = true;
                }
            }
            break;

        case STATE_SEED_INPUT:
            {
                struct { uint8_t row; kb_key_t key; char digit; } numKeys[] = {
                    {3, kb_0, '0'}, {3, kb_1, '1'}, {4, kb_2, '2'}, {5, kb_3, '3'},
                    {3, kb_4, '4'}, {4, kb_5, '5'}, {5, kb_6, '6'},
                    {3, kb_7, '7'}, {4, kb_8, '8'}, {5, kb_9, '9'}
                };
                
                kb_key_t cur_row_data[6] = {0, kb_Data[1], kb_Data[2], kb_Data[3], kb_Data[4], kb_Data[5]};
                kb_key_t prev_row_data[6] = {0, prev_key1, prev_key2, prev_key3, prev_key4, prev_key5};
                
                for (int k = 0; k < 10; k++) {
                    uint8_t r = numKeys[k].row;
                    kb_key_t keyMask = numKeys[k].key;
                    if ((cur_row_data[r] & keyMask) && !(prev_row_data[r] & keyMask)) {
                        size_t len = strlen(seedInputBuf);
                        if (len < 10) {
                            seedInputBuf[len] = numKeys[k].digit;
                            seedInputBuf[len + 1] = '\0';
                        }
                    }
                }
                
                if ((kb_Data[1] & kb_Window) && !(prev_key1 & kb_Window)) {
                    uint32_t newSeed = (uint32_t)rtc_Time() ^ rand();
                    sprintf(seedInputBuf, "%lu", (unsigned long)newSeed);
                }
                
                if ((kb_Data[1] & kb_Del) && !(prev_key1 & kb_Del)) {
                    size_t len = strlen(seedInputBuf);
                    if (len > 0) seedInputBuf[len - 1] = '\0';
                }
                
                if (BTN_CANCEL) {
                    state.currentState = STATE_SPLASH_MENU;
                }
                
                if (BTN_CONFIRM) {
                    if (strlen(seedInputBuf) > 0) {
                        state.seed = (uint32_t)strtoul(seedInputBuf, NULL, 10);
                    } else {
                        state.seed = 123456789;
                    }
                    presetIndex = PRESET_STANDARD;
                    state.randoFlags = get_preset_flags(PRESET_STANDARD);
                    flagMenuIndex = 0;
                    state.currentState = STATE_FLAG_SELECT;
                }
            }
            break;
            
        case STATE_FLAG_SELECT:
            {
                if (BTN_UP) {
                    flagMenuIndex = (flagMenuIndex > 0) ? flagMenuIndex - 1 : 10;
                }
                if (BTN_DOWN) {
                    flagMenuIndex = (flagMenuIndex < 10) ? flagMenuIndex + 1 : 0;
                }
                
                if (flagMenuIndex == 0) {
                    if (BTN_LEFT) {
                        presetIndex = (presetIndex > 0) ? presetIndex - 1 : 4;
                        if (presetIndex < 4) state.randoFlags = get_preset_flags(presetIndex);
                    }
                    if (BTN_RIGHT) {
                        presetIndex = (presetIndex < 4) ? presetIndex + 1 : 0;
                        if (presetIndex < 4) state.randoFlags = get_preset_flags(presetIndex);
                    }
                }
                
                if (BTN_CONFIRM) {
                    if (flagMenuIndex == 0) {
                        presetIndex = (presetIndex < 4) ? presetIndex + 1 : 0;
                        if (presetIndex < 4) state.randoFlags = get_preset_flags(presetIndex);
                    } else if (flagMenuIndex >= 1 && flagMenuIndex <= 9) {
                        state.randoFlags ^= bit_mask[flagMenuIndex - 1];
                        presetIndex = PRESET_CUSTOM;
                    } else if (flagMenuIndex == 10) {
                        state.currentState = STATE_GENERATING;
                    }
                }
                
                if (BTN_CANCEL) {
                    state.currentState = STATE_SEED_INPUT;
                }
            }
            break;

        case STATE_GENERATING:
            {
                ui_DrawGeneratingLoader(state.seed, state.randoFlags);
                gfx_SwapDraw();
                
                if (!map_Load("PYDW037")) {
                    strcpy(state.genericMsg, "ERROR: Missing PYDW037.8xv!");
                    state.currentState = STATE_SPLASH_MENU;
                } else {
                    init_randomizer(state.seed, state.randoFlags);
                    initNewGame();
                    fadeInFromBlack();
                    state.currentState = STATE_EXPLORING;
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
                    fadeInFromBlack();
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

        case STATE_MENU:
            if (BTN_UP) commandMenuIndex = (commandMenuIndex > 0) ? commandMenuIndex - 1 : 3;
            if (BTN_DOWN) commandMenuIndex = (commandMenuIndex < 3) ? commandMenuIndex + 1 : 0;
            if (BTN_CANCEL) state.currentState = STATE_EXPLORING;
            if (BTN_CONFIRM) {
                if (commandMenuIndex == 0) {
                    inventoryTab = 0; inventoryMenuIndex = 0; inventoryScrollOffset = 0;
                    rebuildInventoryList();
                    state.currentState = STATE_INVENTORY;
                } else if (commandMenuIndex == 1) {
                    spellMenuIndex = 0; spellScrollOffset = 0;
                    state.currentState = STATE_SPELLS;
                } else if (commandMenuIndex == 2) {
                    state.currentState = STATE_STATS;
                } else if (commandMenuIndex == 3) {
                    state.currentState = STATE_SELECT_SAVE_SLOT;
                    saveSlotIndex = 0;
                }
            }
            break;

        case STATE_INVENTORY:
            if (BTN_LEFT) {
                inventoryTab = (inventoryTab > 0) ? inventoryTab - 1 : 2;
                inventoryMenuIndex = 0; inventoryScrollOffset = 0;
                rebuildInventoryList();
            }
            if (BTN_RIGHT) {
                inventoryTab = (inventoryTab < 2) ? inventoryTab + 1 : 0;
                inventoryMenuIndex = 0; inventoryScrollOffset = 0;
                rebuildInventoryList();
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
                    inventoryActionIndex = 0;
                    state.currentState = STATE_INVENTORY_ACTION;
                }
            }
            if (BTN_CANCEL) state.currentState = STATE_MENU;
            break;

        case STATE_INVENTORY_ACTION:
            {
                ItemEnum selItem = currentInventoryList[inventoryMenuIndex];
                const ItemDef *def = &itemTable[selItem];
                bool canEquip = (def->type == ITEM_TYPE_WEAPON || def->type == ITEM_TYPE_ARMOR || def->type == ITEM_TYPE_SHIELD);
                
                if (BTN_UP) inventoryActionIndex = (inventoryActionIndex > 0) ? inventoryActionIndex - 1 : (canEquip ? 1 : 0);
                if (BTN_DOWN) inventoryActionIndex = (inventoryActionIndex < (canEquip ? 1 : 0)) ? inventoryActionIndex + 1 : 0;
                if (BTN_CANCEL) state.currentState = STATE_INVENTORY;
                if (BTN_CONFIRM) {
                    if (inventoryActionIndex == 0) { // USE
                        if (useItem(selItem)) {
                            rebuildInventoryList();
                            if (inventoryMenuIndex >= numInventoryItems && numInventoryItems > 0) {
                                inventoryMenuIndex = numInventoryItems - 1;
                            }
                        }
                    } else if (inventoryActionIndex == 1 && canEquip) { // EQUIP
                        if (def->type == ITEM_TYPE_WEAPON) state.equippedWeapon = (state.equippedWeapon == selItem) ? ITEM_NONE : selItem;
                        else if (def->type == ITEM_TYPE_ARMOR) state.equippedArmor = (state.equippedArmor == selItem) ? ITEM_NONE : selItem;
                        else if (def->type == ITEM_TYPE_SHIELD) state.equippedShield = (state.equippedShield == selItem) ? ITEM_NONE : selItem;
                        markStatsDirty();
                        state.currentState = STATE_INVENTORY;
                    }
                }
            }
            break;

        case STATE_SPELLS:
            {
                SpellEnum known[16];
                int count = getKnownSpells(known);
                if (BTN_CANCEL) state.currentState = STATE_MENU;
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
                        castHeroSpell(known[spellMenuIndex]);
                    }
                }
            }
            break;
            
        case STATE_STATS:
            if (BTN_CANCEL) state.currentState = STATE_MENU;
            break;

        case STATE_MESSAGE:
            if (BTN_CONFIRM) {
                if (!continue_action()) state.currentState = STATE_EXPLORING;
            }
            break;
            
        case STATE_LEVEL_UP:
            if (BTN_CONFIRM) {
                state.currentState = STATE_EXPLORING;
                fadeInFromBlack();
            }
            break;
            
        case STATE_DIALOG_MENU:
            if (BTN_UP) dialogMenuIndex = (dialogMenuIndex > 0) ? dialogMenuIndex - 1 : state.numMenuOptions - 1;
            if (BTN_DOWN) dialogMenuIndex = (dialogMenuIndex < state.numMenuOptions - 1) ? dialogMenuIndex + 1 : 0;
            if (BTN_CONFIRM) {
                extern uint24_t current_action_offset;
                current_action_offset = state.menuOffsets[dialogMenuIndex];
                if (!continue_action()) state.currentState = STATE_EXPLORING;
            }
            break;
            
        case STATE_VENDOR_BUY:
            if (BTN_UP) {
                if (vendorMenuIndex > 0) {
                    vendorMenuIndex--;
                    if (vendorMenuIndex < vendorScrollOffset) vendorScrollOffset = vendorMenuIndex;
                }
            }
            if (BTN_DOWN) {
                if (vendorMenuIndex < state.vendorNumItems - 1) {
                    vendorMenuIndex++;
                    if (vendorMenuIndex >= vendorScrollOffset + 6) vendorScrollOffset = vendorMenuIndex - 5;
                }
            }
            if (BTN_CANCEL) {
                if (!continue_action()) state.currentState = STATE_EXPLORING;
            }
            if (BTN_CONFIRM) {
                ItemEnum selectedItem = state.vendorItemIds[vendorMenuIndex];
                uint16_t price = get_item_price(selectedItem);
                if (heroStats.gold >= price) {
                    heroStats.gold -= price;
                    giveItem(selectedItem);
                    if (!continue_action()) state.currentState = STATE_EXPLORING;
                }
            }
            break;
            
        case STATE_VENDOR_SELL:
            if (BTN_UP) {
                if (vendorMenuIndex > 0) {
                    vendorMenuIndex--;
                    if (vendorMenuIndex < vendorScrollOffset) vendorScrollOffset = vendorMenuIndex;
                }
            }
            if (BTN_DOWN) {
                if (vendorMenuIndex < state.vendorNumItems - 1) {
                    vendorMenuIndex++;
                    if (vendorMenuIndex >= vendorScrollOffset + 6) vendorScrollOffset = vendorMenuIndex - 5;
                }
            }
            if (BTN_CANCEL) {
                if (!continue_action()) state.currentState = STATE_EXPLORING;
            }
            if (BTN_CONFIRM) {
                ItemEnum selectedItem = state.vendorItemIds[vendorMenuIndex];
                removeItem(selectedItem);
                uint16_t basePrice = get_item_price(selectedItem);
                heroStats.gold += basePrice / 2;
                if (!continue_action()) state.currentState = STATE_EXPLORING;
            }
            break;

        default:
            break;
    }
}

void renderMenu(void) {
    if (state.currentState == STATE_SPLASH_MENU) {
        gfx_ZeroScreen();
        gfx_TransparentSprite(title, 35, 20);
        ui_DrawSplashMenu(splashMenuIndex);
    } else if (state.currentState == STATE_SEED_INPUT) {
        gfx_ZeroScreen();
        gfx_TransparentSprite(title, 35, 20);
        ui_DrawSeedInputMenu(seedInputBuf);
    } else if (state.currentState == STATE_FLAG_SELECT) {
        gfx_ZeroScreen();
        gfx_TransparentSprite(title, 35, 20);
        ui_DrawFlagSelectMenu(state.seed, presetIndex, state.randoFlags, flagMenuIndex);
    } else if (state.currentState == STATE_SELECT_LOAD_SLOT || state.currentState == STATE_SELECT_SAVE_SLOT) {
        gfx_ZeroScreen();
        gfx_TransparentSprite(title, 35, 20);
        ui_DrawSelectSaveSlot(saveSlotIndex);
    } else if (state.currentState == STATE_MENU) {
        ui_DrawCommandMenu(commandMenuIndex);
    } else if (state.currentState == STATE_INVENTORY) {
        ui_DrawInventoryMenu(currentInventoryList, numInventoryItems, inventoryMenuIndex, inventoryScrollOffset, inventoryTab);
    } else if (state.currentState == STATE_INVENTORY_ACTION) {
        ui_DrawInventoryMenu(currentInventoryList, numInventoryItems, inventoryMenuIndex, inventoryScrollOffset, inventoryTab);
        ItemEnum selItem = currentInventoryList[inventoryMenuIndex];
        bool canEquip = (itemTable[selItem].type == ITEM_TYPE_WEAPON || itemTable[selItem].type == ITEM_TYPE_ARMOR || itemTable[selItem].type == ITEM_TYPE_SHIELD);
        ui_DrawInventoryActionMenu(canEquip, inventoryActionIndex);
    } else if (state.currentState == STATE_SPELLS) {
        ui_DrawSpellMenu(heroStats.level, spellMenuIndex, spellScrollOffset);
    } else if (state.currentState == STATE_STATS) {
        ui_DrawStatsMenu(&heroStats);
    } else if (state.currentState == STATE_MESSAGE) {
        ui_DrawMessageWindow(state.genericMsg);
    } else if (state.currentState == STATE_LEVEL_UP) {
        ui_DrawLevelUpDialog();
    } else if (state.currentState == STATE_DIALOG_MENU) {
        ui_DrawDialogMenu(dialogMenuIndex);
    } else if (state.currentState == STATE_VENDOR_BUY) {
        ui_DrawVendorBuy(vendorMenuIndex, vendorScrollOffset);
    } else if (state.currentState == STATE_VENDOR_SELL) {
        ui_DrawVendorSell(vendorMenuIndex, vendorScrollOffset);
    }
}
