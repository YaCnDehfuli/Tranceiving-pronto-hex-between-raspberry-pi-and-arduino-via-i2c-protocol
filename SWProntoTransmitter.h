#ifndef __SW_PRONTO_TRANSMITTER__
#define __SW_PRONTO_TRANSMITTER__

#include <TimerOne.h>

// Include IRremote library resulted in errors so
// Macro that was needed is redefined
#define USECPERTICK_IRremote 50

#define PRONTO_IR_SOURCE 0 // Pronto code byte 0
#define PRONTO_FREQ_CODE 1 // Pronto code byte 1
#define PRONTO_SEQUENCE1_LENGTH 2 // Pronto code byte 2
#define PRONTO_SEQUENCE2_LENGTH 3 // Pronto code byte 3
#define PRONTO_CODE_START 4 // Pronto code byte 4
#define PRONTO_DEFAULT_GAP 45000 // micros

enum status {
  waiting,
  transmitting
};

class SWProntoTransmitter
{
    public:
        const uint16_t *ir_code = NULL;
        uint16_t ir_cycle_count = 0;
        uint32_t ir_total_cycle_count = 0;
        uint8_t ir_seq_index = 0;
        uint8_t ir_led_state = LOW;
        uint8_t mask = HIGH;
        int Repeat_Value = 3;

        // Taken from ISR (callback)
        uint16_t sequenceIndexEnd;
        uint16_t repeatSequenceIndexStart;

        // Used to reduce overhead in ISR
        uint16_t current_len = 0;

        uint8_t pin;

        volatile status stat = waiting;
        
        void ir_on();
        void ir_off();
        void ir_toggle();
        inline void ir_pulse();

        void ir_start(uint16_t *code);
        void ir_stop();

        void convert_raw_pronto(uint16_t *code_raw, uint16_t len, uint16_t fr_hz, uint16_t* code);
        void transmit_blocking(uint16_t *code);
        void transmit_raw_blocking(uint16_t *code, uint16_t len, uint16_t fr_hz);
        SWProntoTransmitter(uint8_t pin);
        ~SWProntoTransmitter();
};

#endif
