#include "interactables.h"
#include "game.h"
#include "action_ids.h"
#include "randomizer.h"
#include <fileioc.h>
#include <string.h>
#include <stdio.h>
#include "map.h"

#define OP_END 0x00
#define OP_TEXT 0x01
#define OP_JUMP_IF_FLAG 0x02
#define OP_JUMP_IF_NOT_FLAG 0x03
#define OP_SET_FLAG 0x04
#define OP_CLEAR_FLAG 0x05
#define OP_GIVE_ITEM 0x06
#define OP_REMOVE_ITEM 0x07
#define OP_CHECK_ITEM 0x08
#define OP_PLAY_SOUND 0x09
#define OP_GOTO_ACTION 0x0A
#define OP_PROMPT_YES_NO 0x0B
#define OP_GIVE_GOLD 0x0C
#define OP_JUMP 0x0D
#define OP_SHOW_MENU 0x0E
#define OP_VENDOR_BUY 0x0F
#define OP_VENDOR_SELL 0x10
#define OP_TAKE_GOLD 0x11
#define OP_CHECK_GOLD 0x12
#define OP_HEALTH_RESTORE 0x13
#define OP_ASSERT_FACING_LOCKED 0x14
#define OP_OPEN_LOCKED 0x15
#define OP_GOTO_COORDINATES 0x16
#define OP_REPEL_MONSTERS 0x17
#define OP_ASSERT_OUTSIDE 0x18
#define OP_ASSERT_NOT_COMBAT 0x19
#define OP_SET_LIGHT 0x1A
#define OP_PLAY_MUSIC 0x1B
#define OP_CAST_SPELL 0x1C
#define OP_START_ENCOUNTER 0x1D
#define OP_MAGIC_RESTORE 0x1E
#define OP_TRIGGER_RANDOM_ENCOUNTER 0x1F
#define OP_IS_AT_COORDINATES 0x20
#define OP_VISUAL_EFFECT 0x21

const uint8_t bit_mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

uint24_t current_action_offset = 0;

void start_action(uint16_t action_id) {
    if (action_id == 0) return;
    
    // NOTE TO FUTURE DEVELOPERS: 
    // The Dialogue VM (start_action / continue_action) cannot be safely executed 
    // during the Combat State Machine (STATE_COMBAT). The VM takes over `state.currentState`
    // (often jumping to STATE_MESSAGE or triggering other dialogs) and enters its own
    // rendering loops, which will break the rigid combat phase sequence. 
    // If you need items or spells to have effects in combat, they must be hardcoded
    // into the switch statements inside STATE_COMBAT_ITEM or STATE_COMBAT_SPELLS in main.c.
    
    uint8_t file_handle = ti_Open("PYDWTXT", "r");
    if (!file_handle) {
        strcpy(state.genericMsg, "ERROR: Missing PYDWTXT.8xv!");
        state.currentState = STATE_MESSAGE;
        return;
    }
    
    // Read number of actions
    uint16_t num_actions;
    ti_Read(&num_actions, 2, 1, file_handle);
    
    if (action_id >= num_actions) {
        ti_Close(file_handle);
        return;
    }
    
    // Read offset table entry
    ti_Seek(2 + (action_id * 2), SEEK_SET, file_handle);
    uint16_t offset;
    ti_Read(&offset, 2, 1, file_handle);
    ti_Close(file_handle);
    
    if (offset == 0) {
        // Empty action
        return;
    }
    
    current_action_offset = offset;
    continue_action();
}

bool continue_action(void) {
    if (current_action_offset == 0) return false;
    
    uint8_t file_handle = ti_Open("PYDWTXT", "r");
    if (!file_handle) return false;
    
    ti_Seek(current_action_offset, SEEK_SET, file_handle);
    
    bool executing = true;
    while (executing) {
        uint8_t opcode;
        if (ti_Read(&opcode, 1, 1, file_handle) != 1) {
            current_action_offset = 0;
            executing = false;
            break;
        }
        
        switch (opcode) {
            case OP_END:
                current_action_offset = 0;
                executing = false;
                break;
                
            case OP_TEXT:
                {
                    int i = 0;
                    uint8_t ch;
                    while (1) {
                        if (ti_Read(&ch, 1, 1, file_handle) != 1 || ch == 0) break;
                        if (i < (int)sizeof(state.genericMsg) - 1) {
                            state.genericMsg[i++] = ch;
                        }
                    }
                    state.genericMsg[i] = '\0';
                    state.currentState = STATE_MESSAGE;
                    current_action_offset = ti_Tell(file_handle);
                    executing = false; // Stop executing to wait for user input
                }
                break;
                
            case OP_JUMP_IF_FLAG:
                {
                    uint16_t flag_id, jump_offset;
                    ti_Read(&flag_id, 2, 1, file_handle);
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    
                    if (check_flag(flag_id)) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_JUMP_IF_NOT_FLAG:
                {
                    uint16_t flag_id, jump_offset;
                    ti_Read(&flag_id, 2, 1, file_handle);
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    
                    if (!check_flag(flag_id)) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_SET_FLAG:
                {
                    uint16_t flag_id;
                    ti_Read(&flag_id, 2, 1, file_handle);
                    set_flag(flag_id);
                }
                break;
                
            case OP_CLEAR_FLAG:
                {
                    uint16_t flag_id;
                    ti_Read(&flag_id, 2, 1, file_handle);
                    clear_flag(flag_id);
                }
                break;
                
            case OP_GIVE_ITEM:
                {
                    uint8_t item_id;
                    ti_Read(&item_id, 1, 1, file_handle);
                    giveItem(get_item_mapping((ItemEnum)item_id));
                }
                break;
                
            case OP_REMOVE_ITEM:
                {
                    uint8_t item_id;
                    ti_Read(&item_id, 1, 1, file_handle);
                    removeItem((ItemEnum)item_id);
                }
                break;
                
            case OP_GIVE_GOLD:
                {
                    uint16_t gold;
                    ti_Read(&gold, 2, 1, file_handle);
                    heroStats.gold += gold;
                    if (heroStats.gold > 99999) heroStats.gold = 99999;
                }
                break;
                
            case OP_GOTO_ACTION:
                {
                    uint16_t next_action;
                    ti_Read(&next_action, 2, 1, file_handle);
                    // Tail call
                    ti_Close(file_handle);
                    start_action(next_action);
                    return current_action_offset != 0;
                }
                break;
                
            case OP_JUMP:
                {
                    uint16_t jump_offset;
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    ti_Seek(jump_offset, SEEK_CUR, file_handle);
                }
                break;
                
            case OP_SHOW_MENU:
                {
                    ti_Read(&state.numMenuOptions, 1, 1, file_handle);
                    if (state.numMenuOptions > 4) state.numMenuOptions = 4;
                    for (int o = 0; o < state.numMenuOptions; o++) {
                        int i = 0;
                        uint8_t ch;
                        while (1) {
                            if (ti_Read(&ch, 1, 1, file_handle) != 1 || ch == 0) break;
                            if (i < (int)sizeof(state.menuOptions[o]) - 1) {
                                state.menuOptions[o][i++] = ch;
                            }
                        }
                        state.menuOptions[o][i] = '\0';
                    }
                    for (int o = 0; o < state.numMenuOptions; o++) {
                        ti_Read(&state.menuOffsets[o], 2, 1, file_handle);
                    }
                    state.currentState = STATE_DIALOG_MENU;
                    current_action_offset = ti_Tell(file_handle);
                    executing = false;
                }
                break;
                
            case OP_VENDOR_BUY:
                {
                    ti_Read(&state.vendorNumItems, 1, 1, file_handle);
                    if (state.vendorNumItems > 8) state.vendorNumItems = 8;
                    for (int o = 0; o < state.vendorNumItems; o++) {
                        ti_Read(&state.vendorItemIds[o], 1, 1, file_handle);
                    }
                    state.currentState = STATE_VENDOR_BUY;
                    current_action_offset = ti_Tell(file_handle);
                    executing = false;
                }
                break;
                
            case OP_VENDOR_SELL:
                {
                    uint8_t typeVal;
                    ti_Read(&typeVal, 1, 1, file_handle);
                    state.vendorNumItems = 0;
                    const ItemDef *item = &itemTable[1];
                    for (int i = 1; i < NUM_ITEMS; i++, item++) {
                        if (state.inventory[i] > 0 && item->price > 0) {
                            state.vendorItemIds[state.vendorNumItems++] = i;
                            if (state.vendorNumItems >= 8) break;
                        }
                    }
                    state.currentState = STATE_VENDOR_SELL;
                    current_action_offset = ti_Tell(file_handle);
                    executing = false;
                }
                break;
                
            case OP_TAKE_GOLD:
                {
                    uint16_t amount;
                    ti_Read(&amount, 2, 1, file_handle);
                    if (heroStats.gold >= amount) {
                        heroStats.gold -= amount;
                    } else {
                        heroStats.gold = 0;
                    }
                }
                break;
                
            case OP_CHECK_GOLD:
                {
                    uint16_t amount;
                    uint16_t jump_offset;
                    ti_Read(&amount, 2, 1, file_handle);
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    if (heroStats.gold < amount) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_HEALTH_RESTORE:
                {
                    uint8_t min_val, max_val;
                    ti_Read(&min_val, 1, 1, file_handle);
                    ti_Read(&max_val, 1, 1, file_handle);
                    if (min_val == 255) {
                        heroStats.hp = heroStats.maxHp;
                    } else {
                        uint8_t amount = min_val;
                        if (max_val > min_val) amount += rand() % (max_val - min_val + 1);
                        if ((int)heroStats.hp + amount > heroStats.maxHp) {
                            heroStats.hp = heroStats.maxHp;
                        } else {
                            heroStats.hp += amount;
                        }
                    }
                }
                break;
                
            case OP_MAGIC_RESTORE:
                heroStats.mp = heroStats.maxMp;
                break;
                
            case OP_ASSERT_FACING_LOCKED:
                {
                    uint16_t jump_offset;
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    int tx = state.playerX / TILE_SIZE;
                    int ty = state.playerY / TILE_SIZE;
                    if (state.playerDirection == DIR_NORTH) ty--;
                    else if (state.playerDirection == DIR_SOUTH) ty++;
                    else if (state.playerDirection == DIR_EAST) tx++;
                    else if (state.playerDirection == DIR_WEST) tx--;
                    
                    bool found_door = false;
                    uint8_t *obj = current_map_interactables_raw;
                    for (int i = 0; i < current_map_num_interactables; i++, obj += 6) {
                        if (obj[2] == OBJ_DOOR && obj[0] == tx && obj[1] == ty) {
                            found_door = true;
                            break;
                        }
                    }
                    if (!found_door) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_OPEN_LOCKED:
                {
                    int tx = state.playerX / TILE_SIZE;
                    int ty = state.playerY / TILE_SIZE;
                    if (state.playerDirection == DIR_NORTH) ty--;
                    else if (state.playerDirection == DIR_SOUTH) ty++;
                    else if (state.playerDirection == DIR_EAST) tx++;
                    else if (state.playerDirection == DIR_WEST) tx--;
                    
                    uint8_t *obj = current_map_interactables_raw;
                    for (int i = 0; i < current_map_num_interactables; i++, obj += 6) {
                        if (obj[2] == OBJ_DOOR && obj[0] == tx && obj[1] == ty) {
                            // Delete it by shifting the array
                            uint8_t remaining = current_map_num_interactables - 1 - i;
                            if (remaining > 0) {
                                memmove(obj, obj + 6, remaining * 6);
                            }
                            current_map_num_interactables--;
                            break;
                        }
                    }
                }
                break;
                
            case OP_GOTO_COORDINATES:
                {
                    strcpy(state.currentMapName, "PYDW001");
                    state.playerX = 52 * TILE_SIZE;
                    state.playerY = 45 * TILE_SIZE;
                    state.exitFlag = true; 
                    current_action_offset = 0;
                    executing = false;
                }
                break;
                
            case OP_REPEL_MONSTERS:
                {
                    uint8_t decay;
                    ti_Read(&decay, 1, 1, file_handle);
                    state.repelSteps = decay;
                }
                break;
                
            case OP_SET_LIGHT:
                {
                    uint8_t count;
                    uint8_t decay;
                    ti_Read(&count, 1, 1, file_handle);
                    ti_Read(&decay, 1, 1, file_handle);
                    state.lightDiameter = count;
                    state.lightDecaySteps = decay;
                    state.lightDecayCounter = decay;
                }
                break;
                
            case OP_ASSERT_OUTSIDE:
                {
                    uint16_t jump_offset;
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    if (strcmp(state.currentMapName, "PYDW001") != 0) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_ASSERT_NOT_COMBAT:
                {
                    uint16_t jump_offset;
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    if (state.currentState == STATE_COMBAT) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_START_ENCOUNTER:
                {
                    uint8_t monster_id;
                    uint16_t victory_id;
                    ti_Read(&monster_id, 1, 1, file_handle);
                    ti_Read(&victory_id, 2, 1, file_handle);
                    
                    triggerScriptedCombat(monster_id, victory_id);
                    
                    current_action_offset = 0;
                    executing = false;
                }
                break;
                
            case OP_TRIGGER_RANDOM_ENCOUNTER:
                {
                    current_action_offset = 0;
                    executing = false;
                    triggerCombat();
                }
                break;
                
            case OP_IS_AT_COORDINATES:
                {
                    uint8_t targetX;
                    uint8_t targetY;
                    uint16_t jump_offset;
                    ti_Read(&targetX, 1, 1, file_handle);
                    ti_Read(&targetY, 1, 1, file_handle);
                    ti_Read(&jump_offset, 2, 1, file_handle);
                    
                    if ((state.playerX / TILE_SIZE) == targetX && (state.playerY / TILE_SIZE) == targetY) {
                        ti_Seek(jump_offset, SEEK_CUR, file_handle);
                    }
                }
                break;
                
            case OP_VISUAL_EFFECT:
                {
                    uint8_t effect_id;
                    ti_Read(&effect_id, 1, 1, file_handle);
                    
                    if (effect_id == 0) { // rainbowEffect
                        if (current_map_data) {
                            current_map_data[50 * current_map_width + 65] = TILE_BRIDGE;
                        }
                    }
                }
                break;
                
            case OP_PLAY_MUSIC:
                // No parameters, no functional effect
                break;

                
            default:
                // Unknown opcode
                current_action_offset = 0;
                executing = false;
                break;
        }
    }
    
    ti_Close(file_handle);
    return current_action_offset != 0;
}
