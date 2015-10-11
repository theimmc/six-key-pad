#include <EEPROM.h>

int button_pins[6] = { 19, 18, 8, 7, 15, 9 } ;
unsigned long pin_state[6] = { 0, 0, 0, 0, 0, 0 };

int key_code[2][6] = { { 0, KEY_UP_ARROW, KEY_RETURN , KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW },
                       { 0, 'w', ' ', 'a', 's', 'd' } };

int led_pin = 21;
int shift_status = 0;

#define LED_OFF 0
#define LED_ON  128

void setup() {
  // put your setup code here, to run once:
  int cnt;

  for (cnt = 0; cnt < 6; cnt++) {
    pinMode(button_pins[cnt], INPUT_PULLUP);
    digitalWrite(button_pins[cnt], HIGH);
  }

  pinMode(led_pin, OUTPUT);
  shift_status = EEPROM.read(0);
  if (shift_status != 1) shift_status = 0;
  EEPROM.write(0, shift_status);
  analogWrite(led_pin, (shift_status == 0 ? LED_OFF : LED_ON));
  
  Keyboard.begin();
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
      
    if (digitalRead(button_pins[cnt]) == HIGH) {
      if (pin_state[cnt] != 0) {
        if (key_code[shift_status][cnt] != 0) {
            Keyboard.release(key_code[shift_status][cnt]);
        }
        pin_state[cnt] = 0;
      }
    } else {
      if (pin_state[cnt] == 0) {
        if (key_code[shift_status][cnt] != 0) {
          Keyboard.press(key_code[shift_status][cnt]);
        } else {
          if (cnt == 0) {
            shift_status = 1 - shift_status;
            analogWrite(led_pin, (shift_status == 0 ? LED_OFF : LED_ON));
            EEPROM.write(0, shift_status);
          }
        }
        pin_state[cnt] = tick_now;
      }
    }
  }

  delay(1);
}
