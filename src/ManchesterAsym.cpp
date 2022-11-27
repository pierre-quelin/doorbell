
#include "ManchesterAsym.h"

static const ManchesterAsym::Protocol ManchesterAsym::Protocols[] = {
  { 500, { 1, 14 }, { 1, 3 }, { 3, 1 }, false }, // protocol 0 Blyss Doorbell Ref. DC6-FR-WH 656185
};

ManchesterAsym::ManchesterAsym(int nTransmitterPin, TimerOne& timer1) :
    _timer1(timer1),
    _nTransmitterPin(nTransmitterPin)
{
    pinMode(_nTransmitterPin, OUTPUT);
    digitalWrite(_nTransmitterPin, LOW);

    // Default values
    setRepeatTransmit(16); // TODO - à inclure dans la définition du protocol
    setProtocol(Protocols[0]);
}

/**
  * Sets the protocol to send.
  */
void ManchesterAsym::setProtocol(const Protocol& protocol)
{
    _protocol = protocol;
    _firstLogicLevel = (_protocol.invertedSignal) ? LOW : HIGH;
    _secondLogicLevel = (_protocol.invertedSignal) ? HIGH : LOW;
}


/**
  * Sets pulse length in microseconds
  */
void ManchesterAsym::setPulseLength(int nPulseLength)
{
  _protocol.pulseLength = nPulseLength;
}

/**
 * Sets Repeat Transmits
 */
void ManchesterAsym::setRepeatTransmit(int nRepeatTransmit)
{
    _nRepeatTransmit = nRepeatTransmit;
}

/**
 * Enable transmissions
 */
void ManchesterAsym::enableTransmit()
{
    // TODO - Specifique ATTiny85
    // Power Reduction Timer/Counter1
    // Timer/Counter1 used for clock
    PRR &= ~_BV(PRTIM1);

    _timer1.initialize(_protocol.pulseLength);
    _timer1.attachInterrupt(handleInterrupt);
    _timer1.start();
}

/**
 * Disable transmissions
 */
void ManchesterAsym::disableTransmit()
{
    _timer1.stop();
    _timer1.detachInterrupt();

    // TODO - Specifique ATTiny85
    // Power Reduction Timer/Counter1
    // Timer/Counter1 used for clock
    PRR |= _BV(PRTIM1);
}

/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 */
void ManchesterAsym::send(unsigned long code, unsigned int length)
{
    enableTransmit();

    for (int nRepeat = 0; nRepeat < _nRepeatTransmit; nRepeat++)
    {
        for (int i = length-1; i >= 0; i--)
        {
            if (code & (1L << i))
                transmit(_protocol.one);
            else
                transmit(_protocol.zero);
        }
        transmit(_protocol.syncFactor);
    }

    // wait interrupt for the last transmit
    waitInterrupt();
    // Disable transmit after sending (i.e., for inverted protocols)
    digitalWrite(_nTransmitterPin, LOW);

    disableTransmit();
}

/**
 * Transmit a single high-low pulse.
 */
void ManchesterAsym::transmit(HighLow pulses)
{
    for(uint8_t i=0; i<pulses.high; ++i)
    {
        waitInterrupt();
        digitalWrite(_nTransmitterPin, _firstLogicLevel);
    }
    for(uint8_t i=0; i<pulses.low; ++i)
    {
        waitInterrupt();
        digitalWrite(_nTransmitterPin, _secondLogicLevel);
    }
}

void ManchesterAsym::waitInterrupt()
{
    while( _interruptOccured == false) {}
    _interruptOccured = false; // clear flag
}

void ManchesterAsym::handleInterrupt()
{
    _interruptOccured = true; // set flag
}
