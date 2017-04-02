/*

  File: IBM_Keyboard_Model_M_Using_Interrupts_V11_Cali.ino

  Terminal J - An IBM Model M USB Teensy-based Codable Keyboard Adapter with Arduino Source Code

  By J. Pagliaccio

  Parts of this code came from the PS2Keyboard library.

  PS2Keyboard.h - PS2Keyboard library
  Copyright (c) 2007 Free Software Foundation.  All right reserved.
  Written by Christian Weichel <info@32leaves.net>

  ** Mostly rewritten Paul Stoffregen <paul@pjrc.com>, June 2010
  ** Modified for use with Arduino 13 by L. Abraham Smith, <n3bah@microcompdesign.com> *
  ** Modified for easy interrup pin assignement on method begin(datapin,irq_pin). Cuningan <cuninganreset@gmail.com> **

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  For Jules.

  Notes
  --------------------------------------------------------

  This was written for the IBM Model M Keyboard - early version without the LEDs
  and the RJ-45 plug.

  This was written and tested using the Teensey 3.2.

  Use F1 to set debug mode and pressing escape unsets it.
  Its off by default staring in beta 00.00.01

  ESC - to escape from special keys mode

  Shift Hold is reboot
  ESC is debug off
  F1 is debug on
  Left Reset = ALT
  Right Reset is CTRL
  BLANK 1 is dump
  Blank 2 is clear
  ATTEN is Alex


  If you want no modifier keys pressed, use a zero.

  Keyboard.set_modifier(0);

  To press just one modifier is simple.

  Keyboard.set_modifier(MODIFIERKEY_SHIFT);

  To press more than one modifier, use the logic OR operator. For example:

  Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_ALT);


  Key send code
  57985 hibernate and shut down
  57986 sleep (quick)
  58509 win media center
  58592 Vol Up
  58594 Vol ???
  58601 vol up
  58602 vol down
  58755 media player
  58762 outlook email
  58770 calculator
  58772 open windows explorer - my computer
  58913 search
  58914 google search
*/

#define KEY_INTERNET_SEARCH 58915
#define KEY_CALCULATOR 58770
#define KEY_SLEEP 57986
#define KEY_SHUTDOWN 58000
#define KEY_VOL_UP 58592
#define KEY_VOL_DOWN 58602

//#include "PS2Keyboard.h"

#define PROGNAME "IBM Model M Keyboard Scancode Converter v00.00.01"

// Map the key to the decimal scan code and it works
// eg: #define KEY_VOL_UP 61447 is a lower case d,  Hex: F007

// 59600

// F9 Dec: 71,  E0 7A 57466
//#define KEY_VOL_UP 58000

// F10 Decimal in is 79,
//#define KEY_VOL_DN 57505
// 57394

const int ledPin = 13;
int clockPin = 2;
int dataPin = 3;
int first_bit_test = 0;
int counter = 0;
int numbits = 11;

bool debug = false;
static uint8_t incoming = 0;

//#define BUFFER_SIZE 45
//static volatile uint8_t buffer[BUFFER_SIZE];
//static volatile uint8_t head, tail;

bool shift = false;
bool ctrl = false;
bool alt = false;
bool caps = false;
bool ledState = false;

#define BUFMAX 9999

char keyBuffer[BUFMAX];
int keyBufferPos = 0;

unsigned long previousMillis = 0L;
unsigned long interval = 1000L;


// 58000
unsigned long b = 0;


//
// Setup
//----------------------------------------------------------
//
void setup() {
  pinMode(dataPin, INPUT);
  pinMode(clockPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);
  delay(10);

  Serial.println(PROGNAME);
  digitalWrite(ledPin, HIGH);
  delay(99);
  digitalWrite(ledPin, LOW);
  delay(99);
  digitalWrite(ledPin, HIGH);
  delay(99);
  digitalWrite(ledPin, LOW);

  Serial.print("KEY_PRINTSCREEN = ");
  Serial.println(KEY_PRINTSCREEN);

  attachInterrupt(clockPin, ps2interrupt, FALLING);
}

//
// The main loop
//----------------------------------------------------------
//
void loop() {

  if (digitalRead(clockPin) == LOW && first_bit_test == 0 && counter < numbits) {

    first_bit_test = 1;
    digitalWrite(ledPin, HIGH);
    //  data = data >> 1;
    //if (digitalRead(dataPin) == HIGH) {
    //  bitSet(data, 7);
    // }
    counter++;
  }

  if (digitalRead(clockPin) == HIGH && first_bit_test == 1) {
    first_bit_test = 0;
  }

  if (counter >= numbits) {

    if (debug) {
      Serial.print("Incoming decimal = ");
      Serial.println(incoming);
    }

    process_key(int(incoming));
    digitalWrite(ledPin, LOW);

    incoming = 0;
    counter = 0;
  }


  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
}


//
// The interrupt service routine from the clock pin
//----------------------------------------------------------
//
void ps2interrupt(void)
{
  static uint8_t bitcount = 0;
  static uint32_t prev_millis = 0;
  uint32_t now_millis;
  uint8_t n, val;

  val = digitalRead(dataPin);

  // get the milliseconds since boot
  now_millis = millis();

  // See if its been more than one millisecond since we had an inturrupt
  if (now_millis - prev_millis > 1) {
    bitcount = 0;
    incoming = 0;
  }

  prev_millis = now_millis;
  n = bitcount - 1;

  if (n <= 7) {
    incoming |= (val << n);
  }

  bitcount++;

  // The scan code size
  if (bitcount == 11) {
    bitcount = 0;
  }
}

//
// Process - eg: remap the key inputs
//----------------------------------------------------------
//
void process_key(int r)
{
  long s = 0L;
  char c = 0;

  switch (r) {
    case 7: s = KEY_F1;
      debug = true;
      break;
    case 8: s = KEY_ESC; // escape
      debug = false;
      Keyboard.releaseAll();
      Keyboard.set_modifier(0);
      keyBufferPos = 0;
      c = 0;
      break;
    case 13: s = KEY_TAB; break;
    case 14: s = KEY_TILDE; break;
    case 17: // Left Enter is now LEFT_CTRL
      if (ctrl == false) {
        ctrl = true;
        //Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.set_modifier(MODIFIERKEY_CTRL);
      } else {
        ctrl = false;
        //Keyboard.release(KEY_LEFT_CTRL);
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
      }
      // see 240 for release all
      break;
    case 18: // Left Shift
      if (shift == false) {
        //Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.set_modifier(MODIFIERKEY_SHIFT);
        shift = true;
      } else {
        Keyboard.releaseAll();
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
        shift = false;
      }
      break;
    case 20:
      s = KEY_CAPS_LOCK;
      break;
    case 21: s = KEY_Q; c = 'q'; break;
    case 22: s = KEY_1; c = '1'; break;
    case 25: // Left Alt
      if (alt == false) {
        Keyboard.set_modifier(MODIFIERKEY_ALT);
        alt = true;
      } else {
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
        alt = false;
      }
      break; // ALT
    case 26: s = KEY_Z; c = 'z'; break;
    case 27: s = KEY_S; c = 's'; break;
    case 28: s = KEY_A; c = 'a'; break;
    case 29: s = KEY_W; c = 'w'; break;
    case 30: s = KEY_2; c = '2'; break;
    case 33: s = KEY_C; c = 'c'; break;
    case 34: s = KEY_X; c = 'x'; break;
    case 35: s = KEY_D; c = 'd'; break;
    case 36: s = KEY_E; c = 'e'; break;
    case 37: s = KEY_4; c = '4'; break;
    case 38: s = KEY_3; c = '3'; break;
    case 41: s = KEY_SPACE; c = ' '; break;
    case 42: s = KEY_V; c = 'v'; break;
    case 43: s = KEY_F; c = 'f'; break;
    case 44: s = KEY_T; c = 't'; break;
    case 45: s = KEY_R; c = 'r'; break;
    case 46: s = KEY_5; c = '5'; break;
    case 49: s = KEY_N; c = 'n'; break;
    case 50: s = KEY_B; c = 'b'; break;
    case 51: s = KEY_H; c = 'h'; break;
    case 52: s = KEY_G; c = 'g'; break;
    case 53: s = KEY_Y; c = 'y'; break;
    case 54: s = KEY_6; c = '6'; break;
    case 57: // Right ALT
      if (alt == false) {
        Keyboard.set_modifier(MODIFIERKEY_ALT);
        alt = true;
      } else {
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
        alt = false;
      }
      break;
    case 58: s = KEY_M; c = 'm'; break;
    case 59: s = KEY_J; c = 'j'; break;
    case 60: s = KEY_U; c = 'u'; break;
    case 61: s = KEY_7; c = '7'; break;
    case 62: s = KEY_8; c = '8'; break;
    case 65: s = KEY_COMMA; c = ','; break;
    case 66: s = KEY_K; c = 'k'; break;
    case 67: s = KEY_I; c = 'i'; break;
    case 68: s = KEY_O; c = 'o'; break;
    case 69: s = KEY_0; c = '0'; break;
    case 70: s = KEY_9; c = '9'; break;
    case 71: // F9
      s = KEY_VOL_DOWN;
      c = '?';
      break;
    case 73: s = KEY_PERIOD; c = '.'; break;
    case 74: s = KEY_SLASH; c = '/'; break;
    case 75: s = KEY_L; c = 'l'; break;
    case 76: s = KEY_SEMICOLON; c = ';'; break;
    case 77: s = KEY_P; c = 'p'; break;
    case 78: s = KEY_MINUS; c = '-'; break;
    case 79: // F10
      s = KEY_VOL_UP;
      break;
    case 82: s = KEY_QUOTE; c = '"'; break;
    case 84: s = KEY_LEFT_BRACE; c = '['; break;
    case 85: s = KEY_EQUAL; c = '='; break;
    case 86: // F11
      s = KEY_INTERNET_SEARCH;
      break;
    case 87: s = KEY_PRINTSCREEN; break;
    case 88: // Right Reset is CTRL
      if (ctrl == false) {
        ctrl = true;
        Keyboard.set_modifier(MODIFIERKEY_CTRL);
      } else {
        ctrl = false;
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
      }

      break;
    case 89: //Keyboard.press(KEY_RIGHT_SHIFT);
      if (shift == false) {
        Keyboard.set_modifier(MODIFIERKEY_SHIFT);
        shift = true;
      } else {
        Keyboard.set_modifier(0);
        Keyboard.set_key1(0);
        Keyboard.send_now();
        shift = false;
      }
      break;
    case 90: s = KEY_ENTER; c = '\n'; break;
    case 91: s = KEY_RIGHT_BRACE; c = ']'; break;
    case 92: s = KEY_BACKSLASH; c = '\\'; break;
    case 93: s = KEY_QUOTE; c = '"'; break;
    case 94: // F12
      s = KEY_CALCULATOR;
      break;
    case 95: s = KEY_PRINTSCREEN; break;
    case 96: s = KEY_DOWN_ARROW; break;
    case 97: s = KEY_LEFT_ARROW; break;
    case 98: // HOLD and Shift Hold Key
      if (shift) {
        SCB_AIRCR = 0x05FA0004; // software reset
      }
      break;
    case 99: s = KEY_UP_ARROW; break;
    case 100: s = KEY_DELETE; break;
    case 101: s = KEY_END; break;
    case 102: s = KEY_BACKSPACE; break;
    case 103: s = KEY_INSERT; break;
    case 105: s = KEY_1; c = '1'; break;
    case 106: s = KEY_RIGHT_ARROW; break;
    case 107: s = KEY_4; c = '4'; break;
    case 108: s = KEY_7; c = '7'; break;
    case 109: s = KEY_PAGE_DOWN; break;
    case 110: s = KEY_HOME; break;
    case 111: s = KEY_PAGE_UP; break;
    case 112: s = KEY_0; c = '0'; break;
    case 113: s = KEY_PERIOD; c = '.'; break;
    case 114: s = KEY_2; c = '2'; break;
    case 115: s = KEY_5; c = '5'; break;
    case 116: s = KEY_6; c = '6'; break;
    case 117: s = KEY_8; c = '8'; break;
    case 118:
      // Display the keystroke buffer to the output
      Keyboard.print(keyBuffer);
      break;
    case 119:
      // Empty the keystroke buffer
      keyBufferPos = 0;
      break;
    case 120: // F11
      s = KEY_INTERNET_SEARCH;
      break;
    //  case 121: s = KEY_PLUS; break;
    case 122: s = KEY_3; c = '3'; break;
    case 123: s = KEY_MINUS; c = '-'; break;
    case 125: s = KEY_9; c = '9'; break;
    case 126: s = KEY_SCROLL_LOCK; break;
    case 131: s = KEY_F7; break;
    case 132:

      Keyboard.print("Alex19 isn't so great? Are you kidding me? ");
      Keyboard.print("When was the last time you saw a player with such an ability and movement with fox? ");
      Keyboard.print("Alex puts the game in another level, and we will be blessed if we ever see a player with his skill ");
      Keyboard.print("and passion for the game again. mang0 breaks records. Armada breaks records. ");
      Keyboard.println("Alex19 breaks the rules. You can keep your statistics. I prefer the magic.");

      break;
    case 240: // the release of a modal key
      Keyboard.set_modifier(0);
      Keyboard.set_key1(0);
      Keyboard.send_now();
      Keyboard.releaseAll();
      break;
    default: break;
  }

  // save these keystrokes
  if (keyBufferPos < BUFMAX && c) {
    keyBuffer[keyBufferPos++] = c;
    keyBuffer[keyBufferPos + 1] = 0;
  }

  // If the scan code is not zero
  // It will be zero/null for the special keys and skip this
  if (s) {
    if (debug) {
      Serial.print(" Scan code = ");
      Serial.println(s);
    }

    // otherwise - for all normal keys we do this
    Keyboard.press(s);
    Keyboard.release(s);
  }
}
