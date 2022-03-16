# THIS REPO IS DEPRECATED

# inoklima
Arduino indoor climate.
Uses the breakout board sensors BME280, SGP30 and BH1750fvi.

## Measures
- Temperature (in Celcius)
- Humidity
- Pressure
- Dew Point
- Equivalent Sea Level Pressure
- Raw H2
- Raw Ethanol
- TVOC
- Lux

## Schematics
![alt text][logo]

[logo]: https://github.com/turbosnute/inoklima/blob/main/inoklima_figure.png
```
Connecting the BME280  +  SGP30  +  BJ1750 (i2c) Sensors:
Sensor              ->  Board
-----------------------------
Vin (Voltage In)    ->  3.3V or 5.0V
Gnd (Ground)        ->  Gnd
SDA (Serial Data)   ->  A4 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
SCK (Serial Clock)  ->  A5 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro
```



## Example output
```
{ "Sensors": [{ "Name": "BME280","Temp": 24.40 ,"Humidity": 25.80 ,"Pressure": 100126.52, "DewPoint": -5.79, "EquivSeaLvlPressure": 104263.52}, {"Name": "SGP30", "RawH2": 13536.00, "RawEthanol":18693.00, "TVOC": 0.00, "eCO2": 400.00}, {"Name": "BH1750FVI", "lux": 73.33}]}
```
