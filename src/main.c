#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include <compression.h>

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

static void *gfx_data = NULL;

int main(void) {
    ti_var_t gfx_slot = ti_Open("PYDWGFX", "r");
    if (!gfx_slot) {
        return 1;
    }

    gfx_data = malloc(PYDWGFX_appvar_uncompressed_size);
    if (!gfx_data) {
        ti_Close(gfx_slot);
        return 1;
    }

    zx7_Decompress(gfx_data, ti_GetDataPtr(gfx_slot));
    ti_Close(gfx_slot);

    PYDWGFX_init(gfx_data);

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
    if (gfx_data) {
        free(gfx_data);
    }
    return 0;
}
