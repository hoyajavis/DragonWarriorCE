#include "save.h"
#include "game.h"
#include "map.h"
#include "ui.h"
#include <fileioc.h>
#include <graphx.h>
#include <string.h>

bool save_Exists(uint8_t slotIndex) {
    if (slotIndex >= 3) return false;
    ti_var_t file = ti_Open("PYDWSAVE", "r");
    if (file) {
        SaveFile sf;
        ti_Read(&sf, sizeof(SaveFile), 1, file);
        ti_Close(file);
        return sf.slots[slotIndex].slotInUse;
    }
    return false;
}

void save_Game(uint8_t slotIndex) {
    if (slotIndex >= 3) return;
    
    SaveFile sf = {0};
    ti_var_t file = ti_Open("PYDWSAVE", "r");
    if (file) {
        ti_Read(&sf, sizeof(SaveFile), 1, file);
        ti_Close(file);
    }
    
    gfx_SetTextBGColor(0x00);
    gfx_SetTextTransparentColor(0x00);
    gfx_PrintStringXY("Saving Data...", 20, 100);
    gfx_SwapDraw();
    
    sf.slots[slotIndex].slotInUse = true;
    memcpy(&sf.slots[slotIndex].state, &state, sizeof(GameState));
    memcpy(&sf.slots[slotIndex].savedHero, &heroStats, sizeof(EntityStats));
    
    file = ti_Open("PYDWSAVE", "w");
    if (file) {
        ti_Write(&sf, sizeof(SaveFile), 1, file);
        ti_Close(file);
    }
}

bool load_Game(uint8_t slotIndex) {
    if (slotIndex >= 3) return false;
    ti_var_t file = ti_Open("PYDWSAVE", "r");
    if (file) {
        SaveFile sf;
        ti_Read(&sf, sizeof(SaveFile), 1, file);
        ti_Close(file);
        
        if (sf.slots[slotIndex].slotInUse) {
            memcpy(&state, &sf.slots[slotIndex].state, sizeof(GameState));
            memcpy(&heroStats, &sf.slots[slotIndex].savedHero, sizeof(EntityStats));
            markStatsDirty();
            map_Load(state.currentMapName);
            return true;
        }
    }
    return false;
}
