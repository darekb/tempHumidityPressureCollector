# tempHumidityPressureCollector
part of WatherStationProject
Get data from BME280, convert to readable data and send via RF module to laptop with RF recivier module connect by Arduino Duemilanove.

## Part list:

* BME280 Temperature Humidity Pressure Sensor
* RF module 433.92MHz
* Atmega328A-PU
* 16MHz oscilator
* 100nF capacitors
* 22nF capacitors
* 4.7μF capacitors
* LF33CV3 linear voltage regulator
* some wires and connectors :)

## Software:

* Pure C code (measure fired by 8bit timer)
* adapted library: [https://github.com/cosminp12/BME280-Weather-Station](https://github.com/cosminp12/BME280-Weather-Station)
* adapted from arduino version (Mike McCauley library VirtualWire): [VirtualWire API documentation and download instructions](http://www.airspayce.com/mikem/arduino/VirtualWire/index.html)

## Status

Working library BME280
Working RF module 433.92MHz library (VirtualWire from arduino)

##Next step
Add function to sleeping microcontroller between measurement.


![image1](https://github.com/darekb/tempHumidityPressureCollector/blob/master/IMG_20161004_230553.jpg?raw=true)



