// Copyright 2019 denxhun

#include <computhermrf.h>

const uint16_t ComputhermRF::_TICK_LENGTH = 220;
const uint16_t ComputhermRF::_SHORT_MIN = _TICK_LENGTH * 0.5;  // 110
const uint16_t ComputhermRF::_LONG_MIN = _TICK_LENGTH * 1.5;   // 330
const uint16_t ComputhermRF::_SYNC_MIN = _TICK_LENGTH * 2.5;   // 550
const uint16_t ComputhermRF::_SYNC_MAX = _TICK_LENGTH * 3.5;   // 770
const uint16_t ComputhermRF::_STOP_MIN = _TICK_LENGTH * 7;     // 1760
const uint16_t ComputhermRF::_STOP_MAX = _TICK_LENGTH * 10;    // 2200
const uint16_t ComputhermRF::_BUFF_SIZE = 70;
const uint16_t ComputhermRF::_MSG_LENGTH = 56;

uint8_t ComputhermRF::_inputPin;
uint8_t ComputhermRF::_outputPin;
volatile bool ComputhermRF::_avail;
volatile byte ComputhermRF::_buff[_BUFF_SIZE];
volatile byte ComputhermRF::_buffEnd;
byte ComputhermRF::_lastBuff[_BUFF_SIZE];
uint32_t ComputhermRF::_lastMessageArrived;

ComputhermRF::ComputhermRF() {
  ComputhermRF(255, 255);
}

ComputhermRF::ComputhermRF(uint8_t inputPin, uint8_t outputPin) {
  setPins(inputPin, outputPin);
}

void ComputhermRF::setPins(uint8_t inputPin, uint8_t outputPin) {
  stopReceiver();
  _inputPin = inputPin;
  _outputPin = outputPin;
  if (_outputPin < 255) {
    pinMode(_outputPin, OUTPUT);
  }
}

void ComputhermRF::startReceiver() {
  if (_inputPin < 255) {
    pinMode(_inputPin, INPUT);
    _avail = false;
    _buffEnd = 0;
    _lastMessageArrived = 0;
    attachInterrupt(digitalPinToInterrupt(_inputPin), _handler, CHANGE);
  }
}
void ComputhermRF::stopReceiver() {
  detachInterrupt(digitalPinToInterrupt(_inputPin));
}
bool ComputhermRF::isDataAvailable() {
  return _avail;
}
void ComputhermRF::getData(String &id, bool &on) {
    computhermMessage result = getData();
    id = result.address;
    on = (result.command == "ON");
}
computhermMessage ComputhermRF::getData() {
  computhermMessage result;
  String a = "     ";
  uint8_t n = 0;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 4; j++) {
      for (int j = 0; j < 4; j++) {
        if (_buff[i*4+j] == 1) {
          n |= 1 << (3-j);
        }
      }
      a[i] = _toHex(n);
      n = 0;
    }
  }
  result.address = a;
  String c = "";
  if (_buff[20] == 0 && _buff[21] == 0 && _buff[22] == 0 && _buff[23] == 0) {
    c = "ON";
  } else {
    c = "OFF";
  }
  result.command = c;
  _avail = false;
  return result;
}
void ComputhermRF::sendMessage(computhermMessage message) {
  sendMessage(message.address, message.command == "ON");
}
void ComputhermRF::sendMessage(String address, bool on) {
  if (address.length() != 5)
    return;
  _wakeUpTransmitter();
  stopReceiver();
  for (int i = 0; i < 8; i++) {
    _sendSync();
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 5; k++) {
        _sendHalfByte(address[k]);
      }
      if (on) {
        _sendHalfByte('0');  // ON
      } else {
        _sendHalfByte('F');  // OFF
      }
      _sendHalfByte('0');    // padding
    }
    _sendStop();
  }
  startReceiver();
}
void ComputhermRF::_wakeUpTransmitter() {
  digitalWrite(_outputPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(_outputPin, LOW);
}
void ComputhermRF::_sendPulse(uint8_t lowTime, uint8_t highTime) {
  digitalWrite(_outputPin, LOW);
  delayMicroseconds(lowTime * _TICK_LENGTH);
  if (highTime > 0) {
    digitalWrite(_outputPin, HIGH);
    delayMicroseconds(highTime * _TICK_LENGTH);
    digitalWrite(_outputPin, LOW);
  }
}
void ComputhermRF::_sendStop() {
  _sendPulse(6, 0);
}
void ComputhermRF::_sendSync() {
  _sendPulse(3, 3);
  _sendPulse(3, 3);
}
void ComputhermRF::_sendBit(bool bit) {
  if (bit) {
    _sendPulse(1, 2);
  } else {
    _sendPulse(2, 1);
  }
}
void ComputhermRF::_sendHalfByte(char ch) {
  uint8_t num = 0;
  if (ch >= '0' && ch <= '9') {
    num = ch - '0';
  } else {
    if (ch >= 'a' && ch <= 'f') {
      num = ch - 'a' + 10;
    } else {
      if (ch >= 'A' && ch <= 'F') {
        num = ch - 'A' + 10;
      } else {
        return;
      }
    }
  }
  for (int i = 0; i < 4; i++) {
    _sendBit(num & 1 << (3-i));
  }
}
bool ComputhermRF::_isRepeat() {
  bool result = false;
  for (int i = 0; i < _buffEnd; i++) {
    if (_buff[i] != _lastBuff[i]) {
      for (int j = 0; j < _buffEnd; j++) {
        _lastBuff[j] = _buff[j];
      }
      _lastMessageArrived = millis();
      return false;
    }
  }
  result = (millis()-_lastMessageArrived < 2000);
  _lastMessageArrived = millis();
  return result;
}
void ICACHE_RAM_ATTR ComputhermRF::_handler() {
  static uint32_t lastMs = 0, currMs, diffMs;
  currMs = micros();
  diffMs = currMs - lastMs;
  lastMs = currMs;
  if (_buffEnd == _BUFF_SIZE) {
    _buffEnd = 0;
  }
  if (!_avail) {
    if (digitalRead(_inputPin) == LOW) {  // Falling edge
      if (diffMs >= _SHORT_MIN && diffMs <= _SYNC_MAX) {
        if (diffMs >= _SYNC_MIN) {
            _buffEnd = 0;
        } else {
          if (diffMs <= _LONG_MIN) {
            _buff[_buffEnd++] = 0;
          } else {
            if (diffMs < _SYNC_MIN) {
              _buff[_buffEnd++] = 1;
            }
          }
        }
      }
    } else {  // Raising edge, only stop could be detected
      if (diffMs >= _STOP_MIN) {
        if (_buffEnd == _MSG_LENGTH && !_isRepeat()) {
          _avail = true;
        } else {
          _buffEnd = 0;
        }
      }
    }
  }
}
char ComputhermRF::_toHex(uint8_t num) {
  if (num < 10) {
    return '0' + num;
  } else {
    return 'A' + (num - 10);
  }
}
