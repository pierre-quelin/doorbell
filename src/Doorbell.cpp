// Do not remove the include below
#include "Doorbell.h"
#include "ManchesterAsym.h"

#include <TimerOne.h>


#define EMITTER_POWER_PIN PB0
#define TX_PIN PB1
#define BUTTON_PIN PB2

/**
 * Interrupt routine
 */
ISR(PCINT0_vect)
{
    // Not used
}

/**
 * Reduced consumption to a minimum.
 */
void reducePower()
{
    // * Disable ADC
    ADCSRA &= ~_BV(ADEN);
    // * Disable Analog Comparator
    ACSR  |= _BV(ACD);
    // * Disable Brown-out Detector
    // TODO - Don't - Not secure
    // * Disable Internal Voltage Reference
    // TODO - needed by the Brown-out Detection
    // * Disable Watchdog Timer
    WDTCR &= ~_BV(WDE);
    // * Disable Port Pins
    // The simplest method to ensure a defined level of an unused pin,
    // is to enable the internal pull-up.
    // All pins as INPUTs
    DDRB &= ~(_BV(PB5) | _BV(PB4) | _BV(PB3) | _BV(PB2) | _BV(PB1) | _BV(PB0));
    // Pull-up Disable = 0
    MCUCR &= ~_BV(PUD);
    // Enable all Pull-up resistor
    PORTB |= _BV(PB5) | _BV(PB4) | _BV(PB3) | _BV(PB2) | _BV(PB1) | _BV(PB0);

    // * Power Reduction Register
    // PRTIM1: Power Reduction Timer/Counter1
    // PRTIM0: Power Reduction Timer/Counter0
    // PRUSI: Power Reduction USI (Universal Serial Interface)
    // PRADC: Power Reduction ADC
    PRR |= _BV(PRTIM1) | _BV(PRTIM0) | _BV(PRUSI) | _BV(PRADC);

    // MCUCR - MCU Control Register
    // Select Power-down sleep mode (SM1=1 SM0=0)
    // TODO - à optimiser
    MCUCR &= ~_BV(SM0);
    MCUCR |= _BV(SM1);

    // System Clock Prescaler
    // Not needed - Power-down sleep mode halts all generated clocks, allowing
    // operation of asynchronous modules only.
    // 6.5.2 CLKPR – Clock Prescale Register
    // Change back to 1 MHz by changing clock prescaler to 8 (8 MHz/8)
    // CLKPR = _BV(CLKPCE); // Prescaler enable
    // CLKPR = /*_BV(CLKPS3) | _BV(CLKPS2) |*/ _BV(CLKPS1) | _BV(CLKPS0); // Clock division factor 8 (0011)
}

/**
 * Enable the external pin PB2 interrupt
 */
void enablePinPB2Interrupt()
{
    // PB2/INT0 as input with pull-up
    // Already done see reducePower()

    // Soluce 1 - Interrupt on low level
    // ISC01=0 ISC00=0
    // MCUCR &= ~(_BV(ISC01) | _BV(ISC00));

    // Soluce 2 - Interrupt on falling edge
    // ISC01=1 ISC00=0
    MCUCR |= _BV(ISC01);
    MCUCR &= ~_BV(ISC00);

    // Soluce 3 - Any logical change
    // ISC01=0 ISC00=1
    // MCUCR &= ~_BV(ISC01);
    // MCUCR |= _BV(ISC00);

    // General Interrupt Mask Register - Pin change interrupt Enable
    GIMSK |= _BV(PCIE);
    // Pin Change Interrupt to 2nd pin PB2
    PCMSK |= _BV(PCINT2);
}

/**
 * Enable Power-down sleep mode
 */
void powerDown()
{
    // Power-down sleep mode SM1=1 SM0=0
    // Already done see reducePower()

    // Enabling sleep enable bit
    MCUCR |= _BV(SE);
    // Sleep instruction to put controller to sleep
    __asm__ __volatile__ ( "sleep" "\n\t" :: );
}

/**
 * To be call at first after wake-up
 */
void wakeUp()
{
    // "To avoid the MCU entering the sleep mode unless it is the programmer’s purpose,
    // it is recommended to write the Sleep Enable (SE) bit to one
    // just before the execution of the SLEEP instruction
    // and to clear it immediately after waking up."
    // Disabling sleep enable bit
    MCUCR &= ~_BV(SE);
}

/**
 * Delay for the given number of milliseconds.
 */
void delayMs(unsigned int ms)
{
    for(unsigned int i=0; i<ms; ++i)
    {
        delayMicroseconds(1000);
    }
}

/**
 * A basic RAII pin ON commande
 */
class On
{
public:
    On(uint8_t pin)
    {
        _pin = pin;
        digitalWrite(_pin, HIGH);
    }
    ~On()
    {
        digitalWrite(_pin, LOW);
    }
private:
    uint8_t _pin;
};


class BlissDoorbell
{
public:
    BlissDoorbell(uint8_t txPin, uint8_t emitterPowerPin) :
        _protocol(txPin, Timer1),
        _emitterPower(emitterPowerPin),
        _toggleRingCode(true)
    {
        // The 433MHz emitter power pin
        pinMode(_emitterPower, OUTPUT);
        // Power off
        digitalWrite(_emitterPower, LOW);

        // Blyss
        // setRepeatTransmit(16);
        // _protocol.setProtocol({ 500, {  1, 14 }, {  1,  3 }, {  3,  1 }, false });
    }

    void ring()
    {
        On power(_emitterPower);

        _toggleRingCode = !_toggleRingCode;

        // Ring
        // Received 1155892691 = 0x44E585D3 / 32bit Protocol: 2 / 1000100111001011000010111010011
        // Received 1449842067 = 0x566AD593 / 32bit Protocol: 2 / 1010110011010101101010110010011
        _protocol.send(_toggleRingCode ? 0x44E585D3 : 0x566AD593, 32);
    }

private:
    ManchesterAsym _protocol;
    uint8_t _emitterPower;
    bool _toggleRingCode;
};



//The setup function is called once at startup of the sketch
void setup()
{
    cli(); // Disable interrupts

    reducePower();

    enablePinPB2Interrupt();

    sei(); // Enable interrupts
}

// The loop function is called in an endless loop
void loop()
{
    BlissDoorbell bliss(TX_PIN, EMITTER_POWER_PIN);

    while (true)
    {
        // Enter sleep mode
        powerDown();

        // Wake-up
        wakeUp();

        // Ring twice
        bliss.ring();
        delayMs(500);
        bliss.ring();
    }
}
