/*
  AmiPET - Use an Amiga Keyboard on a Commodore PET
  Amiga UK Layout to PET Business UK Layout keymap
*/



#define BITMASK_A500CLK 0b00010000    // PC4 Amiga Keyboard Clock
#define BITMASK_A500DAT 0b00100000    // PC5 Amiga Keyboard Data
#define BITMASK_SWITCH_ROW 0b00001111 // PC0, PC1, PC2, PC3 control AX0-AX3 (rows) of M3493B2
                                      // PD0, PD1 left free for serial communication
#define BITMASK_SWITCH_COL 0b00011100 // PD2, PD3, PD4 control AY0-AY2 (colums) of M3493B2
#define BITMASK_SWITCH_STB 0b00100000 // PD5 STROBE; High: the switch addressed by the select lines will be opened or closed (depending upon the state of the DATA pin)
#define BITMASK_SWITCH_RST 0b01000000 // PD6 RESET Master Reset; active high
#define BITMASK_SWITCH_DAT 0b10000000 // PD7 DATA; High: Keydown, switch closes; Low: Keyrelease, switch opens


#define SYNC            1
#define HANDSHAKE       2
#define READ            3
#define WAIT_LO         4

uint32_t counter = 0;
uint8_t state, bitn, key, keydown;

uint8_t key_matrix[0x68][2] = {
  //Array of PET Key Rows and Colums encoded for M3493B2 multiswitch
  //Entries represent Amiga Raw Key codes #00 to #68 (0-103 decimal)
  //Rows bits encode bitvalues of 1 2 4 6 for bits 0-3 (logic hole in M3493B2), Bits 4-6 unused, Bit 7 marks undefined keys
  //Colum bits leftshifted 2 bits to accomodate Arduino port setup (PD0, PD1 left free for serial communication)
  //Colum bits encode bitvalues of 1 2 4 for bits 2-4, bit 0,1,5,6 unused, Bit7 of marks shifted keys (cursor left/up)
  
  //Rows       Colums       // Amiga UK Layout  PET Business UK Layout
  {0b00000001, 0b00010100}, // ~                Up Arrow
  {0b00000001, 0b00000000}, // 1
  {0b00000000, 0b00000000}, // 2
  {0b00001011, 0b00000100}, // 3
  {0b00000001, 0b00000100}, // 4
  {0b00000000, 0b00000100}, // 5
  {0b00001011, 0b00001000}, // 6
  {0b00000001, 0b00001000}, // 7
  
  {0b00000000, 0b00001000}, // 8
  {0b00001011, 0b00001100}, // 9
  {0b00000001, 0b00001100}, // 0
  {0b00001011, 0b00010100}, // - _              : *
  {0b00000000, 0b00001100}, // = +              = -
  {0b00000100, 0b00010000}, // \ |
  {0b10000000, 0b00000000}, // <not defined>
  {0b00001001, 0b00010000}, // "Num 0"

  {0b00000101, 0b00000000}, // Q
  {0b00000100, 0b00000100}, // W
  {0b00000101, 0b00000100}, // E
  {0b00000100, 0b00001000}, // R
  {0b00000101, 0b00001000}, // T
  {0b00000100, 0b00001100}, // Y
  {0b00000101, 0b00001100}, // U
  {0b00000100, 0b00010100}, // I

  {0b00000101, 0b00010100}, // O
  {0b00000100, 0b00011000}, // P
  {0b00000101, 0b00011000}, // [ {              [
  {0b00000010, 0b00010000}, // ] }              ]
  {0b10000000, 0b00000000}, // <not defined>
  {0b00001010, 0b00011100}, // Num 1
  {0b00001001, 0b00011100}, // Num 2
  {0b00001000, 0b00011100}, // Num 3

  {0b00000011, 0b00000000}, // A
  {0b00000010, 0b00000100}, // S
  {0b00000011, 0b00000100}, // D
  {0b00000010, 0b00001000}, // F
  {0b00000011, 0b00001000}, // G
  {0b00000010, 0b00001100}, // H
  {0b00000011, 0b00001100}, // J 
  {0b00000010, 0b00010100}, // K
  
  {0b00000011, 0b00010100}, // L
  {0b00000010, 0b00011000}, // ; :              ; +
  {0b00000011, 0b00011000}, // # @              @
  {0b10000000, 0b00000000}, // <not defined>
  {0b10000000, 0b00000000}, // <not defined>
  {0b00000101, 0b00011100}, // Num 4
  {0b00000010, 0b00011100}, // Num 5
  {0b00000011, 0b00011100}, // Num 6

  {0b00000000, 0b00000000}, // <not defined>
  {0b00001001, 0b00000000}, // Z
  {0b00001010, 0b00000100}, // X
  {0b00001000, 0b00000100}, // C
  {0b00001001, 0b00000100}, // V 
  {0b00001000, 0b00001000}, // B
  {0b00001001, 0b00001000}, // N
  {0b00001010, 0b00001100}, // M
  
  {0b00001001, 0b00001100}, // , <
  {0b00001000, 0b00001100}, // . >
  {0b00001010, 0b00011000}, // / ?
  {0b10000000, 0b00000000}, // <not defined>
  {0b00001000, 0b00010000}, // Num .
  {0b00000001, 0b00010000}, // Num 7
  {0b00000000, 0b00010000}, // Num 8
  {0b00000001, 0b00011100}, // Num 9

  {0b00001010, 0b00001000}, // Space
  {0b00001011, 0b00000000}, // Backspace        Left Arrow
  {0b00000100, 0b00000000}, // Tab
  {0b00000011, 0b00010000}, // Num Enter        Return
  {0b00000011, 0b00010000}, // Return
  {0b00000010, 0b00000000}, // Esc
  {0b00000100, 0b00011100}, // Del              Inst Del
  {0b10000000, 0b00000000}, // <not defined>
  
  {0b10000000, 0b00000000}, // <not defined>
  {0b10000000, 0b00000000}, // <not defined>
  {0b10000000, 0b00000000}, // Num -
  {0b10000000, 0b00000000}, // <not defined>
  {0b00000101, 0b10010000}, // Cursor Up        Shift+Cursor Up/Down
  {0b00000101, 0b00010000}, // Cursor Down      Cursor Up/Down
  {0b00000000, 0b00010100}, // Cursor Right     Cursor left/right
  {0b00000000, 0b10010100}, // Cursor Left      Shift+Cursor left/right

  {0b10000000, 0b00000000}, // F1
  {0b10000000, 0b00000000}, // F2
  {0b10000000, 0b00000000}, // F3
  {0b10000000, 0b00000000}, // F4
  {0b10000000, 0b00000000}, // F5
  {0b10000000, 0b00000000}, // F6
  {0b10000000, 0b00000000}, // F7
  {0b10000000, 0b00000000}, // F8

  {0b10000000, 0b00000000}, // F9
  {0b10000000, 0b00000000}, // F10
  {0b10000000, 0b00000000}, // Num (
  {0b10000000, 0b00000000}, // Num )
  {0b10000000, 0b00000000}, // Num /
  {0b10000000, 0b00000000}, // Num *
  {0b10000000, 0b00000000}, // Num +
  {0b10000000, 0b00000000}, // Help

  {0b00001000, 0b00000000}, // Left Shift
  {0b00001000, 0b00011000}, // Right Shift
  {0b00001000, 0b00000000}, // CapsLock
  {0b10000000, 0b00000000}, // CTRL
  {0b00001010, 0b00000000}, // Left Alt         Off Rvs
  {0b00001010, 0b00010000}, // Right Alt        CLR Home
  {0b00001011, 0b00010000}, // Left Amiga       RUN Stop
  {0b00001001, 0b00011000}, // Right Amiga      Repeat
};