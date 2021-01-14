/*
  AmiPET - Use an Amiga Keyboard on a Commodore PET
  The Amiga Keyboard to Arduino communication is based on Code from Olaf Berthold https://forum.arduino.cc/index.php?topic=139358.0
*/

#include "Amiga_PETKey.h"

void setup() {
  DDRC &= ~(BITMASK_A500CLK | BITMASK_A500DAT);                           // INPUT
  DDRC |= BITMASK_SWITCH_ROW;                                             // OUTPUT
  DDRD |= BITMASK_SWITCH_COL;                                             // OUTPUT
  DDRD |= (BITMASK_SWITCH_STB | BITMASK_SWITCH_DAT | BITMASK_SWITCH_RST); // OUTPUT

  switchReset();
  state = SYNC;
}

void loop() {
  if (state == SYNC) {                                                    // sync pulse LOW
    if ((PINC & BITMASK_A500CLK) != 0) state = HANDSHAKE;
  }
  else if (state == HANDSHAKE) {                                          // handshake
    if (counter == 0) {
      DDRC |= BITMASK_A500DAT;                                            // set IO direction to OUTPUT
      PORTB &= ~BITMASK_A500DAT;                                          // set OUTPUT to LOW
      counter = millis();
    }
    else if (millis() - counter > 10) {
      counter = 0;
      DDRC &= ~BITMASK_A500DAT;                                           // set IO direction to INPUT
      state = WAIT_LO;
      key = 0;
      bitn = 7;
    }
  }
  else if (state == WAIT_LO) {                                            // waiting for the next bit
    if ((PINC & BITMASK_A500CLK) == 0) {
      noInterrupts();
      state = READ;
    }
  }
  else if (state == READ) {                                               // read key message (8 bits)
    if ((PINC & BITMASK_A500CLK) != 0) {
      if (bitn--) {
        key += ((PINC & BITMASK_A500DAT) == 0) << (bitn);                 // key code (add bits 0...6)
        state = WAIT_LO;
      }
      else {                                                              // read last bit (key down)
        keydown = ((PINC & BITMASK_A500DAT) != 0);                        // true if key down
        interrupts();
        state = HANDSHAKE;
        if (!(key_matrix[key][0] & 0b10000000)) {                         // key has existing PET definition
          if (keydown) {
            if (key_matrix[key][1] & 0b10000000) {                        // key matrix entry needs PET shift key to be pressed
              shiftPress();
            }
            switchSetPressed();
          }
          else {
            if (key_matrix[key][1] & 0b10000000) {                        // key matrix entry needs PET shift key to be released
              shiftRelease();
            }
            switchSetReleased();
          }
          switchSetKey(key);
          switchStrobe();
        }
      }
    }
  }
}

void switchSetKey(uint8_t k) {
  PORTC &= ~BITMASK_SWITCH_ROW;                                         // ROWS: clear all bits in row mask area
  PORTC |= (BITMASK_SWITCH_ROW & key_matrix[k][0]);                     // ROWS: set Bits in row mask Area according to Keyboard key_matrix

  PORTD &= ~BITMASK_SWITCH_COL;                                         // COLUMS: clear all bit in colum mask area
  PORTD |= (BITMASK_SWITCH_COL & key_matrix[k][1]);                     // COLUMS: set bits in column mask area according to Keyboard key_matrix
}

void switchSetPressed() {
  PORTD |= BITMASK_SWITCH_DAT;
}

void switchSetReleased() {
  PORTD &= ~BITMASK_SWITCH_DAT;
}

void switchStrobe() {                                                   // activate switches according to state of select lines
  PORTD |= BITMASK_SWITCH_STB;
  PORTD &= ~BITMASK_SWITCH_STB;
}

void shiftPress() {
  switchSetKey(0x60);
  switchSetPressed();
  switchStrobe();
}

void shiftRelease() {
  switchSetKey(0x60);
  switchSetReleased();
  switchStrobe();
}

void switchReset() {
  PORTD |= BITMASK_SWITCH_RST;
  delay(1);
  PORTD &= ~BITMASK_SWITCH_RST;
}
