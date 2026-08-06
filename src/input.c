#include "input.h"

kb_key_t prev_key1 = 0;
kb_key_t prev_key2 = 0;
kb_key_t prev_key3 = 0;
kb_key_t prev_key4 = 0;
kb_key_t prev_key5 = 0;
kb_key_t prev_key6 = 0;
kb_key_t prev_key7 = 0;

void input_UpdatePrevKeys(void) {
    prev_key1 = kb_Data[1];
    prev_key2 = kb_Data[2];
    prev_key3 = kb_Data[3];
    prev_key4 = kb_Data[4];
    prev_key5 = kb_Data[5];
    prev_key6 = kb_Data[6];
    prev_key7 = kb_Data[7];
}
