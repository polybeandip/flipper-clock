#ifndef CONFIG_H

// Flipper Wiring
#define NUM_FLIPPERS 3
#define HOURS        0
#define TENS         1
#define ONES         2

struct FlipperConfig {
  // pins
  int in1;
  int in2;
  int enable;
  int home;
  
  // number of flips it takes to see the same flap again
  int recurrence; 

  int (*dataToPos)(int);
};

extern FlipperConfig configs[NUM_FLIPPERS];

#endif 
