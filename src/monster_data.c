#include "monster_data.h"
#include "gfx/gfx.h"

const MonsterDef monsterTable[41] = {
    {"Stopper", 3, 3, 5, 1, 3, 1, 1, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Slime", 3, 3, 5, 1, 3, 1, 1, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Red Slime", 4, 4, 7, 1, 3, 1, 2, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Drakee", 5, 6, 9, 3, 6, 2, 2, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Ghost", 6, 7, 11, 4, 8, 3, 3, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Magician", 10, 13, 11, 6, 12, 4, 9, true, 0, 0, 0, 16, SPELL_HURT, 50, SPELL_NONE, 0, 0, NULL},
    {"Magidrakee", 12, 15, 14, 7, 14, 5, 9, true, 0, 0, 0, 16, SPELL_HURT, 50, SPELL_NONE, 0, 0, NULL},
    {"Scorpion", 16, 20, 18, 8, 16, 6, 12, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Druin", 17, 22, 20, 9, 18, 7, 12, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Poltergeist", 18, 23, 18, 10, 20, 8, 13, true, 0, 0, 0, 16, SPELL_HURT, 75, SPELL_NONE, 0, 0, NULL},
    {"Droll", 19, 25, 24, 12, 24, 10, 18, true, 0, 87, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Drakeema", 16, 20, 22, 13, 26, 11, 15, true, 12, 0, 0, 16, SPELL_HEAL, 25, SPELL_HURT, 50, 25, NULL},
    {"Skeleton", 23, 30, 28, 11, 22, 11, 22, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Warlock", 23, 30, 28, 11, 22, 13, 26, true, 18, 6, 0, 16, SPELL_SLEEP, 25, SPELL_HURT, 50, 0, NULL},
    {"Metal Scorpion", 17, 22, 36, 21, 42, 14, 30, true, 0, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Wolf", 26, 34, 40, 15, 30, 16, 37, true, 6, 93, 0, 16, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Wraith", 28, 36, 44, 17, 34, 17, 45, true, 43, 0, 0, 16, SPELL_HEAL, 25, SPELL_HURT, 50, 25, NULL},
    {"Metal Slime", 4, 4, 10, 127, 255, 115, 4, true, 93, 93, 93, 16, SPELL_HURT, 75, SPELL_NONE, 0, 0, NULL},
    {"Specter", 28, 36, 40, 19, 38, 18, 52, true, 18, 6, 0, 16, SPELL_SLEEP, 25, SPELL_HURT, 75, 0, NULL},
    {"Wolflord", 29, 38, 50, 18, 36, 20, 60, true, 25, 43, 0, 16, SPELL_STOPSPELL, 50, SPELL_NONE, 0, 0, NULL},
    {"Druinlord", 27, 35, 47, 20, 40, 20, 63, true, 93, 0, 0, 16, SPELL_HEAL, 75, SPELL_HURT, 25, 25, NULL},
    {"Drollmagi", 29, 38, 52, 25, 50, 22, 67, true, 12, 12, 0, 24, SPELL_STOPSPELL, 50, SPELL_NONE, 0, 0, NULL},
    {"Wyvern", 32, 42, 56, 24, 48, 24, 75, true, 25, 93, 0, 24, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Rogue Scorpion", 27, 35, 60, 45, 90, 26, 82, true, 43, 93, 0, 24, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Wraith Knight", 35, 46, 68, 28, 56, 28, 90, true, 31, 0, 18, 24, SPELL_HEAL, 75, SPELL_NONE, 0, 25, NULL},
    {"Golem", 53, 70, 120, 30, 60, 5, 7, true, 93, 93, 93, 24, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Goldman", 38, 50, 48, 20, 40, 6, 150, true, 81, 93, 0, 24, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Knight", 42, 55, 76, 39, 78, 33, 97, true, 37, 43, 0, 24, SPELL_STOPSPELL, 50, SPELL_NONE, 0, 0, NULL},
    {"Magiwyvern", 44, 58, 78, 34, 68, 34, 105, true, 12, 0, 0, 24, SPELL_SLEEP, 50, SPELL_NONE, 0, 0, NULL},
    {"Demon Knight", 38, 50, 79, 32, 64, 37, 112, true, 93, 93, 93, 24, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Werewolf", 46, 60, 86, 35, 70, 40, 116, true, 43, 93, 0, 24, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Green Dragon", 49, 65, 88, 37, 74, 45, 120, true, 43, 93, 12, 32, SPELL_BREATH_FIRE, 25, SPELL_NONE, 0, 0, NULL},
    {"Starwyvern", 49, 65, 86, 40, 80, 43, 120, true, 50, 0, 6, 32, SPELL_HEALMORE, 75, SPELL_BREATH_FIRE, 25, 25, NULL},
    {"Wizard", 49, 65, 80, 35, 70, 50, 123, true, 93, 43, 93, 32, SPELL_HURTMORE, 50, SPELL_NONE, 0, 0, NULL},
    {"Axe Knight", 53, 70, 94, 41, 82, 54, 123, true, 93, 18, 6, 32, SPELL_SLEEP, 25, SPELL_NONE, 0, 0, NULL},
    {"Blue Dragon", 53, 70, 98, 42, 84, 60, 112, true, 93, 93, 43, 32, SPELL_BREATH_FIRE, 25, SPELL_NONE, 0, 0, NULL},
    {"Stoneman", 121, 160, 100, 20, 40, 65, 105, true, 12, 93, 6, 64, SPELL_NONE, 0, SPELL_NONE, 0, 0, NULL},
    {"Armored Knight", 68, 90, 105, 43, 86, 70, 105, true, 93, 43, 6, 64, SPELL_HEALMORE, 75, SPELL_HURTMORE, 25, 25, NULL},
    {"Red Dragon", 76, 100, 120, 45, 90, 100, 105, true, 93, 43, 93, 64, SPELL_SLEEP, 25, SPELL_BREATH_FIRE, 25, 0, NULL},
    {"Dragonlord", 76, 100, 90, 37, 75, 0, 0, false, 93, 93, 93, 64, SPELL_STOPSPELL, 25, SPELL_HURTMORE, 75, 0, NULL},
    {"Dragonlord's True Form", 130, 130, 140, 100, 200, 0, 0, false, 93, 93, 93, 64, SPELL_BREATH_STRONG_FIRE, 50, SPELL_NONE, 0, 0, NULL},
};

gfx_sprite_t *get_monster_sprite(uint8_t monster_id) {
    switch (monster_id) {
        case MONSTER_STOPPER: return stopper;
        case MONSTER_SLIME: return slime;
        case MONSTER_RED_SLIME: return red_slime;
        case MONSTER_DRAKEE: return drakee;
        case MONSTER_GHOST: return ghost;
        case MONSTER_MAGICIAN: return magician;
        case MONSTER_MAGIDRAKEE: return magidrakee;
        case MONSTER_SCORPION: return scorpion;
        case MONSTER_DRUIN: return druin;
        case MONSTER_POLTERGEIST: return poltergeist;
        case MONSTER_DROLL: return droll;
        case MONSTER_DRAKEEMA: return drakeema;
        case MONSTER_SKELETON: return skeleton;
        case MONSTER_WARLOCK: return warlock;
        case MONSTER_METAL_SCORPION: return metal_scorpion;
        case MONSTER_WOLF: return wolf;
        case MONSTER_WRAITH: return wraith;
        case MONSTER_METAL_SLIME: return metal_slime;
        case MONSTER_SPECTER: return specter;
        case MONSTER_WOLFLORD: return wolflord;
        case MONSTER_DRUINLORD: return druinlord;
        case MONSTER_DROLLMAGI: return drollmagi;
        case MONSTER_WYVERN: return wyvern;
        case MONSTER_ROGUE_SCORPION: return rogue_scorpion;
        case MONSTER_WRAITH_KNIGHT: return wraith_knight;
        case MONSTER_GOLEM: return golem;
        case MONSTER_GOLDMAN: return goldman;
        case MONSTER_KNIGHT: return knight;
        case MONSTER_MAGIWYVERN: return magiwyvern;
        case MONSTER_DEMON_KNIGHT: return demon_knight;
        case MONSTER_WEREWOLF: return werewolf;
        case MONSTER_GREEN_DRAGON: return green_dragon;
        case MONSTER_STARWYVERN: return starwyvern;
        case MONSTER_WIZARD: return wizard;
        case MONSTER_AXE_KNIGHT: return axe_knight;
        case MONSTER_BLUE_DRAGON: return blue_dragon;
        case MONSTER_STONEMAN: return stoneman;
        case MONSTER_ARMORED_KNIGHT: return armored_knight;
        case MONSTER_RED_DRAGON: return red_dragon;
        case MONSTER_DRAGONLORD: return dragonlord_first_form;
        case MONSTER_DRAGONLORDS_TRUE_FORM: return dragonlord_second_form;
        default: return NULL;
    }
}

const uint8_t monsterSets[20][10] = {
    {1, 1, 2, 2, 2, 0, 0, 0, 0, 255},
    {1, 3, 2, 2, 2, 255, 255, 255, 255, 255},
    {1, 2, 3, 4, 4, 255, 255, 255, 255, 255},
    {2, 2, 3, 4, 5, 255, 255, 255, 255, 255},
    {4, 5, 6, 6, 7, 255, 255, 255, 255, 255},
    {4, 5, 6, 7, 12, 255, 255, 255, 255, 255},
    {6, 7, 12, 13, 15, 255, 255, 255, 255, 255},
    {12, 13, 14, 15, 15, 255, 255, 255, 255, 255},
    {14, 16, 19, 19, 26, 255, 255, 255, 255, 255},
    {16, 19, 26, 22, 22, 255, 255, 255, 255, 255},
    {22, 23, 24, 27, 29, 255, 255, 255, 255, 255},
    {24, 27, 29, 28, 17, 255, 255, 255, 255, 255},
    {27, 29, 28, 30, 32, 255, 255, 255, 255, 255},
    {30, 32, 32, 31, 33, 255, 255, 255, 255, 255},
    {9, 10, 11, 12, 13, 255, 255, 255, 255, 255},
    {18, 19, 20, 21, 24, 255, 255, 255, 255, 255},
    {30, 31, 32, 33, 34, 255, 255, 255, 255, 255},
    {33, 34, 35, 35, 36, 255, 255, 255, 255, 255},
    {33, 36, 37, 37, 38, 255, 255, 255, 255, 255},
    {4, 5, 7, 8, 8, 255, 255, 255, 255, 255},
};
