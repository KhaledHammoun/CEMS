#include <Farmerama.h>
#include <CO2Task.h>
#include <SoundTask.h>
#include <HumidityTemperatureTask.h>
#include <UplinkMessageBuilder.h>
#include <task.h>
#include <stdbool.h>
#include <Configuration.h>

#define TASK_NAME "FarmeramaTask"
#define TASK_INTERVAL 300000UL //5�Minutes
#define TASK_PRIORITY configMAX_PRIORITIES - 1
#define PORT 1

static void _run(void* params);
static TickType_t lastWakeTime;
static QueueHandle_t _senderQueue;
static QueueHandle_t _humidityQueue;
static QueueHandle_t _temperatureQueue;
static QueueHandle_t _co2Queue;
static QueueHandle_t _soundQueue;
static EventGroupHandle_t _actEventGroup;
static EventGroupHandle_t _doneEventGroup;
static bool _errorState = false;

void farmerama_create(QueueHandle_t senderQueue, 
				      QueueHandle_t humidityQueue, 
					  QueueHandle_t temperatureQueue, 
					  QueueHandle_t co2Queue, 
					  QueueHandle_t soundQueue, 
					  EventGroupHandle_t actEventGroup, 
					  EventGroupHandle_t doneEventGroup) {
	_senderQueue = senderQueue;
	_humidityQueue = humidityQueue;
	_temperatureQueue = temperatureQueue;
	_co2Queue = co2Queue;
	_soundQueue = soundQueue;
	_actEventGroup = actEventGroup;
	_doneEventGroup = doneEventGroup;
	
	xTaskCreate(_run, 
			    TASK_NAME, 
				configMINIMAL_STACK_SIZE, 
				NULL, 
				TASK_PRIORITY, 
				NULL
	);
}

void farmerama_initTask(void* params) {
	lastWakeTime = xTaskGetTickCount();
}

void farmerama_runTask(void) {	
	EventBits_t uxBits = xEventGroupSetBits(_actEventGroup, BIT_HUMIDITY_ACT | BIT_TEMPERATURE_ACT);
	
	if ((uxBits & (BIT_HUMIDITY_ACT | BIT_TEMPERATURE_ACT)) 
				== (BIT_HUMIDITY_ACT | BIT_TEMPERATURE_ACT)){
		_errorState = true;
	}

	EventBits_t waitBits = xEventGroupWaitBits(_doneEventGroup,
						BIT_HUMIDITY_DONE | BIT_TEMPERATURE_DONE | BIT_CO2_DONE | BIT_SOUND_DONE,
						pdTRUE, 
						pdTRUE, 
						pdMS_TO_TICKS(TASK_INTERVAL)
	);
	
	if ((waitBits & (BIT_HUMIDITY_DONE | BIT_TEMPERATURE_DONE | BIT_CO2_DONE | BIT_SOUND_DONE))
				!= (BIT_HUMIDITY_DONE | BIT_TEMPERATURE_DONE | BIT_CO2_DONE | BIT_SOUND_DONE)) {
		_errorState = true;
	}

	uint16_t humidity;
	int16_t temperature;
	uint16_t ppm;
	uint16_t sound;

	if (xQueueReceive(_humidityQueue, &humidity, pdMS_TO_TICKS(10000)) != pdTRUE){
		humidity = CONFIG_INVALID_HUMIDITY_VALUE;}
	if (xQueueReceive(_temperatureQueue, &temperature, pdMS_TO_TICKS(10000)) != pdTRUE){
		temperature = CONFIG_INVALID_TEMPERATURE_VALUE;}
	if (xQueueReceive(_co2Queue, &ppm, pdMS_TO_TICKS(10000)) != pdTRUE){
		ppm = CONFIG_INVALID_CO2_VALUE;}
	if (xQueueReceive(_soundQueue, &sound, pdMS_TO_TICKS(10000)) != pdTRUE){
		sound = CONFIG_INVALID_SOUND_VALUE;}
		
	uplinkMessageBuilder_setHumidityData(humidity);
	uplinkMessageBuilder_setTemperatureData(temperature);
	uplinkMessageBuilder_setCO2Data(ppm);
	uplinkMessageBuilder_setSoundData(sound);
	
	if (true == _errorState){ uplinkMessageBuilder_setSystemErrorState(); }

	_errorState = false;
	xTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TASK_INTERVAL));
}

static void _run(void* params) {
	farmerama_initTask(params);
	
	while (1) {
		farmerama_runTask();
	}
}