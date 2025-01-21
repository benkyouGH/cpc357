# Automated Smart Agriculture System

The Automated Smart Agriculture System aligns with Sustainable Development Goal (SDG) 12: Responsible Consumption and Production.
Traditional agriculture systems suffer from inefficiencies due to manual labour and resource wastage.
Our project aims to address these issues by integrating IoT sensors and actuators to automate the crop management process.
For example, the IoT sensors monitor soil moisture and a pump actuator is activated to prevent the soil from being too dry.
This enables farmers to monitor their plant conditions in real-time and have finer control over their plants.
Therefore, by optimizing resource usage, we reduce the need for manual intervention from the farmer, and ensure optimal crop yields.

## System Architecture
![Architecture diagram](/img/architectureDiagram.png)

The proposed system architecture, shown in the figure above, consists of 4 main components: IoT sensors, actuators, ESP32 gateway, and the V-ONE cloud platform.
The sensors are to be deployed near the plants, and they monitor environmental parameters such as soil moisture, rain, sunlight, temperature and humidity.
The ESP32 acts as the gateway in this system, and after collecting sensor readings, it will trigger the actuators to perform an action based on certain conditions.
For example, when the ambient temperature exceeds 32 degrees celsius, the ESP32 will send a signal to turn on the relay to spin the fan.
When the temperature has dropped below the threshold, the fan would be turned off again.
By automating the crop management process, the farmer can manage optimal conditions for plant growth without manual intervention.

The following flowchart illustrates the different decisions the system would make based on set conditions.

![Flowchart](/img/flowchart.png)

Then, the ESP32 sends data to the V-ONE cloud platform through Wi-Fi using the MQTT protocol.
On the V-ONE cloud platform, the data is processed and stored, where it is then used to perform data visualization on the dashboard, and send notifications to the user.


## Components

![](/img/circuit-diagram.png)

The circuit diagram for wiring the project components are shown in the figure above.

The system consists of the following key hardware components:
- 1x NodeMCU ESP32s
- 1x Breadboard
- 1x DHT11 sensor
- 1x Rain sensor module
- 1x Soil moisture sensor module
- 1x ARD-T000090 TinkerKit LDR sensor module / Any other LDR
- 1x SG90 Micro Servo
- 1x 300C 1.5-6VDC 7mm Shaft Motor with fan blade
- 1x Micro Submersible Water Pump DC 3V-5V
- 1x LED
- 1x Resistor 0.25W 5% 220Ohm
- 2x 1CH Active H/L 3V OctoCoupler Relay Module
- Jumper cables

## Dependencies

- [Cytron IoT Kit V-ONE Library](https://github.com/CytronTechnologies/IoT-Kit-V-One/tree/main)
- [ESP32Servo Library](https://github.com/madhephaestus/ESP32Servo)
- [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)

## Installation

Clone the repository
```
git clone https://github.com/benkyouGH/cpc357.git
```

### Setting up V-ONE

Register your V-ONE account [here](https://cloud.v-one.my/).

#### Registering your gateway

Select "Device Manager" from the home page.
![](/img/setup-1.png)

Give a name to your gateway.
![](/img/setup-2.png)

Select on "View Info" to copy your gateway ID and access token.
![](/img/setup-3.png)

#### Configuring vonesetting.h

Configure your gateway information and Wi-Fi credentials in `vonesetting.h`

The header file can be located from the library installation directory.

On Unix: `$HOME/Documents/Arduino/libraries/IoT-Kit-V-One-main`

On Windows: `$HOME\Documents\Arduino\libraries\Iot-Kit-V-One-main`

vonesetting.h
```c
#if !defined (VONESETTING_H)
#define VONESETTING_H
#else
#error Multiple includes of vonesetting.h
#endif

#define WIFI_SSID "USMSecure"          //Replace this with YOUR WiFi SSID
#define WIFI_PASSWORD "verysecurepassword"  //Replace this with YOUR WiFi Password

#define MQTT_SERVER "mqtt.v-one.my"
#define MQTT_PORT 8883
#define MQTT_USERNAME "myaccesstoken"  //Replace this with the Access Token of YOUR gateway
#define MQTT_PASSWORD " "

#define GATEWAYID "mygatewayid"  //Replace this with the gatewayID of your gateway
#define INTERVAL 1000 //1S
#define INTERVAL2 500 //0.5S
```

#### Registering your devices

From "Device Manager", add all the devices needed for the project.

![](/img/devices-1.png)

Some of the components are not available from V-ONE, so you will have to create your own device type to add them.

![](/img/devices-2.png)

#### Configuring your devices

In the source code file, update the placeholders with the corresponding device IDs.

```c
/* V-ONE devices */
const char* LDRSensor = "DEVICE_ID_HERE";
const char* MoistureSensor = "DEVICE_ID_HERE";
const char* RainSensor = " DEVICE_ID_HERE";
const char* DHT11Sensor = "DEVICE_ID_HERE";

const char* FanRelayActuator = "DEVICE_ID_HERE";
const char* LEDActuator = "DEVICE_ID_HERE";
const char* PumpRelayActuator = "DEVICE_ID_HERE";
const char* ServoActuator = "DEVICE_ID_HERE";
```

Finally, flash the `project.ino` program to the ESP32, and if everything went correctly, the device will be connected to V-ONE cloud.

### Creating the dashboard

We can create our dashboards from the "Business Intelligence" menu.
To include our own charts in the dashboard, we first need to create them in the "Data Analysis" menu.

Select the data source for the chart. In this case, we will select our project sensors.
![Chart setup 1](/img/chart-setup-1.png)

Choose an appropriate chart to visualize the data.
Then, select the columns that would be used to plot the chart in the "Bind Data" section.
You can include other customizations such as titles, font sizes, and scales in the "Edit Chart" section if needed.
![Chart setup 2](/img/chart-setup-2.png)

When the chart has been created, navigate back to the "Dashboard" menu, and include the chart as a widget to import it into the dashboard.

Repeat the same steps for the remaining sensors.
The end result is shown below.

![Main dashboard](/img/dashboard.png)

### Implementing Callbacks

Although the actuators in our system have been automated to perform actions based on set conditions, we wanted to provide the user with an option to remotely activate the actuators if required.
In V-ONE, this can be implemented through callback functions. The callback function implementation is shown in the snippet below:

```c
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
```

To trigger the actuators remotely, we need to add the actuators to our Monitoring Dashboard.

Select "Monitoring Dashboard" under the AIoT Cloud menu, and create a new project dashboard.

Select "Add Control".
![Callback setup 1](/img/callback-setup-1.png)

Drag your actuator into the newly created "Control" object.
![Callback setup 2](/img/callback-setup-2.png)

Select button or slider for the actuator.
![Callback setup 3](/img/callback-setup-3.png)

Then, turn on the button, and the ESP32 should receive the callback, and trigger the actuator.

> The callbacks are a feature of the "Monitoring Dashboard", and cannot be implemented in the dashboard built under the "Business Intelligence" menu. Due to this limitation, we had to create 2 separate dashboards.

### Implementing Workflows

For our system, we implement workflows to send email notifications when sensor readings exceed their thresholds and the actuator has taken the appropriate action.

Workflows can be created from "Workflows" under the "Artificial Intelligence" menu.

The components needed to build the workflow is shown below.
![Workflow setup](/img/workflow.png)

Each data source represents one of the sensors, and we use python to process the data before sending the email.
The workflow python scripts can be found under the [`/workflows`](https://github.com/benkyouGH/cpc357/tree/main/workflows) directory.