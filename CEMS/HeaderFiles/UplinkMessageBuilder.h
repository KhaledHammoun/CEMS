#pragma once

#include <stdint.h>

void uplinkMessageBuilder_setHumidityData(uint16_t data);
void uplinkMessageBuilder_setTemperatureData(int16_t data);
void uplinkMessageBuilder_setCO2Data(uint16_t data);
void uplinkMessageBuilder_setSoundData(uint16_t data);
void uplinkMessageBuilder_setSystemErrorState();