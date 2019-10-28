#include <stdbool.h>
#include <stdint.h>
#include "TimerTestsSrc.c"

bool test_reset_and_start_timer(uint8_t TIMSK_value, uint8_t TCNT1_value) {
    TIMSK1 = TIMSK_value;
    TCNT1 = TCNT1_value;

    reset_and_start_timer();

    if (TIMSK1 % 2 != 0 && TCNT1 == 0)
        return true;
    else
        return false;
}

bool test_reset_timer(uint32_t set_overflow_count, uint32_t set_elapsed_time, uint8_t set_TCNT1) {
    overflow_count = set_overflow_count;
    elapsed_time = set_elapsed_time;
    TCNT1 = set_TCNT1;

    reset_timer();

    if (overflow_count == 0 && elapsed_time == 0 && TCNT1 == 0)
        return true;
}

bool test_setup_timer(uint8_t TCC1A_register, uint8_t TCC1B_register){
    TCCR1A = TCC1A_register;
    TCCR1B = TCC1B_register;

    setup_timer();

    if (TCCR1A == 0 && (TCCR1B & 2) != 0)
        return true;
    return false;
}

bool Test_micros(uint8_t TCNT1_value) {
    TCNT1 = TCNT1_value;
    if (micros() == (TCNT1 + overflow_count * 65536)/ 2)
        return true;
    return false;
}

bool Test_init_millis(uint8_t TCCR3B_register, uint16_t OCR3AH_register, uint16_t OCR3AL_register, uint8_t TIMSK3_register) {
    TCCR3B = TCCR3B_register;
    OCR3AH = OCR3AH_register;
    OCR3AL = OCR3AL_register;
    TIMSK3 = TIMSK3_register;

    init_millis();

    if (((TCCR3B & 10) != 0) && OCR3AH == (2000 >> 8) && OCR3AL == 2000 && (TIMSK3 & 3) != 0 )
        return true;
    return false;
}

bool Test_set_message_to_LED(Adafruit_NeoPixel *p, uint8_t numLEDs, uint8_t bright, uint8_t *pixelArray) {
    p->numLEDs = numLEDs;
    p->brightness = bright;
    p->pixels = pixelArray;
    unsigned char payload[] = {3,0,4,7,65,27,13,48,37,55,79,41,254};

    setMessageToLED(p, payload, sizeof(payload)/3);


    if(p->numLEDs == numLEDs && p->brightness == bright)
        return true;
    return false;
}
