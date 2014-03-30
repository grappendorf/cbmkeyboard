/**
 * CBM Keyboard to PS2 adapter
 *
 * (C) 2008 Dirk Grappendorf, www.grappendorf.net.
 *
 * Licensed under the MIT license, http://opensource.org/licenses/MIT
 */

/**
 * If you want to use this code inside the Arduino IDE, remove the WProgram.h
 * include and delete the path prefixes from the other includes. Also remove
 * the main() function.
 */

#include <Arduino.h>
#include <ps2dev/ps2dev.h>
#include <avr/pgmspace.h>

/** Define this if you want debug information on the serial interface */
//#define DEBUG 1

/** Define this if you want debug information about the PS2 transmission */
//#define DEBUG_PS2 1

#ifdef DEBUG
#define SERIAL_DEBUG_PRINT(msg) Serial.print(msg)
#define SERIAL_DEBUG_PRINT_BIN(msg) Serial.print(msg,2)
#define SERIAL_DEBUG_PRINT_DEC(msg) Serial.print(msg,10)
#define SERIAL_DEBUG_PRINT_HEX(msg) Serial.print(msg,16)
#define SERIAL_DEBUG_PRINTLN(msg) Serial.println(msg)
#define SERIAL_DEBUG_PRINTLN_BIN(msg) Serial.println(msg,2)
#define SERIAL_DEBUG_PRINTLN_DEC(msg) Serial.println(msg,10)
#define SERIAL_DEBUG_PRINTLN_HEX(msg) Serial.println(msg,16)
#else
#define SERIAL_DEBUG_PRINT(msg)
#define SERIAL_DEBUG_PRINT_BIN(msg)
#define SERIAL_DEBUG_PRINT_DEC(msg)
#define SERIAL_DEBUG_PRINT_HEX(msg)
#define SERIAL_DEBUG_PRINTLN(msg)
#define SERIAL_DEBUG_PRINTLN_BIN(msg)
#define SERIAL_DEBUG_PRINTLN_DEC(msg)
#define SERIAL_DEBUG_PRINTLN_HEX(msg)
#endif

/** Pin number of the LED */
const int PIN_LED = 8;

/** Pin numbers of the column output lines */
const int PIN_COLSEL_0 = 5;
const int PIN_COLSEL_1 = 2;
const int PIN_COLSEL_2 = 3;
const int PIN_COLSEL_3 = 4;

/** Pin numbers of the row input lines */
const int PIN_ROW_A = 9;
const int PIN_ROW_B = 10;
const int PIN_ROW_C = 14;
const int PIN_ROW_D = 15;
const int PIN_ROW_E = 16;
const int PIN_ROW_F = 17;
const int PIN_ROW_G = 18;
const int PIN_ROW_H = 19;

/** Pin number of the PS/2 clock line */
const int PIN_PS2_CLK = 6;

/** Pin number of the PS/2 data line */
const int PIN_PS2_DATA = 7;

/** Special value indicating that no row should be selected */
const int NO_ROW = 0xff;

/** Special value indicating that no key is pressed */
const int NO_KEY = 0xff;

/** Modifier flags */
const uint8_t MOD_NONE = 0;
const uint8_t MOD_CTRL = 1;
const uint8_t MOD_LEFT_SHIFT = 2;
const uint8_t MOD_RIGHT_SHIFT = 4;
const uint8_t NUM_MODIFIERS = 3;

/** Delay between selecting a column and reading the rows */
const int COL_SELECT_DELAY_MICROS = 20;

/** Delay between the first and second reading of the keyboard */
const int DEBOUNCE_DELAY_MILLIS = 2;

const uint8_t MOD_COL_NUM[] = { 8, 6, 6 };

const uint8_t MOD_ROW_BIT[] = { 1, 1, 64 };

const uint8_t MOD[] = { MOD_CTRL, MOD_LEFT_SHIFT, MOD_RIGHT_SHIFT };

const uint16_t MAKE_WITH_E0 = 0x0100;
const uint16_t MAKE_WITH_SHIFT = 0x0200;
const uint16_t MAKE_WITH_ALTGR = 0x0400;

/** PS2 protocol request codes */
const uint8_t PS2_SET_RESET_LEDS = 0xed;
const uint8_t PS2_ECHO = 0xee;
const uint8_t PS2_SET_SCAN_CODE_SET = 0xf0;
const uint8_t PS2_READ_ID = 0xf2;
const uint8_t PS2_SET_TYPEMATIC_RATE_DELAY = 0xf3;
const uint8_t PS2_ENABLE = 0xf4;
const uint8_t PS2_DISABLE = 0xf5;
const uint8_t PS2_SET_DEFAULT = 0xf6;
const uint8_t PS2_SET_ALL_KEYS_TYPEMATIC = 0xf7;
const uint8_t PS2_SET_ALL_KEYS_MAKE_BREAK = 0xf8;
const uint8_t PS2_SET_ALL_KEYS_MAKE = 0xf9;
const uint8_t PS2_SET_ALL_KEYS_TYPEMATIC_MAKE_BREAK = 0xfa;
const uint8_t PS2_SET_KEY_TYPE_TYPEMATIC = 0xfb;
const uint8_t PS2_SET_KEY_TYPE_MAKE_BREAK = 0xfc;
const uint8_t PS2_SET_KEY_TYPE_MAKE = 0xfd;
const uint8_t PS2_RESEND = 0xfe;
const uint8_t PS2_RESET = 0xff;

/** PS2 protocol response codes */
const uint8_t PS2_BAT_OK = 0xaa;
const uint8_t PS2_ID_1 = 0xab;
const uint8_t PS2_ID_2 = 0x83;
const uint8_t PS2_ACK = 0xfa;

/** Default typematic delay intervals in milliseconds */
const uint16_t DEFAULT_TYPEMATIC_DELAY_INTERVALL = 500;

/** Typematic delay intervals that can be set by the host */
prog_uint16_t typematicDelayIntervalls[] PROGMEM = { 250, 500, 750, 1000 };

/** Default typematics repeat interval in milliseconds */
const uint16_t DEFAULT_TYPEMATIC_REPEAT_INTERVALL = 54;

/** Typematic repeat intervals that can be set by the host */
prog_uint16_t typematicRepeatIntervalls[] PROGMEM = { 33, 37, 42, 46, 48, 54, 58, 63, 67, 75, 83, 92, 100, 109, 116,
		125, 133, 149, 167, 182, 200, 217, 233, 250, 270, 303, 333, 370, 400, 435, 476, 500 };

/**
 * Make codes for key presses without any modifier
 * |-> Col
 * v Row
 */
prog_uint16_t makeCodes[] PROGMEM
		= { 0x001e, 0x002e, 0x003e, 0x0055, 0x000a, 0x016c, 0x0000, 0x0000, 0x0016, 0x0025, 0x003d, 0x0045, 0x0083,
				0x017a, 0x0000, 0x0001, 0x0076, 0x001b, 0x002b, 0x0033, 0x005d, 0x0042, 0x004c, 0x0003, 0x001c, 0x0023,
				0x0034, 0x003b, 0x005a, 0x004b, 0x0052, 0x000b, 0x000d, 0x001d, 0x002d, 0x0035, 0x005b, 0x0043, 0x004d,
				0x0446, 0x0015, 0x0024, 0x002c, 0x003c, 0x043e, 0x0044, 0x0054, 0x000c, 0x0000, 0x0021, 0x0032, 0x0049,
				0x0461, 0x0000, 0x0000, 0x0004, 0x001a, 0x002a, 0x0031, 0x0041, 0x0415, 0x0000, 0x0172, 0x0006, 0x0000,
				0x0022, 0x0029, 0x003a, 0x0174, 0x0000, 0x004a, 0x0005, 0x0061, 0x0026, 0x0036, 0x0046, 0x0066, 0x004e,
				0x0000, 0x0000, };

/**
 * Make codes for key presses with the shift modifier
 * |-> Col
 * v Row
 */
prog_uint16_t makeCodesShift[] PROGMEM = { 0x021e, 0x022e, 0x023e, 0x0255, 0x020a, 0x0169, 0x0000, 0x0000, 0x0216,
		0x0225, 0x023d, 0x0245, 0x0283, 0x017d, 0x0000, 0x0201, 0x0276, 0x021b, 0x022b, 0x0233, 0x025d, 0x0242, 0x024c,
		0x0203, 0x021c, 0x0223, 0x0234, 0x023b, 0x005a, 0x024b, 0x0252, 0x020b, 0x000d, 0x021d, 0x022d, 0x0235, 0x025b,
		0x0243, 0x024d, 0x044e, 0x0215, 0x0224, 0x022c, 0x023c, 0x000e, 0x0244, 0x0254, 0x020c, 0x0000, 0x0221, 0x0232,
		0x0249, 0x045b, 0x0000, 0x0000, 0x0204, 0x021a, 0x022a, 0x0231, 0x0241, 0x0000, 0x0000, 0x0175, 0x0206, 0x0000,
		0x0222, 0x0000, 0x023a, 0x016b, 0x0000, 0x024a, 0x0205, 0x0261, 0x0226, 0x0236, 0x0246, 0x0171, 0x024e, 0x0000,
		0x0000, };

PS2dev ps2(PIN_PS2_CLK, PIN_PS2_DATA);

uint8_t debounceKey = NO_KEY;

uint8_t pressedKey = NO_KEY;

uint8_t modifiers = MOD_NONE;

/** Timestamp on which the keyboard is re-read to debounce a pressed key */
unsigned long debounceTimestamp = 0;

/** Timestamp on which to switch the LED off */
unsigned long ledOffTimestamp = 0;

/** Timestamp after whicht the LED can blink again */
unsigned long ledBlinkTimestamp = 1000;

/** Delay between first key press and first repeat in milliseconds */
uint16_t typematicDelayIntervall = DEFAULT_TYPEMATIC_DELAY_INTERVALL;

/** Delay between successive key repeats in milliseconds */
uint16_t typematicRepeatIntervall = DEFAULT_TYPEMATIC_REPEAT_INTERVALL;

/** Time stamp on which a key is to be repeated for the next time */
unsigned long typematicTimestamp = 0;

/**
 * Write a byte on the PS2 interface.
 *
 * @param data The data to write
 */
void ps2Write(uint8_t data)
{
#ifdef DEBUG_PS2
	Serial.print("PS2 Write ");
	Serial.println(data, 16);
#endif
	ps2.write(data);
	delay(1);
}

/**
 * Check if data is available on the PS2 interface.
 *
 * @return True if PS2 data is available
 */
uint8_t ps2Available()
{
	return digitalRead(PIN_PS2_DATA) == LOW && digitalRead(PIN_PS2_CLK) == HIGH;
}

/**
 * Read data from the PS2 interface.
 *
 * @return The data read
 */
uint8_t ps2Read()
{
	unsigned char data;
	ps2.read(&data);
#ifdef DEBUG_PS2
	Serial.print("PS2 Read ");
	Serial.println(data, 16);
#endif
	return data;
}

/**
 * Select a single keyboard column 0 <= column <= 9. The selected column line is
 * switched to low, all other column lines are set to high.
 * If column is NO_COL, no column is selected.
 *
 * @param column The column to select
 */
void selectColumn(int column)
{
	if (column >= 0 && column <= 9) {
		digitalWrite(PIN_COLSEL_0, column & 1 ? HIGH : LOW);
		digitalWrite(PIN_COLSEL_1, column & 2 ? HIGH : LOW);
		digitalWrite(PIN_COLSEL_2, column & 4 ? HIGH : LOW);
		digitalWrite(PIN_COLSEL_3, column & 8 ? HIGH : LOW);
	} else {
		digitalWrite(PIN_COLSEL_0, HIGH);
		digitalWrite(PIN_COLSEL_1, HIGH);
		digitalWrite(PIN_COLSEL_2, HIGH);
		digitalWrite(PIN_COLSEL_3, HIGH);
	}
}

/**
 * Read the keyboard rows. Depending on the currently selected column, A
 * row bit is zero if the key at this matrix position is pressed. So if no
 * key is pressed, 0xff is returned.
 *
 * @return The keyboard row value
 */
uint8_t readRow()
{
	return (digitalRead(PIN_ROW_A) ? 1 : 0) | (digitalRead(PIN_ROW_B) ? 2 : 0) | (digitalRead(PIN_ROW_C) ? 4 : 0)
			| (digitalRead(PIN_ROW_D) ? 8 : 0) | (digitalRead(PIN_ROW_E) ? 16 : 0) | (digitalRead(PIN_ROW_F) ? 32 : 0)
			| (digitalRead(PIN_ROW_G) ? 64 : 0) | (digitalRead(PIN_ROW_H) ? 128 : 0);
}

/**
 * Scan the keyboard for a pressed key. If a key is pressed, it's code is
 * 0 r3 r2 r1 r0 c2 c1 c0
 * with row 0 <= r <= 9 and column 0 <= c <= 7.
 * If no key is pressed, NO_KEY is returned.
 *
 * @param modifiers A pointer to a variable that holds the modifiers that
 * currently pressed (can be null)
 * @return The code of the pressed key or NO_KEY if no key is pressed
 */
uint8_t scanKeyboard(uint8_t *modifiers)
{
	uint8_t col;
	uint8_t row;
	uint8_t rowNoMods;
	uint8_t i;
	uint8_t key = NO_KEY;

	if (modifiers != NULL) {
		*modifiers = MOD_NONE;
	}

	for (col = 0; col <= 9; ++col) {
		selectColumn(col);
		delayMicroseconds(COL_SELECT_DELAY_MICROS);
		row = readRow();

		rowNoMods = row;
		for (i = 0; i < NUM_MODIFIERS; ++i) {
			if (col == MOD_COL_NUM[i]) {
				rowNoMods |= MOD_ROW_BIT[i];
			}
		}

		if (key == NO_KEY && rowNoMods != 0xff) {
			key = col << 3;
			while ((rowNoMods & 1) == 1) {
				rowNoMods >>= 1;
				++key;
			}
		}

		if (modifiers != NULL) {
			for (i = 0; i < NUM_MODIFIERS; ++i) {
				if (col == MOD_COL_NUM[i] && (row & MOD_ROW_BIT[i]) == 0) {
					*modifiers |= MOD[i];
				}
			}
		}
	}

	selectColumn(NO_ROW);

	return key;
}

/**
 * System setup.
 */
void setup()
{
	digitalWrite(PIN_LED, HIGH);
	pinMode(PIN_LED, OUTPUT);
	selectColumn(-1);
	pinMode(PIN_COLSEL_0, OUTPUT);
	pinMode(PIN_COLSEL_1, OUTPUT);
	pinMode(PIN_COLSEL_2, OUTPUT);
	pinMode(PIN_COLSEL_3, OUTPUT);
	digitalWrite(PIN_ROW_A, HIGH);
	digitalWrite(PIN_ROW_B, HIGH);
	digitalWrite(PIN_ROW_C, HIGH);
	digitalWrite(PIN_ROW_D, HIGH);
	digitalWrite(PIN_ROW_E, HIGH);
	digitalWrite(PIN_ROW_F, HIGH);
	digitalWrite(PIN_ROW_G, HIGH);
	digitalWrite(PIN_ROW_H, HIGH);
	pinMode(PIN_ROW_A, INPUT);
	pinMode(PIN_ROW_B, INPUT);
	pinMode(PIN_ROW_C, INPUT);
	pinMode(PIN_ROW_D, INPUT);
	pinMode(PIN_ROW_E, INPUT);
	pinMode(PIN_ROW_F, INPUT);
	pinMode(PIN_ROW_G, INPUT);
	pinMode(PIN_ROW_H, INPUT);
	Serial.begin(9600);
}

/**
 * Turn the LED on.
 */
void blinkLed()
{
	if (millis() > ledBlinkTimestamp) {
		ledOffTimestamp = millis() + 50;
		ledBlinkTimestamp = millis() + 250;
		digitalWrite(PIN_LED, LOW);
	}
}

/**
 * Switch the LED off if the blink duration elapsed.
 */
void calmDownLed()
{
	if (millis() > ledOffTimestamp) {
		digitalWrite(PIN_LED, HIGH);
	}
}

void doPs2Protocol()
{
	if (ps2Available()) {
		unsigned char data = ps2Read();
		switch (data) {
			case PS2_SET_RESET_LEDS:
				delay(1);
				ps2Write(PS2_ACK);
				data = ps2Read();
				delay(1);
				ps2Write(PS2_ACK);
				SERIAL_DEBUG_PRINT("Set LEDS ");
				SERIAL_DEBUG_PRINTLN_BIN(data);
				break;
			case PS2_ECHO:
				SERIAL_DEBUG_PRINTLN("Echo");
				delay(1);
				ps2Write(PS2_ECHO);
				break;
			case PS2_SET_SCAN_CODE_SET:
				delay(1);
				ps2Write(PS2_ACK);
				data = ps2Read();
				delay(1);
				ps2Write(PS2_ACK);
				SERIAL_DEBUG_PRINTLN("Set Scan Code Set ");
				SERIAL_DEBUG_PRINTLN_DEC(data);
				if (data == 0) {
					delay(1);
					ps2Write(0);
				}
				break;
			case PS2_READ_ID:
				SERIAL_DEBUG_PRINTLN("Read Id");
				delay(1);
				ps2Write(PS2_ID_1);
				ps2Write(PS2_ID_1);
				break;
			case PS2_SET_TYPEMATIC_RATE_DELAY:
				delay(1);
				ps2Write(PS2_ACK);
				data = ps2Read();
				SERIAL_DEBUG_PRINT("Set Typematic Rate/Delay ");
				SERIAL_DEBUG_PRINTLN_HEX(data);
				typematicDelayIntervall = pgm_read_word(&(typematicDelayIntervalls[(data & 0x60) >> 5]));
				typematicRepeatIntervall = pgm_read_word(&(typematicRepeatIntervalls[data & 0x1f]));
				break;
			case PS2_ENABLE:
				SERIAL_DEBUG_PRINTLN("Enable");
				delay(1);
				ps2Write(PS2_ACK);
				break;
			case PS2_DISABLE:
				SERIAL_DEBUG_PRINTLN("Disable");
				delay(1);
				ps2Write(PS2_ACK);
				break;
			case PS2_SET_DEFAULT:
				SERIAL_DEBUG_PRINTLN("Set Default");
				typematicDelayIntervall = DEFAULT_TYPEMATIC_DELAY_INTERVALL;
				typematicRepeatIntervall = DEFAULT_TYPEMATIC_REPEAT_INTERVALL;
				break;
			case PS2_SET_ALL_KEYS_TYPEMATIC:
				SERIAL_DEBUG_PRINTLN("Set All Keys Typematic");
				break;
			case PS2_SET_ALL_KEYS_MAKE_BREAK:
				SERIAL_DEBUG_PRINTLN("Set All Keys Make/Break");
				break;
			case PS2_SET_ALL_KEYS_MAKE:
				SERIAL_DEBUG_PRINTLN("Set All Keys Make");
				break;
			case PS2_SET_ALL_KEYS_TYPEMATIC_MAKE_BREAK:
				SERIAL_DEBUG_PRINTLN("Set All Keys Typematic/Make/Break");
				break;
			case PS2_SET_KEY_TYPE_TYPEMATIC:
				SERIAL_DEBUG_PRINTLN("Key Type Typematic");
				break;
			case PS2_SET_KEY_TYPE_MAKE_BREAK:
				SERIAL_DEBUG_PRINTLN("Key Type Make/break");
				break;
			case PS2_SET_KEY_TYPE_MAKE:
				SERIAL_DEBUG_PRINTLN("Key Type Make");
				break;
			case PS2_RESEND:
				SERIAL_DEBUG_PRINTLN("Resend");
				break;
			case PS2_RESET:
				SERIAL_DEBUG_PRINTLN("Reset");
				delay(1);
				ps2Write(PS2_ACK);
				delay(5);
				ps2Write(PS2_BAT_OK);
				break;
			default:
				break;
		}
	}

}

/**
 * Read from the serial interface and respond to any commands.
 */
void doSerialProtocol()
{
	if (Serial.available() > 0) {
		int cmd = Serial.read();
		switch (cmd) {
			case 'd': {
				// Display some current variable values.

				Serial.print("pressedKey = ");
				Serial.println(pressedKey, 16);
				Serial.print("debounceKey = ");
				Serial.println(debounceKey, 16);
				break;
			}
			case 's': {
				// Send a PS2 make code.

				char buf[3];
				while (Serial.available() == 0)
					;
				buf[0] = Serial.read();
				while (Serial.available() == 0)
					;
				buf[1] = Serial.read();
				buf[2] = '\0';
				int data;
				sscanf(buf, "%02x", &data);
				ps2Write(data);
				ps2Write(0xf0);
				ps2Write(data);
				break;
			}
			case 'r': {
				// Scan all columns and print the row values until a new
				// character is received.

				while (Serial.available() == 0) {
					for (uint8_t col = 0; col <= 9; ++col) {
						selectColumn(col);
						delayMicroseconds(COL_SELECT_DELAY_MICROS);
						uint8_t row = 255 - readRow();
						Serial.print(row >> 4, 16);
						Serial.print(row & 0x0f, 16);
						Serial.print(' ');
					}
					Serial.println();
					delay(500);
				}
				Serial.read();
				break;
			}
		}
	}

}

uint16_t getMakeCode()
{
	if (modifiers & MOD_LEFT_SHIFT) {
		return pgm_read_word(&(makeCodesShift[pressedKey]));
	} else {
		return pgm_read_word(&(makeCodes[pressedKey]));
	}
}

void sendModifiersMake()
{
	uint16_t makeCode = getMakeCode();
	if (makeCode != 0) {
		if (modifiers & MOD_CTRL) {
			ps2Write(0x14);
		}

		if (modifiers & MOD_RIGHT_SHIFT) {
			ps2Write(0x11);
		}

		if (makeCode & MAKE_WITH_SHIFT) {
			ps2Write(0x12);
		}

		if (makeCode & MAKE_WITH_ALTGR) {
			ps2Write(0xe0);
			ps2Write(0x11);
		}
	}
}

void sendModifiersBreak()
{
	uint16_t makeCode = getMakeCode();
	if (makeCode != 0) {
		if (makeCode & MAKE_WITH_ALTGR) {
			ps2Write(0xe0);
			ps2Write(0xf0);
			ps2Write(0x11);
		}

		if (makeCode & MAKE_WITH_SHIFT) {
			ps2Write(0xf0);
			ps2Write(0x12);
		}

		if (modifiers & MOD_RIGHT_SHIFT) {
			ps2Write(0xf0);
			ps2Write(0x11);
		}

		if (modifiers & MOD_CTRL) {
			ps2Write(0xf0);
			ps2Write(0x14);
		}
	}
}

void sendKeyMake()
{
	uint16_t makeCode = getMakeCode();
	if (makeCode != 0) {
		if (makeCode & MAKE_WITH_E0) {
			ps2Write(0xe0);
		}
		ps2Write((uint8_t) makeCode & 0x00ff);
	}
}

void sendKeyBreak()
{
	uint16_t makeCode = getMakeCode();
	if (makeCode != 0) {
		if (makeCode & MAKE_WITH_E0) {
			ps2Write(0xe0);
		}
		ps2Write(0xf0);
		ps2Write((uint8_t) makeCode & 0x00ff);
	}
}

void doKeyboard()
{
	if (pressedKey == NO_KEY) {
		if (debounceKey == NO_KEY) {
			debounceKey = scanKeyboard(&modifiers);
			if (debounceKey != NO_KEY) {
				debounceTimestamp = millis() + DEBOUNCE_DELAY_MILLIS;
			}
		} else {
			if (millis() > debounceTimestamp) {
				uint8_t key = scanKeyboard(&modifiers);
				if (key == debounceKey) {
					pressedKey = key;
#ifdef DEBUG
					Serial.print("Press: ");
					Serial.print(pressedKey >> 3, 10);
					Serial.print("/");
					Serial.print(pressedKey & 7, 10);
					Serial.print("/");
					Serial.println(modifiers, 16);
#endif
					sendModifiersMake();
					sendKeyMake();
					blinkLed();

					typematicTimestamp = millis() + typematicDelayIntervall;
				}
				debounceKey = NO_KEY;
			}
		}
	} else {
		if (scanKeyboard(NULL) == NO_KEY) {
#ifdef DEBUG
			Serial.print("Release: ");
			Serial.print(pressedKey >> 3, 10);
			Serial.print("/");
			Serial.println(pressedKey & 7, 10);
#endif
			sendKeyBreak();
			sendModifiersBreak();
			blinkLed();
			pressedKey = NO_KEY;
		} else {
			if (millis() > typematicTimestamp) {
				sendKeyMake();
				typematicTimestamp = millis() + typematicRepeatIntervall;
			}
		}
	}
}

/**
 * Main program entry point and execution loop.
 */
int main()
{
	init();
	setup();
	sei();
	while (1) {
		doKeyboard();
		doPs2Protocol();
		doSerialProtocol();
		calmDownLed();
	}
	return 0;
}
