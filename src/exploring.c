#include "exploring.h"
#include "game.h"
#include "map.h"
#include "hero.h"
#include "combat.h"
#include "effects.h"
#include "input.h"
#include "interactables.h"
#include "randomizer.h"
#include "ui.h"
#include "gfx/gfx.h"
#include <graphx.h>
#include <string.h>
#include <stdlib.h>

bool isMoving = false;
uint8_t moveFrames = 0;
int8_t moveStepX = 0;
int8_t moveStepY = 0;
bool needsFadeIn = false;
uint8_t inputDelay = 0;

void updateExploring(void) {
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
            uint16_t targetX = state.playerX;
            uint16_t targetY = state.playerY;
            
            if (state.playerDirection == DIR_NORTH) targetY -= TILE_SIZE;
            else if (state.playerDirection == DIR_SOUTH) targetY += TILE_SIZE;
            else if (state.playerDirection == DIR_EAST) targetX += TILE_SIZE;
            else if (state.playerDirection == DIR_WEST) targetX -= TILE_SIZE;
            
            if (targetX < ((uint16_t)current_map_width << 4) && targetY < ((uint16_t)current_map_height << 4)) {
                uint8_t targetTile = current_map_data[(targetY >> 4) * current_map_width + (targetX >> 4)];
                if (targetTile == TILE_COUNTER) {
                    if (state.playerDirection == DIR_NORTH) targetY -= 16;
                    else if (state.playerDirection == DIR_SOUTH) targetY += 16;
                    else if (state.playerDirection == DIR_EAST) targetX += 16;
                    else if (state.playerDirection == DIR_WEST) targetX -= 16;
                }
            }
            
            bool found = false;
            uint8_t *obj = current_map_interactables_raw;
            for (uint8_t i = 0; i < current_map_num_interactables; i++, obj += 6) {
                uint16_t objX = (uint16_t)obj[0] << 4;
                uint16_t objY = (uint16_t)obj[1] << 4;
                
                if (targetX == objX && targetY == objY) {
                    uint8_t objType = obj[2];
                    uint16_t actionId = obj[4] | (obj[5] << 8);
                    
                    if (objType == OBJ_CHEST) {
                        found = true;
                        start_action(actionId);
                        break;
                    } else if (objType == OBJ_DOOR) {
                        found = true;
                        if (hasItem(ITEM_KEY)) {
                            removeItem(ITEM_KEY);
                            start_action(actionId);
                        } else {
                            strcpy(state.genericMsg, "Thou hast not a key! [2ND]");
                            state.currentState = STATE_MESSAGE;
                        }
                        break;
                    } else if (objType >= OBJ_NPC_KING && objType <= OBJ_NPC_TRUMPETER) {
                        found = true;
                        start_action(actionId);
                        break;
                    }
                }
            }
            
            if (!found) {
                strcpy(state.genericMsg, "Nothing found here. [2ND]");
                state.currentState = STATE_MESSAGE;
            }
        }
    }

    if (isMoving) {
        state.playerX += moveStepX;
        state.playerY += moveStepY;
        moveFrames--;
        if (moveFrames == 0) {
            isMoving = false;
            
            if (state.repelSteps > 0) state.repelSteps--;
            
            uint16_t pTileX = state.playerX >> 4;
            uint16_t pTileY = state.playerY >> 4;
            
            if (current_map_transitions != NULL) {
                MapTransition *t = current_map_transitions;
                for (uint8_t i = 0; i < current_map_num_transitions; i++, t++) {
                    if (t->trigger_x == pTileX && t->trigger_y == pTileY) {
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
                            state.playerX = (uint16_t)sx << 4;
                            state.playerY = (uint16_t)sy << 4;
                            needsFadeIn = true;
                        }
                        break;
                    }
                }
            }
            
            // Check Random Encounters
            int encnt_denom = 16;
            if (is_flag_active(RND_FLAG_QOL_LOWER_ENCNT)) encnt_denom = 32;
            if (rand() % encnt_denom == 0) {
                triggerCombat();
            }
        }
    }
}

void renderExploring(void) {
    gfx_ZeroScreen();
    int camX = (int)state.playerX - 152;
    int camY = (int)state.playerY - 112;
    if (camX < 0) camX = 0;
    if (camY < 0) camY = 0;
    int maxCamX = ((int)current_map_width << 4) - 320;
    int maxCamY = ((int)current_map_height << 4) - 240;
    if (maxCamX < 0) maxCamX = 0;
    if (maxCamY < 0) maxCamY = 0;
    if (camX > maxCamX) camX = maxCamX;
    if (camY > maxCamY) camY = maxCamY;

    map_Draw(camX, camY);

    // Draw Hero Sprite
    gfx_sprite_t *heroSpr = hero;
    if (state.playerDirection == DIR_NORTH) heroSpr = hero_up;
    else if (state.playerDirection == DIR_SOUTH) heroSpr = hero_down;
    else if (state.playerDirection == DIR_WEST) heroSpr = hero_left;
    else if (state.playerDirection == DIR_EAST) heroSpr = hero_right;

    int renderX = (int)state.playerX - camX;
    int renderY = (int)state.playerY - camY;
    gfx_TransparentSprite_NoClip(heroSpr, renderX, renderY);

    // Fade in effect
    if (needsFadeIn) {
        needsFadeIn = false;
        fadeInFromBlack();
    }
}
