#ifndef SENSORS_H
#define SENSORS_H

extern int pirState;  // Current state of the PIR sensor
extern unsigned long lastMotionTime;  // Last time motion was detected
extern bool inIdleAnimation;  // Whether the device is in idle animation mode
extern const unsigned long idleTimeout;  // Timeout for entering idle animation

#include <Arduino.h>

void setupMotionSensor();
void motionSensorLoop();

#endif
