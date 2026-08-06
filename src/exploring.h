#ifndef EXPLORING_H
#define EXPLORING_H

#include <stdbool.h>
#include <stdint.h>

extern bool isMoving;
extern uint8_t moveFrames;
extern int8_t moveStepX;
extern int8_t moveStepY;
extern bool needsFadeIn;
extern uint8_t inputDelay;

void updateExploring(void);
void renderExploring(void);

#endif // EXPLORING_H
