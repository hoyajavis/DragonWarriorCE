#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

#include "gfx/gfx.h"
#include "game.h"
#include "map.h"
#include "hero.h"
#include "combat.h"
#include "exploring.h"
#include "menu.h"
#include "effects.h"
#include "input.h"

// Define primary game state
GameState state;

int main(void) {
    if (PYDWGFX_init(NULL) == 0) {
        return 1;
    }
    gfx_Begin();
    gfx_SetDrawBuffer(); 
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    map_InitLUT();
    
    srand(rtc_Time());
    
    // Wait for all keys to be released before starting the game loop
    do {
        kb_Scan();
    } while (kb_AnyKey());
    
    // Set safe defaults for RETURN/OUTSIDE teleporting
    strcpy(state.lastOutsideMap, "PYDW001");
    state.lastOutsideX = 43 * TILE_SIZE;
    state.lastOutsideY = 44 * TILE_SIZE;

    state.currentState = STATE_SPLASH_MENU;
    state.exitFlag = false;

    while (!state.exitFlag) {
        kb_Scan(); 
        
        // 1. Logic Update Phase
        if (state.currentState == STATE_EXPLORING) {
            updateExploring();
        } else if (state.currentState == STATE_COMBAT || state.currentState == STATE_COMBAT_SPELLS || state.currentState == STATE_COMBAT_ITEM) {
            updateCombat();
        } else {
            updateMenu();
        }
        
        input_UpdatePrevKeys();

        // 2. Graphics Render Phase
        if (state.currentState == STATE_EXPLORING) {
            renderExploring();
        } else if (state.currentState == STATE_COMBAT || state.currentState == STATE_COMBAT_SPELLS || state.currentState == STATE_COMBAT_ITEM) {
            renderCombat();
        } else {
            renderMenu();
        }

        gfx_SwapDraw();
    }

    gfx_End();
    return 0;
}
