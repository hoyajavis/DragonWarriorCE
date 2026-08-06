#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdbool.h>

#define TILE_SIZE 16

// Map tile types
#define TILE_PLAIN    0
#define TILE_WATER    1
#define TILE_FOREST   2
#define TILE_MOUNTAIN 3
#define TILE_HILL     4
#define TILE_SWAMP    5
#define TILE_DESERT   6
#define TILE_STONE    7
#define TILE_BRIDGE   8
#define TILE_PATH     9
#define TILE_COUNTER  10
#define TILE_INN      11
#define TILE_DARKNESS 12
#define TILE_CHEST    13
#define TILE_ROOF     14
#define TILE_ARMOR    15
#define TILE_BARRIER  16
#define TILE_TRANSPARENT 17

#include <fileioc.h>

typedef struct {
    uint8_t trigger_x;
    uint8_t trigger_y;
    char target_map[8];
    uint8_t spawn_x;
    uint8_t spawn_y;
} MapTransition;

extern uint8_t *current_map_data;
extern uint8_t current_map_width;
extern uint8_t current_map_height;
extern bool current_map_is_outside;
extern uint8_t current_map_light_diameter;
extern uint8_t current_map_num_transitions;
extern MapTransition *current_map_transitions;

typedef enum {
    OBJ_NONE = 0,
    OBJ_NPC_KING,
    OBJ_NPC_PRINCESS,
    OBJ_NPC_GUARD,
    OBJ_NPC_SAGE,
    OBJ_NPC_MERCHANT,
    OBJ_NPC_WARRIOR,
    OBJ_NPC_BOY,
    OBJ_NPC_GIRL,
    OBJ_NPC_TRUMPETER,
    OBJ_CHEST,
    OBJ_DOOR,
    OBJ_STAIRS_UP,
    OBJ_STAIRS_DOWN,
    // Overworld decorations (rendered as sprites on top of terrain)
    OBJ_CASTLE_STONE_A,      // 14 - Tantegel
    OBJ_CASTLE_STONE_TALL_A, // 15 - Charlock
    OBJ_TOWN_STONE_MED_A,    // 16 - Garinham, Rimuldar, Cantlin
    OBJ_TOWN_STONE_MED_B,    // 17 - Brecconary
    OBJ_TOWN_WOOD_MED_A,     // 18 - Hauksness
    OBJ_TOWN_WOOD_MED_B,     // 19 - Kol
    OBJ_CAVE,                // 20 - Caves (Erdrick's Tomb, Mountain Cave, Swamp Caves)
    OBJ_SHRINE_STONE_A,      // 21 - Northern Shrine
    OBJ_SHRINE_STONE_B       // 22 - Southern Shrine
} ObjectType;

extern uint8_t current_map_num_interactables;
extern uint8_t current_map_interactables_raw[32 * 6];

extern uint8_t current_map_global_monster_set;
extern uint8_t current_map_num_monster_zones;
extern uint8_t current_map_monster_zones[16 * 5];

// Map Handling
void map_InitLUT(void);
bool map_Load(const char *appvar_name);



// Draws the visible portion of the map based on the absolute camera pixel coordinates
void map_Draw(uint16_t cameraX, uint16_t cameraY);

bool isPassable(uint16_t px, uint16_t py);

#endif
