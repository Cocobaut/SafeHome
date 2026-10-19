#ifndef INC_SECURITYCONTROL_H_
#define INC_SECURITYCONTROL_H_

#include "globals.h"

extern void initSecurity();
extern void loopSecurity();

// Actions
extern void securityRequestUnlock();
extern void securityResetAlarms();
extern void securitySetManualBuzzer(bool on);
extern void securitySetManualRelay(bool on);
extern void securityAdminServo(uint8_t index, bool open);

// State
extern bool securityUnlockActive();
extern unsigned long securityUnlockRemainingMs();
extern bool securityGasEmergency();
extern bool securityDoorAlarm();
extern bool securityUltrasonicAlarm(uint8_t index);
extern bool securityUltrasonicSuppressed();

#endif /* INC_SECURITYCONTROL_H_ */
