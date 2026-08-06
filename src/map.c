#include "map.h"
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <fileioc.h>
#include "gfx/gfx.h"
#include "game.h"

uint8_t *current_map_data = NULL;
uint8_t current_map_width = 0;
uint8_t current_map_height = 0;
bool current_map_is_outside = true;
uint8_t current_map_light_diameter = 255;
uint8_t current_map_num_transitions = 0;
MapTransition *current_map_transitions = NULL;
static uint8_t current_map_slot = 0;

uint8_t current_map_num_interactables = 0;
uint8_t current_map_interactables_raw[32 * 6] = {0};

uint8_t current_map_global_monster_set = 255;
uint8_t current_map_num_monster_zones = 0;
uint8_t current_map_monster_zones[16 * 5] = {0};

bool map_Load(const char *appvar_name) {
    uint8_t slot;
    if (current_map_slot != 0) {
        ti_Close(current_map_slot);
        current_map_slot = 0;
    }
    
    slot = ti_Open(appvar_name, "r");
    if (!slot) return false;
    
    uint16_t file_size = ti_GetSize(slot);
    if (file_size < 4) { ti_Close(slot); return false; }
    
    current_map_slot = slot;
    
    uint8_t *data = (uint8_t*)ti_GetDataPtr(slot);
    uint8_t *ptr = data;
    
    #define MAP_REQUIRE_BYTES(n) \
        if ((uint16_t)(ptr - data + (n)) > file_size) { ti_Close(slot); current_map_slot = 0; return false; }
    
    current_map_width = *ptr++;
    current_map_height = *ptr++;
    current_map_is_outside = *ptr++ != 0;
    current_map_light_diameter = *ptr++;
    current_map_data = ptr;
    
    MAP_REQUIRE_BYTES(current_map_width * current_map_height + 1);
    ptr += current_map_width * current_map_height;
    
    current_map_num_transitions = *ptr++;
    
    if (current_map_num_transitions > 0) {
        MAP_REQUIRE_BYTES(current_map_num_transitions * sizeof(MapTransition));
        current_map_transitions = (MapTransition*)ptr;
        ptr += current_map_num_transitions * sizeof(MapTransition);
    } else {
        current_map_transitions = NULL;
    }
    
    // Read Interactables
    MAP_REQUIRE_BYTES(1);
    uint8_t raw_interactables_count = *ptr++;
    
    MAP_REQUIRE_BYTES(raw_interactables_count * 6);
    current_map_num_interactables = 0;
    
    // Handle lighting transition natively upon map load
    // This catches staircases, teleports, death, and Outside spell
    if (state.lightDiameter != 255 && current_map_light_diameter == 255) {
        state.lightDiameter = 255;
        state.lightDecaySteps = 0;
        state.lightDecayCounter = 0;
    } else if (state.lightDiameter == 255 && current_map_light_diameter != 255) {
        state.lightDiameter = current_map_light_diameter;
        state.lightDecaySteps = 0;
        state.lightDecayCounter = 0;
    }
    
    if (raw_interactables_count > 0) {
        uint8_t *obj = ptr;
        for (int i = 0; i < raw_interactables_count; i++, obj += 6) {
            if (current_map_num_interactables >= 32) break;
            
            uint8_t objType = obj[2];
            uint16_t actionId = obj[4] | (obj[5] << 8);
            
            // Check if this object is a chest or door that has already been opened
            bool already_opened = false;
            if (objType == OBJ_CHEST || objType == OBJ_DOOR) {
                if (actionId < 512 && check_flag(actionId)) {
                    already_opened = true;
                }
            }
            
            if (!already_opened) {
                memcpy(&current_map_interactables_raw[current_map_num_interactables * 6], obj, 6);
                current_map_num_interactables++;
            }
        }
    }
    
    ptr += raw_interactables_count * 6; // advance by what is in file
    
    // Read Encounter Data
    MAP_REQUIRE_BYTES(2);
    current_map_global_monster_set = *ptr++;
    current_map_num_monster_zones = *ptr++;
    
    MAP_REQUIRE_BYTES(current_map_num_monster_zones * 5);
    int num_to_copy = current_map_num_monster_zones;
    if (num_to_copy > 16) num_to_copy = 16;
    
    memcpy(current_map_monster_zones, ptr, num_to_copy * 5);
    ptr += current_map_num_monster_zones * 5;
    
    #undef MAP_REQUIRE_BYTES

    // DO NOT CALL ti_Close(slot) here! Wait, the original code had ti_Close(slot) removed or not?
    // Actually, earlier the file was left open or not? Let's check.
    // The previous code did NOT call ti_Close(slot) because the map appvar MUST stay open for tile rendering!
    // current_map_slot = slot;
    // so we must NOT close it!
    if (strcmp(appvar_name, "PYDW001") == 0) {
        if (check_flag(FLAG_PM_CREATED_RAINBOW_BRIDGE)) {
            current_map_data[50 * current_map_width + 65] = TILE_BRIDGE;
        } else {
            current_map_data[50 * current_map_width + 65] = TILE_WATER;
        }
    }
    
    return true;
}

static gfx_sprite_t *tile_lut[18];

// LUT for interactable/decoration object sprites, indexed by ObjectType
// OBJ_NONE(0)..OBJ_STAIRS_DOWN(13) are the dungeon objects
// OBJ_CASTLE_STONE_A(14)..OBJ_SHRINE_STONE_B(22) are overworld decorations
static gfx_sprite_t *obj_lut[23];

void map_InitLUT(void) {
    tile_lut[TILE_PLAIN]       = plain;
    tile_lut[TILE_WATER]       = water;
    tile_lut[TILE_FOREST]      = forest;
    tile_lut[TILE_MOUNTAIN]    = mountain;
    tile_lut[TILE_HILL]        = hill;
    tile_lut[TILE_SWAMP]       = swamp;
    tile_lut[TILE_DESERT]      = desert;
    tile_lut[TILE_STONE]       = stone;
    tile_lut[TILE_BRIDGE]      = bridge;
    tile_lut[TILE_PATH]        = path;
    tile_lut[TILE_COUNTER]     = counter;
    tile_lut[TILE_INN]         = inn;
    tile_lut[TILE_DARKNESS]    = darkness;
    tile_lut[TILE_CHEST]       = plain; // Chests are drawn separately as interactables
    tile_lut[TILE_ROOF]        = roof;
    tile_lut[TILE_ARMOR]       = armor;
    tile_lut[TILE_BARRIER]     = barrier;
    tile_lut[TILE_TRANSPARENT] = plain;

    // NPC and dungeon object sprites (indices 0..13)
    obj_lut[OBJ_NONE]          = NULL;
    obj_lut[OBJ_NPC_KING]      = king;
    obj_lut[OBJ_NPC_PRINCESS]  = princess;
    obj_lut[OBJ_NPC_GUARD]     = guard;
    obj_lut[OBJ_NPC_SAGE]      = sage;
    obj_lut[OBJ_NPC_MERCHANT]  = merchant;
    obj_lut[OBJ_NPC_WARRIOR]   = warrior;
    obj_lut[OBJ_NPC_BOY]       = boy;
    obj_lut[OBJ_NPC_GIRL]      = girl;
    obj_lut[OBJ_NPC_TRUMPETER] = trumpeter;
    obj_lut[OBJ_CHEST]         = chest;
    obj_lut[OBJ_DOOR]          = door;
    obj_lut[OBJ_STAIRS_UP]     = stairs_up;
    obj_lut[OBJ_STAIRS_DOWN]   = stairs_down;

    // Overworld decoration sprites (indices 14..22)
    obj_lut[OBJ_CASTLE_STONE_A]      = castle_stone_a;
    obj_lut[OBJ_CASTLE_STONE_TALL_A] = castle_stone_tall_a;
    obj_lut[OBJ_TOWN_STONE_MED_A]    = town_stone_med_a;
    obj_lut[OBJ_TOWN_STONE_MED_B]    = town_stone_med_b;
    obj_lut[OBJ_TOWN_WOOD_MED_A]     = town_wood_med_a;
    obj_lut[OBJ_TOWN_WOOD_MED_B]     = town_wood_med_b;
    obj_lut[OBJ_CAVE]                = cave;
    obj_lut[OBJ_SHRINE_STONE_A]      = shrine_stone_a;
    obj_lut[OBJ_SHRINE_STONE_B]      = shrine_stone_b;
}

void map_Draw(uint16_t cameraX, uint16_t cameraY) {
    if (!current_map_data) return;
    
    uint16_t startX = cameraX >> 4;
    uint16_t startY = cameraY >> 4;
    uint8_t offsetX = cameraX & 15;
    uint8_t offsetY = cameraY & 15;
    
    uint8_t active_diameter = state.lightDiameter > current_map_light_diameter ? state.lightDiameter : current_map_light_diameter;
    int radius = 0;
    int heroScreenX = 0;
    int heroScreenY = 0;
    
    if (active_diameter != 255) {
        radius = active_diameter << 3;
        heroScreenX = state.playerX - cameraX + 8;
        heroScreenY = state.playerY - cameraY + 8;
    }
    
    // Draw terrain tiles
    int drawY = -offsetY;
    uint8_t *rowPtr = current_map_data + (startY * current_map_width);
    for (int y = 0; y < 16; y++) { 
        int tileY = startY + y;
        if (tileY >= current_map_height) break;
        
        int drawX = -offsetX;
        uint8_t *tilePtr = rowPtr + startX;
        
        int dy = 0;
        if (active_diameter != 255) {
            int tileScreenY = drawY + 8;
            dy = tileScreenY - heroScreenY;
            if (dy < 0) dy = -dy;
        }

        for (int x = 0; x < 21; x++) { 
            int tileX = startX + x;
            if (tileX >= current_map_width) break;
            
            uint8_t tileId = *tilePtr;
            if (tileId > 17) tileId = TILE_PLAIN;
            
            if (active_diameter != 255) {
                int tileScreenX = drawX + 8;
                int dx = tileScreenX - heroScreenX;
                if (dx < 0) dx = -dx;
                
                if (dx > radius || dy > radius) {
                    if (drawX >= 0 && drawX <= 304 && drawY >= 0 && drawY <= 224) {
                        gfx_SetColor(0); // Black
                        gfx_FillRectangle(drawX, drawY, 16, 16);
                    }
                    drawX += 16;
                    tilePtr++;
                    continue;
                }
            }
            
            if (drawX >= 0 && drawX <= 304 && drawY >= 0 && drawY <= 224) {
                gfx_Sprite_NoClip(tile_lut[tileId], drawX, drawY);
            } else {
                gfx_Sprite(tile_lut[tileId], drawX, drawY);
            }
            
            tilePtr++;
            drawX += 16;
        }
        drawY += 16;
        rowPtr += current_map_width;
    }

    // Draw interactables (NPCs, chests, doors, stairs, overworld decorations) on top
    uint8_t *obj = current_map_interactables_raw;
    for (int i = 0; i < current_map_num_interactables; i++, obj += 6) {
        uint8_t objX    = obj[0];
        uint8_t objY    = obj[1];
        uint8_t objType = obj[2];

        // Cull to visible viewport
        if (objX < startX || objX >= (startX + 21)) continue;
        if (objY < startY || objY >= (startY + 16)) continue;
        if (objType == OBJ_NONE || objType >= 23) continue;

        gfx_sprite_t *spr = obj_lut[objType];
        if (!spr) continue;

        int screenX = (((int)objX - (int)startX) << 4) - offsetX;
        int screenY = (((int)objY - (int)startY) << 4) - offsetY;

        if (screenX >= 0 && screenX <= 304 && screenY >= 0 && screenY <= 224) {
            gfx_Sprite_NoClip(spr, screenX, screenY);
        } else {
            gfx_Sprite(spr, screenX, screenY);
        }
    }
}

bool isPassable(uint16_t px, uint16_t py) {
    uint16_t tx = px >> 4;
    uint16_t ty = py >> 4;
    if (tx >= current_map_width || ty >= current_map_height) return false;
    
    // Check objects first
    uint8_t *obj = current_map_interactables_raw;
    for (uint8_t i = 0; i < current_map_num_interactables; i++, obj += 6) {
        uint16_t objX = obj[0];
        uint16_t objY = obj[1];
        if (objX == tx && objY == ty) {
            uint8_t objType = obj[2];
            // Can't walk on NPCs, Chests, or Doors
            if ((objType >= OBJ_NPC_KING && objType <= OBJ_NPC_TRUMPETER) || 
                objType == OBJ_CHEST || 
                objType == OBJ_DOOR) {
                return false;
            }
        }
    }
    
    uint8_t tile = current_map_data[ty * current_map_width + tx];
    return (tile != TILE_WATER && tile != TILE_MOUNTAIN && tile != TILE_STONE && tile != TILE_COUNTER && tile != TILE_ROOF && tile != TILE_INN && tile != TILE_ARMOR && tile != TILE_DARKNESS);
}
