#include "save.h"
#include "game.h"
#include "map.h"
#include "ui.h"
#include "randomizer.h"
#include <fileioc.h>
#include <graphx.h>
#include <string.h>

#define SAVE_MAGIC 0x4457

// Single static SaveFile to avoid ~1,132 bytes of stack allocation per call
static SaveFile sf;

static uint16_t compute_checksum(const SaveFile *sfp) {
    uint16_t sum = 0;
    const uint8_t *ptr = (const uint8_t*)sfp;
    // Checksum over magic + slots (excluding checksum field at end)
    size_t len = sizeof(SaveFile) - sizeof(uint16_t);
    for (size_t i = 0; i < len; i++) {
        sum = (sum << 1) + ptr[i];
    }
    return sum;
}

bool save_Exists(uint8_t slotIndex) {
    if (slotIndex >= 3) return false;
    ti_var_t file = ti_Open("PYDWSAVE", "r");
    if (file) {
        if (ti_Read(&sf, sizeof(SaveFile), 1, file) == 1) {
            ti_Close(file);
            if (sf.magic == SAVE_MAGIC && sf.checksum == compute_checksum(&sf)) {
                return sf.slots[slotIndex].slotInUse;
            }
        } else {
            ti_Close(file);
        }
    }
    return false;
}

void save_Game(uint8_t slotIndex) {
    if (slotIndex >= 3) return;
    
    memset(&sf, 0, sizeof(SaveFile));
    ti_var_t file = ti_Open("PYDWSAVE", "r");
    if (file) {
        ti_Read(&sf, sizeof(SaveFile), 1, file);
        ti_Close(file);
    }
    
    sf.magic = SAVE_MAGIC;
    sf.slots[slotIndex].slotInUse = true;
    memcpy(&sf.slots[slotIndex].state, &state, sizeof(GameState));
    memcpy(&sf.slots[slotIndex].savedHero, &heroStats, sizeof(EntityStats));
    sf.checksum = compute_checksum(&sf);
    
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
        if (ti_Read(&sf, sizeof(SaveFile), 1, file) == 1) {
            ti_Close(file);
            if (sf.magic == SAVE_MAGIC && sf.checksum == compute_checksum(&sf)) {
                if (sf.slots[slotIndex].slotInUse) {
                    memcpy(&state, &sf.slots[slotIndex].state, sizeof(GameState));
                    memcpy(&heroStats, &sf.slots[slotIndex].savedHero, sizeof(EntityStats));
                    
                    // Rebuild dynamic randomizer RAM tables for the loaded seed!
                    init_randomizer(state.seed, state.randoFlags);
                    
                    markStatsDirty();
                    map_Load(state.currentMapName);
                    return true;
                }
            }
        } else {
            ti_Close(file);
        }
    }
    return false;
}

