
#include <Streaming.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// LED
int ledPin = 0; //D3


// Servo
Servo myservo;

// MQ-3 Sensor
#define WARMUP 900 //15 minutes of warm up the sensor!
#define DRIVE_LIMIT 0.05 // 50mg/dL OR 0.05g/dL

// OLED
#define OLED_RESET -1
#define OLED_SCREEN_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(OLED_RESET);

#define DEBUG 0

void setup()
{

  Serial.begin(9600);
  Serial << endl << "Hello World" << endl;

  pinMode(ledPin, OUTPUT);

  //Servo
  myservo.attach(D8);
  myservo.write(170); //Normal Face position / Initial position

  //OLED
  display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS);

  display.display();
  delay(2000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1); // - a line is 21 chars in this size && 2 - 10
  display.setTextColor(WHITE, BLACK);

  warmupSensor();
  display.clearDisplay();
  printFrame();

  printTitle();
}

void loop()
{
  int ppm = readValue();
  float bac = convert2BAC(ppm);

  printBAC(bac);

  switchLED(aboveLimit(bac));

  moveServo(bac);
}

void warmupSensor() {
  unsigned int time = millis() / 1000; //seconds passed since program started

  unsigned int rectWidth = 120;

  display.setTextSize(1);
  display.setCursor(3, 2);
  display << "Warming up Sensor..." << endl;
  display.display();

  if (DEBUG) {
    Serial << "Begining of warm up..." << endl;
  }

  while (time <= WARMUP) {

    display.drawRect(4, 20, rectWidth, 10, WHITE);
    display.fillRect(4, 20, rectWidth - ((WARMUP - time)*rectWidth / WARMUP), 10, WHITE); //Progress bar
    display.display();

    time = millis() / 1000;//seconds passed since program started

    if (DEBUG) Serial << "Warming up " << time << endl;
  }

  if (DEBUG) Serial << "Warm up finished" << endl;
}

void printFrame() {

  if (DEBUG) Serial << "Drawing rect..." << endl;
  delay(1000);
  display.drawRoundRect(0, 0, 128, 32, 3, WHITE);
  display.display();
}

void printTitle() {
  // display "BAC:"
  display.setTextSize(2);
  display.setCursor(47, 5);
  display << "BAC:" << endl;
  display.display();
}

int readValue() {
  int x[5] = {0};

  for (uint8_t i = 0; i < 5; i++) { //Reading 5 times
    x[i] = analogRead(0);
    delay(100);
  }

  if (DEBUG) Serial << "Reading: " << (x[0] + x[1] + x[2] + x[3] + x[4]) / 5 << endl;

  int val = ((x[0] + x[1] + x[2] + x[3] + x[4]) / 5) - 150; //Calculating the mean value and subtracting 150 to avoid false positives

  if (val < 0) val = 0;

  if (DEBUG) Serial << "Returning: " << val << endl;

  return val;
}

float convert2BAC(int val) {
  //convert ppm to BAC
  if (DEBUG) Serial << "PPM to BAC: " << val / 2600.0 << endl;
  return val / 2600.0; // BAC = parts.per.million / 2600 (Source: https://sgx.cdistore.com/datasheets/sgx/an4-using-mics-sensors-for-alcohol-detection1.pdf)
}

void printBAC(float val) {
  // display the value read

  if (DEBUG) Serial << "Displaying BAC..." << endl;

  display.setTextSize(1);

  display.setCursor(50, 23);
  display << "     " << endl;

  display.setCursor(50, 23);
  display.println(val, 3);

  display.setCursor(85, 23);
  display << "%" << endl;

  display.display();
}

bool aboveLimit(float val) {
  if (val > DRIVE_LIMIT) return true; //If BAC is greater than the Drive Limit in Scotland
  else return false;
}

void switchLED(bool above) {
  if (above) digitalWrite(ledPin, HIGH); // turn on LED
  else digitalWrite(ledPin, LOW); // turn off LED
}

void moveServo(float val) {
  if (val < DRIVE_LIMIT) myservo.write(170); //Normal Face position
  else if (val >= DRIVE_LIMIT && val <= 2 * DRIVE_LIMIT) myservo.write(100); //Tipsy Face position
  else myservo.write(30); //myservo.write(45); //Drunk Face position
}
