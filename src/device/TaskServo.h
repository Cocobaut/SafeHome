#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "globals.h"

extern void initServo();
extern void loopServo();
extern void servoSet(uint8_t index, bool open, bool force = false);
extern void servoSetAll(bool open);
extern bool servoIsOpen(uint8_t index);
extern bool servoIsMoving(uint8_t index);
extern bool servoAnyOpen();

#endif /* INC_SERVO_H_ */
