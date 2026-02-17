#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;


#define THERMISTOR_PIN 34
#define RXD2 16
#define TXD2 17

HardwareSerial sim800(2);


#define SERIES_RESISTOR 10000
#define NOMINAL_RESISTANCE 10000
#define NOMINAL_TEMPERATURE 25
#define B_COEFFICIENT 3950


float tempWarning = 40;   // °C
float tempFault   = 60;   // °C

float vibWarning  = 1.5;  // g
float vibFault    = 2.5;  // g

String lastStatus = "";

void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, RXD2, TXD2);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }

  Serial.println("System Started...");
}

void loop() {

 
  int adcValue = analogRead(THERMISTOR_PIN);
  float voltage = adcValue * 3.3 / 4095.0;

  
  if (voltage <= 0.01) voltage = 0.01;

  //float resistance = SERIES_RESISTOR * (3.3 / voltage - 1);
  float resistance = SERIES_RESISTOR * voltage / (3.3 - voltage);
  float steinhart;
  steinhart = resistance / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
  steinhart = 1.0 / steinhart;

  float temperature = steinhart - 273.15;


  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float vibration = sqrt(a.acceleration.x * a.acceleration.x +
                         a.acceleration.y * a.acceleration.y +
                         a.acceleration.z * a.acceleration.z) / 9.81;

 
  Serial.println("---------------------------");
  Serial.print("Temperature (C): ");
  Serial.println(temperature);

  Serial.print("Vibration (g): ");
  Serial.println(vibration);

  String currentStatus = "NORMAL";

  if (temperature > tempFault || vibration > vibFault) {
    currentStatus = "FAULT";
  }
  else if (temperature > tempWarning || vibration > vibWarning) {
    currentStatus = "WARNING";
  }

  Serial.print("System Status: ");
  Serial.println(currentStatus);

  if (currentStatus != lastStatus) {
    sendSMS(currentStatus, temperature, vibration);
    lastStatus = currentStatus;
  }

  delay(2000);
}

void sendSMS(String status, float temp, float vib) {

  String message = "Status: " + status +
                   "\nTemp: " + String(temp) + " C" +
                   "\nVib: " + String(vib) + " g";

  Serial.println("Sending SMS...");
  Serial.println(message);

  // Simulated AT commands (Wokwi)
  sim800.println("AT");
  delay(300);

  sim800.println("AT+CMGF=1");
  delay(300);

  sim800.println("AT+CMGS=\"+947XXXXXXXX\"");
  delay(300);

  sim800.print(message);
  delay(300);

  sim800.write(26); // CTRL+Z

  Serial.println("SMS Sent (Simulated)");
}