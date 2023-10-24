#include <stdio.h>
#include <avr/io.h>
#include <stdio_driver.h>
#include <serial.h>
#include <ATMEGA_FreeRTOS.h>
#include <queue.h>
#include <event_groups.h>
#include <hih8120.h>
#include <mh_z19.h>
#include <sen14262.h>
#include <semphr.h>
#include <message_buffer.h>

#include <Farmerama.h>
#include <HumidityTemperatureTask.h>
#include <CO2Task.h>
#include <SoundTask.h>
#include <SenderTask.h>
#include <Configuration.h>
#include <ReceiverTask.h>

static QueueHandle_t _humidityQueue;
static QueueHandle_t _temperatureQueue;
static QueueHandle_t _co2Queue;
static QueueHandle_t _soundQueue;
static QueueHandle_t _senderQueue;

static EventGroupHandle_t _actEventGroup;
static EventGroupHandle_t _doneEventGroup;

static SemaphoreHandle_t _mutex;

static MessageBufferHandle_t _messageBuffer;

static void _createQueues(void) {
	_humidityQueue = xQueueCreate(10, sizeof(uint16_t));
	_temperatureQueue = xQueueCreate(10, sizeof(int16_t));
	_co2Queue = xQueueCreate(10, sizeof(uint16_t));
	_soundQueue = xQueueCreate(10, sizeof(uint16_t));
}

static void _createEventGroups(void) {
	_actEventGroup = xEventGroupCreate();
	_doneEventGroup = xEventGroupCreate();
}

static void _createMutexes(void){
	_mutex = xSemaphoreCreateMutex();
}

static void _initDrivers(void) {
	puts("Initializing drivers...");
	hih8120_initialise();
	mh_z19_initialise(ser_USART3);
	sen14262_initialise();
}

static void _createTasks(void) {
	farmerama_create(_senderQueue, _humidityQueue, _temperatureQueue, _co2Queue, _soundQueue, _actEventGroup, _doneEventGroup);
	humidityTemperatureTask_create(_humidityQueue, _temperatureQueue, _actEventGroup, _doneEventGroup);
	co2Task_create(_co2Queue, _actEventGroup, _doneEventGroup);
	soundTask_create(_soundQueue, _actEventGroup, _doneEventGroup);
	senderTask_create(_senderQueue);
	receiverTask_create(_messageBuffer);
}

int main(void) {
	stdio_initialise(ser_USART0);
	
	_createQueues();
	_initDrivers();
	_createEventGroups();
	_createTasks();
	_createMutexes();
	configuration_create(_mutex);
	
	puts("Starting...");
	vTaskStartScheduler();
}