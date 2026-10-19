#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "globals.h"

enum BuzzerMode {
    BUZZER_OFF,
    BUZZER_CONTINUOUS,  // Keu lien tuc (gas / bat thu cong)
    BUZZER_BEEP         // Keu ngat quang (xam nhap)
};

extern void initBuzzer();
extern void loopBuzzer();
extern void buzzerSetMode(BuzzerMode mode);
extern bool buzzerIsOn();

#endif /* INC_BUZZER_H_ */
