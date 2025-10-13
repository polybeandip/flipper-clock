#include <WiFi.h>
#include "time.h"

#define FLIPPERS_SIZE sizeof(flippers)/sizeof(flippers[0])

// WiFi info
const char* ssid = "";
const char* password = "";

// Time holders
int hour;
int minutes;
enum stateType {NORMAL, TIMEDROP, STALLTIME};
enum stateType state;

struct Flipper {
  String type;
  int in1;
  int in2;
  int enable;
  int home;
  byte flipState;
  int flipAmount;
  int flapPosition;
};

// Pin data
Flipper flippers[] = {
  {"Hours", 4, 15, 2, 33, LOW, 0, 0},
  {"Tens", 23, 22, 21, 32, LOW, 0, 0},
  {"Ones", 18, 5, 19, 35, LOW, 0, 0}
};

// State and Flap data
byte latchTime = 150;

// Going home phase
int initialHomeBool = 1;

// TimeTable Index
int currentHour;
int currentMinutes;

// Displayed TimeTable Index
int displayedTrain = 0;


int mod(int x, int n) {
  int rem = x % n;
  if (rem < 0) {
    rem += n;
  }
  return rem;
}


void updateTime() {
  struct tm currentTime;
  getLocalTime(&currentTime);
  hour = currentTime.tm_hour;
  minutes = currentTime.tm_min;
}


void setup() {
  for (int i = 0; i < FLIPPERS_SIZE; i++) {
    pinMode(flippers[i].in1, OUTPUT);
    pinMode(flippers[i].in2, OUTPUT);
    pinMode(flippers[i].enable, OUTPUT);
    pinMode(flippers[i].home, INPUT);
  }

  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
  }
  // Serial.begin(9600);

  configTime(0, 0, "pool.ntp.org");
  setenv("TZ","EST5EDT",1);
  tzset();
}

void loop() {
  updateTime();

  // All Flippers go Home when we start
  if (initialHomeBool) {
    int allHome = goHomeTick();
    if (allHome) {
      initialHomeBool = 0;
    }
  }
  // Move All Flippers to new Positions
  else if (hour != currentHour || minutes != currentMinutes) {
    flippers[0].flipAmount = mod(hour - flippers[0].flapPosition + 1, 60);
    flippers[1].flipAmount = mod(minutes / 10 - flippers[1].flapPosition + 1, 60);
    flippers[2].flipAmount = mod(minutes % 10 - flippers[2].flapPosition + 1, 60);

    int haveFlips = 1;
    while (haveFlips) {
      haveFlips = goNewPositionTick();
    }
    currentHour = hour;
    currentMinutes = minutes;
  }
}


int goNewPositionTick() {
  // Moves all Flipers towards the New Position
  int noFlips = FLIPPERS_SIZE;
  enableAllFlippers();
  for (int i = 0; i < FLIPPERS_SIZE; i++) {
    if (flippers[i].flipAmount) {
      singleFlip(i);
      flippers[i].flipAmount--;
      flippers[i].flapPosition = mod(flippers[i].flapPosition + 1, 60);
    }
    else {
      noFlips--;
    }
  }
  disableAllFlippers();
  return noFlips;
}


int goHomeTick() {
  // Moves all Flipers towards the Home Position
  int allHome = 0;
  enableAllFlippers();
  for (int i = 0; i < FLIPPERS_SIZE; i++) {
    if (digitalRead(flippers[i].home)) {
      singleFlip(i);
    }
    else {
      allHome++;
    }
  }
  disableAllFlippers();
  return allHome == FLIPPERS_SIZE;
}

void enableAllFlippers() {
  // Sets the enable pin HIGH for all flippers
  for (int i = 0; i < FLIPPERS_SIZE; i++) {
    digitalWrite(flippers[i].enable, HIGH);
  }
}

void disableAllFlippers() {
  // Delays for `latchTime` ms and then sets the enable pin LOW for all flippers
  delay(latchTime);
  for (int i = 0; i < FLIPPERS_SIZE; i++) {
    digitalWrite(flippers[i].enable, LOW);
  }
}

void singleFlip(int index) {
  // Advances a Flipper by a Single Flip
  digitalWrite(flippers[index].in1, flippers[index].flipState);
  digitalWrite(flippers[index].in2, !flippers[index].flipState);

  flippers[index].flipState = flippers[index].flipState == HIGH ? LOW : HIGH;
}
