#include <SPI.h>
#include <MySensor.h>
#include <readVcc.h>

#define ANALOG_INPUT_MOISTURE 0   // Digital input did you attach your soil sensor.

#define STEPUP_PIN 2                     // StepUp Transistor 
#define CHILD_ID_Analog 0                   // Id of the sensor child
#define NODE_ID 23                          // ID of node

#define MIN_V 1900                          // empty voltage (0%)
#define MAX_V 3200                          // full voltage (100%)

MySensor gw;
unsigned long SLEEP_TIME = 30 * 60000;       // sleep time between reads

MyMessage msgAnalog(CHILD_ID_Analog, V_LIGHT_LEVEL);

int lastsoilValueAnalog = -1;
int oldBatteryPcnt;
int repeat = 20;

void setup()
{
  gw.begin(NULL, NODE_ID, false);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Plant Sensor", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_Analog, S_LIGHT_LEVEL);
}

void loop()
{
  Serial.println("Waking up...");
  stepup(true);
  delay(50);
  readMoistureAnalog();
  stepup(false);
  sendBattery();

  Serial.println("Going to sleep...");
  Serial.println("");
  gw.sleep(SLEEP_TIME);
}

void stepup(boolean onoff)
{
  pinMode(STEPUP_PIN, OUTPUT);      // sets the pin as output
  Serial.print("---------- StepUp: ");
  if (onoff == true)
  {
    Serial.println("ON");
    digitalWrite(STEPUP_PIN, LOW);      // turn on
  }
  else
  {
    Serial.println("OFF");
    digitalWrite(STEPUP_PIN, HIGH);     // turn off
  }
}

void readMoistureAnalog()
{
  // Read analog soil value
  int soilValueAnalog = analogRead(ANALOG_INPUT_MOISTURE);
  soilValueAnalog = map(soilValueAnalog, 0, 1024, 100, 0);

  if (soilValueAnalog != lastsoilValueAnalog)
  {
    resend(msgAnalog.set(soilValueAnalog), repeat);  // Send the inverse to gw as tripped should be when no water in soil
    lastsoilValueAnalog = soilValueAnalog;
  }
  Serial.print("---------- Moisture level : ");
  Serial.println(soilValueAnalog);
}

void resend(MyMessage &msg, int repeats)
{
  int repeat = 1;
  int repeatdelay = 0;
  boolean sendOK = false;

  while ((sendOK == false) and (repeat < repeats)) {
    if (gw.send(msg)) {
      sendOK = true;
    } else {
      sendOK = false;
      Serial.print("Send ERROR ");
      Serial.println(repeat);
      repeatdelay += 250;
    } repeat++; delay(repeatdelay);
  }
}

void sendBattery() // Measure battery
{
  int batteryPcnt = min(map(readVcc(), MIN_V, MAX_V, 0, 100), 100);
  if (batteryPcnt != oldBatteryPcnt) {
    gw.sendBatteryLevel(batteryPcnt); // Send battery percentage
    oldBatteryPcnt = batteryPcnt;
  }
  Serial.print("---------- Battery: ");
  Serial.println(batteryPcnt);
}
