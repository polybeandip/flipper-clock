#include "config.h"
#include <WiFi.h>

#define NUM_FLAPS  60
#define LATCH_TIME 150

enum state_t {GO_HOME, FLIP, SHOW_TIME};
enum state_t state = GO_HOME;

int currHour;
int currMin;
int prevMin;

struct Flipper {
  byte flipState    = LOW;
  int  flipAmount   = 0;
  int  flapPosition = 0;
};

Flipper flippers[NUM_FLIPPERS];


void updateTime() {
  struct tm time;
  getLocalTime(&time);
  currHour = time.tm_hour;
  currMin  = time.tm_min;
}


void setup() {
  for (int i = 0; i < NUM_FLIPPERS; i++) {
    pinMode(configs[i].in1, OUTPUT);
    pinMode(configs[i].in2, OUTPUT);
    pinMode(configs[i].enable, OUTPUT);
    pinMode(configs[i].home, INPUT);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
  }

  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "EST5EDT", 1);
  tzset();
}


void loop() {
  updateTime();

  switch (state) {
    // Moves all flippers towards the home position
    case GO_HOME: {
      int allHome = 0;

      enableAllFlippers();
      for (int i = 0; i < NUM_FLIPPERS; i++) {
        if (digitalRead(configs[i].home))
          singleFlip(i);
        else 
          allHome++;
      }
      disableAllFlippers();
      
      if (allHome == NUM_FLIPPERS) state = SHOW_TIME;
      break;
    }

    // Move all flippers towards the "new position" (i.e. where flipAmount points)
    case FLIP: {
      int hasFlips = NUM_FLIPPERS;

      enableAllFlippers();
      for (int i = 0; i < NUM_FLIPPERS; i++) {
        if (flippers[i].flipAmount) {
          singleFlip(i);
          flippers[i].flipAmount--;
          flippers[i].flapPosition = flippers[i].flapPosition + 1 % NUM_FLAPS;
        }
        else
          hasFlips--;
      }
      disableAllFlippers();
      
      if (!hasFlips) {
        state = SHOW_TIME;
        prevMin = currMin;
      }
      break;
    }

    // Compute "new positions" for all flippers if on a new minute
    case SHOW_TIME: {
      if (currMin == prevMin) break;

      int targets[NUM_FLIPPERS];
      targets[HOURS] = currHour;
      targets[TENS]  = currMin / 10;
      targets[ONES]  = currMin % 10;

      for (int i = 0; i < NUM_FLIPPERS; i++) {
        int target = configs[i].dataToPos(targets[i]);

        flippers[i].flipAmount = (target - flippers[i].flapPosition) % configs[i].recurrence;
        if (flippers[i].flipAmount < 0)
          flippers[i].flipAmount += configs[i].recurrence;
      }

      state = FLIP;
      break;
    }
  }
}


void enableAllFlippers() {
  // Sets the enable pin HIGH for all flippers
  for (int i = 0; i < NUM_FLIPPERS; i++) {
    digitalWrite(configs[i].enable, HIGH);
  }
}


void disableAllFlippers() {
  // Delays for `LATCH_TIME` ms and then sets the enable pin LOW for all flippers
  delay(LATCH_TIME);
  for (int i = 0; i < NUM_FLIPPERS; i++) {
    digitalWrite(configs[i].enable, LOW);
  }
}


void singleFlip(int index) {
  // Advances flipper `index` by one flip 
  digitalWrite(configs[index].in1, flippers[index].flipState);
  digitalWrite(configs[index].in2, !flippers[index].flipState);

  flippers[index].flipState = !flippers[index].flipState;
}
