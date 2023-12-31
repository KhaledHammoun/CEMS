#include <UplinkMessageBuilder.h>
#include <Configuration.h>

static uint16_t _humidity;
static int16_t _temperature;
static uint16_t _ppm;
static uint16_t _sound;
static int8_t _validationBits;

void uplinkMessageBuilder_setHumidityData(uint16_t data) {
	_humidity = data;
	
	if (data == CONFIG_INVALID_HUMIDITY_VALUE) {
		_validationBits |= 0 << 3;
	} else {
		_validationBits |= 1 << 3;
	}
}

void uplinkMessageBuilder_setTemperatureData(int16_t data) {
	_temperature = data;
	
	if (data == CONFIG_INVALID_TEMPERATURE_VALUE) {
		_validationBits |= 0 << 2;
	} else {
		_validationBits |= 1 << 2;
	}
}

void uplinkMessageBuilder_setCO2Data(uint16_t data) {
	_ppm = data;
	
	if (data == CONFIG_INVALID_CO2_VALUE) {
		_validationBits |= 0 << 1;
	} else {
		_validationBits |= 1 << 1;
	}
}

void uplinkMessageBuilder_setSoundData(uint16_t data) {
	_sound = data;
	
	if (data == CONFIG_INVALID_SOUND_VALUE) {
		_validationBits |= 0 << 0;
	} else {
		_validationBits |= 1 << 0;
	}
}

void uplinkMessageBuilder_setSystemErrorState(){
	_validationBits = 0;
}