#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "00001";

struct Payload {
  char direction;
  int angle;
};

int lastAngle = -1;
const int angleThreshold = 3;
const int centerAngle = 90;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();  // Set as transmitter
  if (!radio.isChipConnected()) {
  Serial.println("NRF24L01 not connected!");
  while(1); // Halt program
} else {
  Serial.println("NRF24L01 connected.");
}
}

void loop() {
  int yVal = analogRead(A0);  // Y-axis
  int xVal = analogRead(A1);  // X-axis

  // Dead zone for direction control (Y-axis)
  char direction = 'S';  // Default to stop
  if (yVal > 570) direction = 'F';
  else if (yVal < 450) direction = 'R';

  // Dead zone for servo (X-axis)
  int angle;
  if (xVal > 470 && xVal < 550) {
    angle = centerAngle;
  } else {
    angle = map(xVal, 0, 1023, 45, 135);
  }

  // Only send if change is meaningful
  static char lastDirection = 'X';
  if (abs(angle - lastAngle) >= angleThreshold || direction != lastDirection) {
    Payload data = { direction, angle };
    bool success = radio.write(&data, sizeof(data));

    Serial.print("Sending: Direction = ");
    Serial.print(data.direction);
    Serial.print(", Angle = ");
    Serial.print(data.angle);
    Serial.print(" -> ");
    Serial.println(success ? "Success" : "Failed");

    lastAngle = angle;
    lastDirection = direction;
  }

  delay(50);
}