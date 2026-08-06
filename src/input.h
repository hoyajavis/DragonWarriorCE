#ifndef INPUT_H
#define INPUT_H

#include <keypadc.h>

extern kb_key_t prev_key1;
extern kb_key_t prev_key2;
extern kb_key_t prev_key3;
extern kb_key_t prev_key4;
extern kb_key_t prev_key5;
extern kb_key_t prev_key6;
extern kb_key_t prev_key7;

// Standardized Keypad Macros
#define BTN_CONFIRM (((kb_Data[1] & kb_2nd) && !(prev_key1 & kb_2nd)) || ((kb_Data[6] & kb_Enter) && !(prev_key6 & kb_Enter)))
#define BTN_CANCEL (((kb_Data[2] & kb_Alpha) && !(prev_key2 & kb_Alpha)) || ((kb_Data[6] & kb_Clear) && !(prev_key6 & kb_Clear)))
#define BTN_UP ((kb_Data[7] & kb_Up) && !(prev_key7 & kb_Up))
#define BTN_DOWN ((kb_Data[7] & kb_Down) && !(prev_key7 & kb_Down))
#define BTN_LEFT ((kb_Data[7] & kb_Left) && !(prev_key7 & kb_Left))
#define BTN_RIGHT ((kb_Data[7] & kb_Right) && !(prev_key7 & kb_Right))
#define BTN_UP_HELD (kb_Data[7] & kb_Up)
#define BTN_DOWN_HELD (kb_Data[7] & kb_Down)
#define BTN_LEFT_HELD (kb_Data[7] & kb_Left)
#define BTN_RIGHT_HELD (kb_Data[7] & kb_Right)

void input_UpdatePrevKeys(void);

#endif // INPUT_H
