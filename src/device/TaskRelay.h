#ifndef INC_RELAY_H_
#define INC_RELAY_H_

#include "globals.h"

extern void initRelay();
extern void relayWrite(bool on);
extern bool relayIsOn();

#endif /* INC_RELAY_H_ */
