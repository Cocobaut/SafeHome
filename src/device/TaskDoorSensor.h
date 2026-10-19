#ifndef INC_DOORSENSOR_H_
#define INC_DOORSENSOR_H_

#include "globals.h"

extern void initDoorSensor();
extern void loopDoorSensor();
extern bool doorIsOpen();
extern bool doorConsumeOpenedEvent();

#endif /* INC_DOORSENSOR_H_ */
