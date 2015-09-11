#include <EEPROM.h>

int buttonPins[6] = { 4, 5, 6, 7, 8, 9 } ;
unsigned long pin_state[6] = { 0, 0, 0, 0, 0, 0 };
int keycode[2][6] = { { 0, KEY_UP_ARROW, 0, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW },
                      { 0, 'w', 0, 'a', 's', 'd' } };

int ledPin = 3;
int shift_status = 0;

void setup() {
  // put your setup code here, to run once:
  int cnt;

  for (cnt = 0; cnt < 6; cnt++) {
    pinMode(buttonPins[cnt], INPUT_PULLUP);
    digitalWrite(buttonPins[cnt], HIGH);
  }

  pinMode(ledPin, OUTPUT);
  shift_status = EEPROM.read(0);
  if (shift_status != 1) shift_status = 0;
  EEPROM.write(0, shift_status);
  digitalWrite(ledPin, (shift_status == 0 ? LOW : HIGH));
  
  Keyboard.begin();
//  Serial.begin(9600);
//  Serial.println("Hello, world!");
}

bool debounce(unsigned long t_now, unsigned long t_prev) {
  unsigned long diff;

  diff = t_now - t_prev; // need to check for underflow?

  if (diff <= 20) return true;
  else return false;
}

void loop() {
  unsigned long tick_now = millis();
  int cnt;

  // since we use non zero to indicate pressed state, we need
  // to handle the edge case where millis() returns 0
  
  if (tick_now == 0) tick_now = 1;

  for (cnt = 0; cnt < 6; cnt++) {
    int key_state = 0;

    // ignore state change for pin if in debounce period
    if (pin_state[cnt] != 0)
      if (debounce(tick_now, pin_state[cnt]) == true)
        continue;
      
    if (digitalRead(buttonPins[cnt]) == HIGH) {
      if (pin_state[cnt] != 0) {
        if (keycode[shift_status][cnt] != 0) {
            Keyboard.release(keycode[shift_status][cnt]);
        }
        pin_state[cnt] = 0;
      }
    } else {
      if (pin_state[cnt] == 0) {
//        char outstring[64];
//        sprintf(outstring, "Key %d pressed", cnt);
//        Serial.println(outstring);
        if (keycode[shift_status][cnt] != 0) {
          Keyboard.press(keycode[shift_status][cnt]);
        } else {
          if (cnt == 0) {
            shift_status = 1 - shift_status;
            digitalWrite(ledPin, (shift_status == 0 ? LOW : HIGH));
            EEPROM.write(0, shift_status);
          }
        }
        pin_state[cnt] = tick_now;
      }
    }
  }

  delay(1);
}
