#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "globals.h"

#define ULTRASONIC_COUNT 2

extern void initUltrasonic();
extern void loopUltrasonic();
extern void ultrasonicRecalibrate();
extern bool ultrasonicCalibrating();
extern float ultrasonicDistance(uint8_t index);
extern float ultrasonicBaseline(uint8_t index);
extern bool ultrasonicDetected(uint8_t index);

#endif /* INC_ULTRASONIC_H_ */
