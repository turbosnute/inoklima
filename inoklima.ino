/*

INOKLIMA:

Libraries
- Adafruit SGP30 Sensor by Adafruit
- BME280 by Tyler Glenn <protglenn@gmail.com> Version 2.3.0
- BH1750 by Christopher Laws Version 1.1.4

Connecting the BME280  +  SGP30  +  BJ1750 (i2c) Sensors:
Sensor              ->  Board
-----------------------------
Vin (Voltage In)    ->  3.3V
Gnd (Ground)        ->  Gnd
SDA (Serial Data)   ->  A4 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
SCK (Serial Clock)  ->  A5 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro

*/

#include <EnvironmentCalculations.h>
#include <BME280I2C.h>
#include "Adafruit_SGP30.h"
#include <Wire.h>
#include <BH1750.h>

#define SERIAL_BAUD 115200

BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,

Adafruit_SGP30 sgp;

BH1750 lightMeter;

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f  +  temperature)) / (273.15f  +  temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup()
{
  Serial.begin(SERIAL_BAUD);

  while(!Serial) {} // Wait

  Wire.begin();

  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(3000);
  }

  // bme.chipID(); // Deprecated. See chipModel().
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }

  if (! sgp.begin()){
    Serial.println("SGP30 Sensor not found :(");
    while (1);
  }

  lightMeter.begin();

  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!

}

//////////////////////////////////////////////////////////////////
int counter = 0;

void loop()
{
   printData(&Serial);
   delay(1000);
}

//////////////////////////////////////////////////////////////////

void printData
(
   Stream* client
)
{
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  EnvironmentCalculations::AltitudeUnit envAltUnit  =  EnvironmentCalculations::AltitudeUnit_Meters;
  EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;

  float altitude = EnvironmentCalculations::Altitude(pres, envAltUnit);
  float dewPoint = EnvironmentCalculations::DewPoint(temp, hum, envTempUnit);
  float seaLevel = EnvironmentCalculations::EquivalentSeaLevelPressure(altitude, temp, pres);
  //client->print(",\"Altitude:\"");
  //client->print(altitude);

  sgp.setHumidity(getAbsoluteHumidity(temp, hum)); //set the absolute humidity to enable the humditiy compensation for the air quality signals

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }

  float tvoc = sgp.TVOC;
  float eco2 = sgp.eCO2;

  if (! sgp.IAQmeasureRaw()) {
     Serial.println("Raw Measurement failed");
     return;
  }

  float rawh2 = sgp.rawH2;
  float rawethanol = sgp.rawEthanol;

  delay(100);

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      //Serial.println("Failed to get baseline readings");
      return;
    }

  }

  delay(100);

  float lux = lightMeter.readLightLevel();

  client->print("{\"Sensors\": [{ \"Name\": \"BME280\", \"Temp\": " + String(temp) + " ,\"Humidity\": " + String(hum) + " ,\"Pressure\": " + String(pres));
  client->print(", \"DewPoint\": " + String(dewPoint) + ", \"EquivSeaLvlPressure\": " + String(seaLevel) + "}, {\"Name\": \"SGP30\", ");
  client->print("\"RawH2\": " + String(rawh2) + ", \"RawEthanol\":" + String(rawethanol) + ", \"TVOC\": "  +  String(tvoc) + ", \"eCO2\": " + String(eco2) + "}, ");
  client->print("{\"Name\": \"BH1750FVI\", \"lux\": " + String(lux) + " }]}");
  client->println();
}
