#ifndef MOTORS_H
#define MOTORS_H


extern const int ENA;
extern const int ENB;
extern const int IN1;
extern const int IN2;
extern const int IN3;
extern const int IN4;
extern const int leftMotorChannel;
extern const int rightMotorChannel;

void setupMotors();
void setMotorSpeed(int leftSpeed, int rightSpeed);
void stopMotors();
void driveForward(int speed);
void driveBackward(int speed);
void turnLeft(int speed);
void turnRight(int speed);

#endif

