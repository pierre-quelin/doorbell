#ifndef MANCHESTER_ASYM_TX_h
#define MANCHESTER_ASYM_TX_h

#include <Arduino.h>
#include <TimerOne.h>

/**
 * Manchester asymmetric encoding
 */
class ManchesterAsym {

  public:
    ManchesterAsym(int nTransmitterPin, TimerOne& timer1);
    
    void send(unsigned long code, unsigned int length);
  
    void enableTransmit();
    void disableTransmit();
    void setPulseLength(int nPulseLength);
    void setRepeatTransmit(int nRepeatTransmit);

    /**
     * Description of a single pule, which consists of a high signal
     * whose duration is "high" times the base pulse length, followed
     * by a low signal lasting "low" times the base pulse length.
     * Thus, the pulse overall lasts (high+low)*pulseLength
     */
    struct HighLow {
        uint8_t high;
        uint8_t low;
    };

    /**
     * A "protocol" describes how zero and one bits are encoded into high/low
     * pulses.
     */
    struct Protocol {
        uint16_t pulseLength;

        HighLow syncFactor;
        HighLow zero;
        HighLow one;

        /**
         * If true, interchange high and low logic levels in all transmissions.
         */
        bool invertedSignal;
    };

    /* Format for protocol definitions:
     * {pulselength, Sync bit, "0" bit, "1" bit, invertedSignal}
     *
     * pulselength: pulse length in microseconds, e.g. 350
     * Sync bit: {1, 31} means 1 high pulse and 31 low pulses
     *     (perceived as a 31*pulselength long pulse, total length of sync bit is
     *     32*pulselength microseconds), i.e:
     *      _
     *     | |_______________________________ (don't count the vertical bars)
     * "0" bit: waveform for a data bit of value "0", {1, 3} means 1 high pulse
     *     and 3 low pulses, total length (1+3)*pulselength, i.e:
     *      _
     *     | |___
     * "1" bit: waveform for a data bit of value "1", e.g. {3,1}:
     *      ___
     *     |   |_
     *
     * These are combined to form Tri-State bits when sending or receiving codes.
     */
    static const Protocol Protocols[];

    void setProtocol(const Protocol& protocol);

  private:
    void transmit(HighLow pulses);

    TimerOne& _timer1;
    static void handleInterrupt();
    void waitInterrupt();
    inline volatile static bool _interruptOccured = false;

    uint8_t _firstLogicLevel;
    uint8_t _secondLogicLevel;

    int _nTransmitterPin;
    int _nRepeatTransmit;
    
    Protocol _protocol;
};

#endif
