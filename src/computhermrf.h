/*
RF Sender and receiver library for Computherm Q8RF wireless thermostat

HW requirements:
- RF Transmitter: HopeRF RFM117W-868S1
- RF Receiver: HopeRF RFM217W-868S1

Note1: please be aware that the mentioned RF modules are not 5V ones.
Note2: maybe other receiver/transmitter modiel are also fine but a few important things needed: 868.35 MHz frequency, ASK OOK modulation, simple 1 pin interface.

Modulation details:

1 tick: 220 us (ideally)

SYNC: (3 x low,) 3 x high, 3 x low, 3 x high
    ___     ___
___|   |___|   |
SYNC length supported: tick x 2.5 ... tick x 3.5 = 550 ... 770 us

0: 2 x low, 1 x high
   _
__| |
0 pulse length: tick x 0.5 ... tick x 1.5 = 110 ... 330 us

1: 1 x low, 2 x high
  __
_|  |
1 pulse length: tick x 1.5 ... tick x 2.5 = 330 ... 550 us

STOP: 6 x low (and after that comes the 3 x low of th next SYNC)

________
STOP length: tick x 7 ... tick x 10 = 1760 ... 2200 us

*/
#ifndef ComputhermRF_h
#define ComputhermRF_h

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

typedef struct{
  String address;
  String command;
} computhermMessage;

class ComputhermRF{
public:
  ComputhermRF();
  ComputhermRF(uint8_t inputPin, uint8_t outputPin);
  void setPins(uint8_t inputPin, uint8_t outputPin);
  void startReceiver();
  void stopReceiver();
  bool isDataAvailable();
  void getData(String &id, bool &on);
  computhermMessage getData();
  void sendMessage(computhermMessage message);
  void sendMessage(String address, bool on);
private:
  static const uint16_t _TICK_LENGTH;
  static const uint16_t _SHORT_MIN;
  static const uint16_t _LONG_MIN;
  static const uint16_t _SYNC_MIN;
  static const uint16_t _SYNC_MAX;
  static const uint16_t _STOP_MIN;
  static const uint16_t _STOP_MAX;
  static const uint16_t _BUFF_SIZE;
  static const uint16_t _MSG_LENGTH;
  static uint8_t _inputPin;
  static uint8_t _outputPin;
  static volatile bool _avail;
  static volatile byte _buff[];
  static volatile byte _buffEnd;
  static byte _lastBuff[];
  static uint32_t _lastMessageArrived;
  void _wakeUpTransmitter();
  void _sendPulse(uint8_t lowTime, uint8_t highTime);
  void _sendStop();
  void _sendSync();
  void _sendBit(bool bit);
  void _sendHalfByte(char ch);
  static bool _isRepeat();
  static void _handler();
  char _toHex(uint8_t num);
};

#endif
