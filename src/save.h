#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include <stdint.h>

bool save_Exists(uint8_t slotIndex);
void save_Game(uint8_t slotIndex);
bool load_Game(uint8_t slotIndex);

#endif
