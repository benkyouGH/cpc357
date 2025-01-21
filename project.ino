#include "VOneMqttClient.h"
#include "DHT.h"
#include <ESP32Servo.h>

/* Defines */
#define DHTTYPE DHT11
#define DHT11_PIN 21
#define MOISTURE_PIN 36
#define RAINSENSOR_PIN 26
#define RAINPOWER_PIN 0
#define LDR_PIN 32
#define LED_PIN 16
#define BRIGHTNESS_THRESHOLD 500
#define FAN_RELAY 23
#define PUMP_RELAY 22
#define SERVO_PIN 19

/* V-ONE devices */
const char* LDRSensor = "893855ed-2066-4e58-abac-0b84f4f10b4c";
const char* MoistureSensor = "22f9594f-78c6-49ed-ae78-cde0eaf154f8";
const char* RainSensor = "3e6b81c8-e604-4d36-95f3-39d1126fcc75";
const char* DHT11Sensor = "b8995945-f990-4ac7-a327-9f46e0b0bf2a";

const char* FanRelayActuator = "125a4cbc-5354-4b3a-a8e6-ecfd8f4c8ee9";
const char* LEDActuator = "f11c3763-f307-444d-ade7-0fdf244f7ba4";
const char* PumpRelayActuator = "04d2a2e5-9c9b-4aa4-81f4-81ced132bc45";
const char* ServoActuator = "ec393cf2-e4cc-4556-a249-8913ba888235";

DHT dht(DHT11_PIN, DHTTYPE);
Servo servo;

//Create an instance of VOneMqttClient
VOneMqttClient voneClient;

//last message time
unsigned long lastMsgTime = 0;

int MinMoistureValue = 4095;
int MaxMoistureValue = 0;
int MinMoisture = 0;
int MaxMoisture = 100;
int Moisture = 0;

bool isRoofClosed = false;

bool fanState = false;
bool pumpState = false;
bool ledState = false;
int servoAngle = 0;

void triggerActuator_callback(const char* actuatorDeviceId, const char* actuatorCommand) {
  Serial.print("Main received callback : ");
  Serial.print(actuatorDeviceId);
  Serial.print(" : ");
  Serial.println(actuatorCommand);

  String errorMsg = "";

  JSONVar commandObjct = JSON.parse(actuatorCommand);
  JSONVar keys = commandObjct.keys();

  if (keys.length() != 1) {
    errorMsg = "Invalid command format";
    voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, errorMsg.c_str(), false);
    return;
  }

  String key = (const char*)keys[0];
  JSONVar commandValue = commandObjct[key];

  Serial.print("Key : ");
  Serial.println(key.c_str());
  Serial.print("Value : ");
  Serial.println(commandValue);

  if (String(actuatorDeviceId) == ServoActuator) {
    servoAngle = (int)commandValue;
    servo.write(servoAngle);
  } else if (String(actuatorDeviceId) == FanRelayActuator) {
    fanState = (bool)commandValue;
    digitalWrite(FAN_RELAY, fanState ? HIGH : LOW);
  } else if (String(actuatorDeviceId) == LEDActuator) {
    ledState = (bool)commandValue;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  } else if (String(actuatorDeviceId) == PumpRelayActuator) {
    pumpState = (bool)commandValue;
    digitalWrite(PUMP_RELAY, pumpState ? HIGH : LOW);
  } else {
    Serial.print("No actuator found : ");
    Serial.println(actuatorDeviceId);
    errorMsg = "No actuator found";
    voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, errorMsg.c_str(), false);
    return;
  }

  voneClient.publishActuatorStatusEvent(actuatorDeviceId, actuatorCommand, errorMsg.c_str(), true);
}

int read_rainsensor() {
  digitalWrite(RAINPOWER_PIN, HIGH);
  delay(100);
  int isRaining = digitalRead(RAINSENSOR_PIN);
  delay(100);
  digitalWrite(RAINPOWER_PIN, LOW);
  return isRaining;
}

void setup_wifi() {
  delay(10);
    // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SERVO_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);

  pinMode(FAN_RELAY, OUTPUT);
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RAINSENSOR_PIN, INPUT);
  pinMode(RAINPOWER_PIN, OUTPUT);
  // Initially keep the rain sensor off
  digitalWrite(RAINPOWER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  dht.begin();
  setup_wifi();
  voneClient.setup();
  voneClient.registerActuatorCallback(triggerActuator_callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!voneClient.connected()) {
    voneClient.reconnect();
    String errorMsg = "Something went wrong";
    voneClient.publishDeviceStatusEvent(DHT11Sensor, true);
    voneClient.publishDeviceStatusEvent(MoistureSensor, true);
    voneClient.publishDeviceStatusEvent(RainSensor, true);
  }
  voneClient.loop();

  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    /* Soil Moisture */
    int moistureValue = analogRead(MOISTURE_PIN);
    Moisture = map(moistureValue, MinMoistureValue, MaxMoistureValue, MinMoisture, MaxMoisture);
    
    Serial.printf("Moisture value: %d\n", moistureValue);
    if (Moisture < 40 && !pumpState) {
      digitalWrite(PUMP_RELAY, HIGH);
      delay(1000);
    } else if (Moisture >= 40 && !pumpState) {
      digitalWrite(PUMP_RELAY, LOW);
    }
    /* DHT11 */
    delay(10);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (t > 32 && !fanState) {
      digitalWrite(FAN_RELAY, HIGH);
    } else if (t <= 32 && !fanState) {
      digitalWrite(FAN_RELAY, LOW);
    }

    /* Rain sensor */
    delay(10); // short delay before reading
    int isRaining = !digitalRead(RAINSENSOR_PIN);
    if (isRaining && !isRoofClosed && servoAngle != 180) {
      // If raining, close the roof
      servo.write(180);
      isRoofClosed = true;
    } else if (!isRaining && isRoofClosed && servoAngle != 0) {
      // Not raining, open the roof
      servo.write(0);
      isRoofClosed = false;
    }

    /* LDR sensor */
    delay(10); // Short delay before reading
    int LDRValue = analogRead(LDR_PIN);
    if (LDRValue < BRIGHTNESS_THRESHOLD && !ledState) {
      digitalWrite(LED_PIN, HIGH);
    } else if (LDRValue >= BRIGHTNESS_THRESHOLD && !ledState) {
      digitalWrite(LED_PIN, LOW);
    }

    //Publish telemtry data
    voneClient.publishTelemetryData(MoistureSensor, "Soil moisture", Moisture);
    JSONVar payloadObject;
    payloadObject["Humidity"] = h;
    payloadObject["Temperature"] = t;
    voneClient.publishTelemetryData(DHT11Sensor, payloadObject);
    voneClient.publishTelemetryData(RainSensor, "Raining", isRaining);
    voneClient.publishTelemetryData(LDRSensor, "brightness", LDRValue);
  }
  // delay 30 seconds
  delay(1000);
}
