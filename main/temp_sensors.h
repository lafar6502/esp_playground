#ifndef _TEMP_SENSORS_H_INCLUDED
#define _TEMP_SENSORS_H_INCLUDED

#define GPIO_DS18B20_0       (CONFIG_ONE_WIRE_GPIO)

void tempSensorsInit();
void tempSensorsRead();
void setupTempSensors2();

#endif

