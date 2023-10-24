#include <Configuration.h>
#include <semphr.h>

#define CHECK_BIT(variable, position) variable & (1 << position)

static SemaphoreHandle_t _mutex;

static uint16_t _humidityLOW;
static uint16_t _humidityHIGH;
static int16_t _temperatureLOW;
static int16_t _temperatureHIGH;
static uint16_t _ppmLOW;
static uint16_t _ppmHIGH;
static uint16_t _soundHIGH;

void configuration_create(SemaphoreHandle_t mutex) {
	_mutex = mutex;
	
	_humidityLOW = CONFIG_INVALID_HUMIDITY_VALUE;
	_humidityHIGH = CONFIG_INVALID_HUMIDITY_VALUE;
	_temperatureLOW = CONFIG_INVALID_TEMPERATURE_VALUE;
	_temperatureHIGH = CONFIG_INVALID_TEMPERATURE_VALUE;
	_ppmLOW = CONFIG_INVALID_CO2_VALUE;
	_ppmHIGH = CONFIG_INVALID_CO2_VALUE;
}

uint16_t configuration_getLowHumidityThreshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _humidityLOW;
		xSemaphoreGive(_mutex);
		return temp;
	}
}

int16_t configuration_getLowTemperatureThreshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _temperatureLOW;
		xSemaphoreGive(_mutex);
		return temp;
	}
}

uint16_t configuration_getLowCO2Threshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _ppmLOW;
		xSemaphoreGive(_mutex);
		return temp;
	}
}

uint16_t configuration_getHighHumidityThreshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _humidityHIGH;
		xSemaphoreGive(_mutex);
		return temp;
	}
}

int16_t configuration_getHighTemperatureThreshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _temperatureHIGH;
		xSemaphoreGive(_mutex);
		return temp;
	}
}

uint16_t configuration_getHighCO2Threshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _ppmHIGH;
		xSemaphoreGive(_mutex);
		return temp;
	}
}

uint16_t configuration_getHighSoundThreshold() {
	if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
		int16_t temp = _soundHIGH;
		xSemaphoreGive(_mutex);
		return temp;
	}
}
