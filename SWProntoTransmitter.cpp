#include "SWProntoTransmitter.h"

static SWProntoTransmitter* pt;

void callback()
{
    pt->ir_cycle_count++;
    pt->ir_pulse();

    if (pt->ir_cycle_count == pt->current_len)
    {
        pt->ir_toggle();
        pt->ir_cycle_count = 0;

        pt->ir_seq_index++;
        pt->current_len = pt->ir_code[pt->ir_seq_index] << 1;

        if (pt->ir_seq_index >= pt->sequenceIndexEnd)
            pt->ir_stop();
    }
}

void SWProntoTransmitter::ir_on()
{
    ir_led_state = HIGH;
    digitalWrite(this->pin, this->ir_led_state);
}

void SWProntoTransmitter::ir_off()
{
    ir_led_state = LOW;
    digitalWrite(this->pin, this->ir_led_state);
}

void SWProntoTransmitter::ir_toggle()
{
    this->mask = !this->mask;
}

inline void SWProntoTransmitter::ir_pulse()
{
    digitalWrite(this->pin, this->ir_led_state & this->mask);
    this->ir_led_state = !this->ir_led_state;
}

void SWProntoTransmitter::ir_stop()
{
    this->ir_off();
    Timer1.stop();
    this->stat = waiting;
}

void SWProntoTransmitter::ir_start(uint16_t *code)
{
    this->ir_code = code;
    double frq = 1000000 / (code[PRONTO_FREQ_CODE] * 0.241246);
    int period_ms = ((1 / frq) * (1000000/2));

    // Taken from ISR (Callback)
    this->sequenceIndexEnd = PRONTO_CODE_START +
                       (this->ir_code[PRONTO_SEQUENCE1_LENGTH] << 1) +
                       (this->ir_code[PRONTO_SEQUENCE2_LENGTH] << 1);

    this->repeatSequenceIndexStart = PRONTO_CODE_START +
                               (this->ir_code[PRONTO_SEQUENCE1_LENGTH] << 1);

    this->ir_seq_index = PRONTO_CODE_START;
    this->current_len = this->ir_code[this->ir_seq_index] << 1;
    this->ir_cycle_count = 0;
    this->ir_on();

    pt = this;    
    
    Timer1.initialize(period_ms);
    Timer1.attachInterrupt(callback); // attaches callback() as a timer overflow interrupt

    this->stat = transmitting;
}

void SWProntoTransmitter::transmit_blocking(uint16_t *code)
{
    this->ir_start(code);

    while (this->stat == transmitting) {
        __asm__ __volatile__ ("nop\n\t");
    }
}

void SWProntoTransmitter::convert_raw_pronto(uint16_t *code_raw, uint16_t len, uint16_t fr_hz, uint16_t* code) {
    code[0] = 0x0000;
    // Freq=1000000/(103*. 241246)= 40,244 
    code[1] = 1000000/(0.241246*fr_hz);
    // For convenience burst pair sequence one length is supposed to be 0 
    code[2] = 0x0000;
    code[3] = len >> 1;

    double period_us = 1000000.0/fr_hz;
    for (int i = 1; i < len; i++) 
      code[3+i] = int(((int)code_raw[i] * USECPERTICK_IRremote)/period_us);

    code[len+3] = PRONTO_DEFAULT_GAP/period_us;
}

void SWProntoTransmitter::transmit_raw_blocking(uint16_t *code_raw, uint16_t len, uint16_t fr_hz)
{
    uint16_t code[len+4];
    this->convert_raw_pronto(code_raw, len, fr_hz, code);
    
    this->ir_start(code);

    while (this->stat == transmitting) {
        __asm__ __volatile__ ("nop\n\t");
    }
}

SWProntoTransmitter::SWProntoTransmitter(uint8_t pin)
{
    pinMode(pin, OUTPUT);
    this->pin = pin;
}

SWProntoTransmitter::~SWProntoTransmitter()
{
}
