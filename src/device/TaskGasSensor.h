#ifndef INC_GASSENSOR_H_
#define INC_GASSENSOR_H_

#include "globals.h"

extern void initGasSensor();
extern void loopGasSensor();
extern int gasRawValue();
extern int gasFilteredValue();
extern bool gasWarmingUp();
extern bool gasOverThreshold();

#endif /* INC_GASSENSOR_H_ */
