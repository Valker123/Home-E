#ifndef ULTRASONIC_H
#define ULTRASONIC_H

const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

const int WALL_THRESHOLD_CM = 20;

void setupUltrasonic();
long readDistanceCm();
bool ObstacleAvoidance();

#endif