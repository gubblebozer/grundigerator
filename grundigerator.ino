#include <Adafruit_DS3502.h>
#include <stdint.h>

#define SETTLE_MS 200

typedef enum g_pins {
    G_SE1     = 2,
    G_SE_GND  = 3,
    G_BE1_BAR = 4,
    G_BE1     = 5,
    G_BUTTON  = 12,
} g_pins;

typedef void (*input_fn)(void);

int b_val = HIGH; // pulled up internally
int pos = 0;
uint32_t last_ms = 0;

void
setup(void)
{
  pinMode(G_SE1, OUTPUT);
  pinMode(G_BE1_BAR, OUTPUT);
  pinMode(G_BE1, OUTPUT);
  pinMode(G_SE_GND, OUTPUT);
  pinMode(G_BUTTON, INPUT);
  digitalWrite(G_BE1, HIGH);
}

void
select_se1(void)
{
    digitalWrite(G_BE1_BAR, HIGH);
    digitalWrite(G_BE1, LOW);
    digitalWrite(G_SE1, HIGH);
    digitalWrite(G_SE_GND, HIGH);
}

void
select_be1(void)
{
    // make, then break
    digitalWrite(G_BE1_BAR, LOW);
    digitalWrite(G_BE1, HIGH);
    digitalWrite(G_SE_GND, LOW);
    digitalWrite(G_SE1, LOW);
}

#define INPUTS_N 2
input_fn inputs[INPUTS_N] = {
    select_be1,
    select_se1
};

typedef enum bstate {
  BS_OFF,
  BS_WAIT,
  BS_PRESSED
} bstate;

int
debounce_button(void)
{
  int b = !digitalRead(G_BUTTON); // button is normally HIGH
  static bstate s = BS_OFF;
  static uint32_t ms_start = 0;

  // releasing button always transitions to off, short-circuiting rest of logic
  if (!b) {
    s = BS_OFF;
    return 0;
  }

  switch (s) {
    case BS_OFF:
      s = BS_WAIT;
      ms_start = millis();
      break;
    case BS_WAIT:
      if (ms_start + SETTLE_MS < millis()) {
        s = BS_PRESSED;
        return 1; // only return 1 on edge of transition to PRESSED
      }
      break;
    case BS_PRESSED:
      break;
  }

  return 0;
}

void
loop(void)
{
  int last_pos = pos;
  
  int pressed = debounce_button();
  if (pressed) {
    if (++pos >= INPUTS_N)
        pos = 0;
  }

  if (pos != last_pos)
      inputs[pos]();
}
