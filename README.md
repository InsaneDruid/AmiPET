# AmiPET
Adapter to use Amiga keyboards on Commodore PET computers.
Uses an arduino to communicate with an Amiga keyboard and a M3493B2 Crosspoint to control the PET keyboard matrix.

## Background
Amiga side sends *rawkey* codes to the Arduino. An array of tupels contains bitvalues for the M3493B2 to switch the PET keyboard matrix rows and columns to the appropriate key. Entries represent Amiga rawkey codes #00 to #68 (0-103 decimal).

The sketch uses Arduino port bitwise operations to keep the latency low. Row values are encoded by bitvalues of 1 2 4 6 on bits 0-3 (logic hole in M3493B2), bits 4-6 are unused , bit 7 marks undefined keys (no PET key will be sent).
Colum bits are leftshifted 2 bits to accomodate Arduino port setup (PD0, PD1 left free for serial communication) and encode bitvalues of 1 2 4 on bits 2-4. Bit 0,1,5,6 are unused, Bit7 of marks shifted keys (cursor left/up). On those the arduino will send a Shift key press/release before sending the encoded key to the PET.

## How to change the Key Bindings
+ Get desired PET matrix entry and Amiga rawkey code.
+ Edit the (n+1)th tupel of *key_matrix* in *Amiga_PETkey.h* where n = amiga rawkey code.
+ Encode PET Row Number with bitvalues of 1 2 4 6 for bits 0-3. Set bit 7 if no PET key should be generated for this amiga key.
+ Encode PET Column Number with bitvalues 1 2 4 for bits 2-4. Set bit 7 if key should be shifted on PET side.

### Example1
Amiga *6* to PET *6*

+ Rawkey code of Amiga *6* key is #06 = 6 decimal, so edit the 7th tupel in the key_matrix array.
+ The keyboard scan matrix for the *6* key on a UK PET Business keyboard is *Row 9 Colum 2*
+ As the M2493B2 interprets the 4th row bit as 6 instead of 8, the row entry is 0b00001011.
+ Column entry is 2bit leftshifted binary representation of decimal 2.

Result: {0b00001011, 0b00001000}

### Example2
Amiga *Cursor Up* to PET *Shift+Cursor Up/Down*

+ Rawkey code of *Amiga Cursor Up* key () is #4C = 76 decimal, so edit the 77th tupel in the key_matrix array.
+ The keyboard scan matrix for the *Cursor Up/Down* key on a UK PET Business keyboard is *Row 5 Colum 4*.
+ The Row entry is a straigt binary representation of decimal 5.
+ Column entry is 2bit leftshifted binary representation of decimal 4 with bit 7 set addintionally to press/release shift before the *Cursor Up/Down* key on the PET side.

Result: {0b00000101, 0b10010000}

## Connecting the Adapter
+ The PET can supply unregulated 9VDC via the internal *RAWPwr" Headers (J11 on a universal Board) to both the Arduinos *VIN* pin and the M3493B2.
+ The Amiga keyboard needs +5V and GND from the Arduino as well as a connection to KBCLK (Arduino PC4) and KBData (Arduino PC5)
+ The PET keyboard connector gets connected to the X0-X9 pins (rows) and the Y0-Y7 pins (colums) of the M3493B2 as well as Ground.

## Printable Parts
+ *DIN_Connector_Mount .stl* can be used to mount the DIN Connector in place of the keyboard connector of an 8032-SK/8296.
+ *Case_Top.stl* and *Case_Bottom.stl* are designed to fit an Uno with the M3493B2 shield mounted.