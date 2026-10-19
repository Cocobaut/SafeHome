#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "globals.h"

#define BUTTON_COUNT 1
#define BUTTON_OPEN  0  // D9  - Nut mo cua tu ben trong

extern void initButtons();
extern void loopButtons();
extern bool buttonConsumeShortPress(uint8_t index);
extern bool buttonConsumeLongPress(uint8_t index);

#endif /* INC_BUTTON_H_ */
