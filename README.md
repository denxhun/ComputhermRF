# Computherm Q8RF

Use your Arduino to send and receive RF messeges to/from Computherm Q8RF thermostat.

## Download
https://github.com/denxhun/ComputhermRF

## Wiki
https://github.com/denxhun/ComputhermRF/wiki

## Info

### Hardware

There are several 868.35 MHz transmitter and receiver available in many places. I used a cheap one and it works perfectly:
- RF Transmitter: HopeRF RFM117W-868S1
- RF Receiver: HopeRF RFM217W-868S1

Please be aware that the mentioned hardwares not tolerating 5V, so use a 3.3V based MCU or don't forget about the level shifter.

You should use Arduino's interrupt enabled digital input pins (on Uno/Nano: D2 or D3) since this library built on top of that feature.

### Usage

Use at your own risk! Truly! There is no warranty!

In the examples directory you can find 2 simple way of usage.

## Protocol

Computherm Q8RF retail product has a quite simple RF protocol. It uses 868.35 MHz ASK OOK modulated communication.
The interpretation of the bits in the message is not 100% clear but managed to identify the ON/OFF section and padding. The rest now handled as security ID - but could contain some other information as well. Feel free to investigate deeply the protocol to make the lib more handy.

## Compatibility

Maybe the lib could be adopted to other retail products - from the same manufacturer - but I don't have any other. Maybe the same product can be sold under other brands so please let me know if you managed to utilize my lib for other thermostats.