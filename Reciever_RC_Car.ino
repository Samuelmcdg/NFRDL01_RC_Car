#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define IN1 4
#define IN2 5
#define EN  6
#define SERVO_PIN 9

RF24 radio(2, 3);  // CE, CSN
Servo myServo;

const byte address[6] = "00001";

struct Payload {
  char direction;
  int angle;
};

Payload receivedData;

char currentDirection = 'S';
int currentAngle = 90;
const int angleThreshold = 3;

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN, OUTPUT);

  stopMotor();

  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);  // Center at startup

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  if (!radio.isChipConnected()) {
  Serial.println("NRF24L01 not connected!");
  while(1); // Halt program
} else {
  Serial.println("NRF24L01 connected.");
}
}

void loop() {
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));

    Serial.print("Direction: ");
    Serial.print(receivedData.direction);
    Serial.print(" | Angle: ");
    Serial.println(receivedData.angle);

    // Update motor direction if changed
    if (receivedData.direction != currentDirection) {
      currentDirection = receivedData.direction;

      switch (currentDirection) {
        case 'F':
          moveForward();
          break;
        case 'R':
          moveReverse();
          break;
        case 'S':
        default:
          stopMotor();
          break;
      }
    }

    // Update steering angle if changed significantly
    int safeAngle = constrain(receivedData.angle, 60, 120);
    if (abs(safeAngle - currentAngle) >= angleThreshold) {
      myServo.write(safeAngle);
      currentAngle = safeAngle;
    }
  }
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(EN, 200);
}

void moveReverse() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(EN, 200);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(EN, 0);
}