# doorbell

A basic, low power consumption, doorbell transmitter to replace or complement an existing system.
Can be added to a letterbox, gate, cat flap or anything else.

Initially created for a Bliss brand doorbell Ref. DC6-FR-WH 656185, it can be easily transposed to any other system.

A big thanks to :
- sui77 for his [rc-switch](https://github.com/sui77/rc-switch) library. 
- [Saleae](https://www.saleae.com/) for the ability to decode protocols effortlessly.

## Software implementation

Sleep mode : Power-down
Wake up on INT0 on pin PB2 change.
Use the timer1 as a tick generator to obtain compliant timings.

### Libs

[TimerOne-master](https://github.com/PaulStoffregen/TimerOne/blob/master/TimerOne.h)

## Hardware

- AA NiMh battery 
- Plastic Battery Storage Case Box Holder For AA
- DC-DC boost converter module. Input voltage 0.9-5V. Output voltage 5V.
- [ATTiny85](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7598_Automotive-Microcontrollers-ATtiny25-45-85_Datasheet.pdf)
- [STX882](https://www.nicerf.com/products/detail/433mhz-ask-transmitter-module-stx882.html) ASK RF module 433mhz transmitter module

## Schematic

TODO

## Prototype

TODO

## License

[![GPLV3](https://www.gnu.org/graphics/gplv3-88x31.png)](LICENSE)
