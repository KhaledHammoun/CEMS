include <ServoTask.h>
#include <Configuration.h>
#include <rc_servo.h>
#include <stdio.h>
#include <stdint.h>
#include <task.h>

#define TASK_NAME "ServoTask"
#define TASK_PRIORITY configMAX_PRIORITIES - 2
#define SERVO_PORT 1
#define SERVO_POS_OPEN 100
#define SERVO_POS_CLOSED -100
#define SERVO_POS_MIDDLE 0

static void _run(void* params);

static QueueHandle_t _servoQueue;

void servoTask_create(QueueHandle_t servoQueue) {
	_servoQueue = servoQueue;
	
	xTaskCreate(_run, 
				TASK_NAME, 
				configMINIMAL_STACK_SIZE, 
				NULL, 
				TASK_PRIORITY, 
				NULL
	);
}

void servoTask_initTask(void* params) {
	// Default the starting window position to be between open and closed.
	rc_servo_setPosition(SERVO_PORT, SERVO_POS_MIDDLE);
}

void servoTask_runTask() {
	uint16_t humidity;
	int16_t temperature;
	uint16_t co2;
	uint16_t sound;
	xQueueReceive(_servoQueue, &humidity, portMAX_DELAY);
	xQueueReceive(_servoQueue, &temperature, portMAX_DELAY);
	xQueueReceive(_servoQueue, &co2, portMAX_DELAY);
	xQueueReceive(_servoQueue, &sound, portMAX_DELAY);
	
	// Delay introduced such that the thresholds are updated before reading them.
	vTaskDelay(pdMS_TO_TICKS(5000));
	
	int16_t lowThreshold = configuration_getLowTemperatureThreshold();
	int16_t highThreshold = configuration_getHighTemperatureThreshold();
	
	// Only open or close the window if the stored thresholds are not set to
	// the default temperature threshold values - the invalid temperature value.
	if (lowThreshold != CONFIG_INVALID_TEMPERATURE_VALUE && temperature < lowThreshold) {
		printf("Measured temperature - %d, low threshold - %d - too cold, the window is closing\n", temperature, lowThreshold);
		rc_servo_setPosition(SERVO_PORT, SERVO_POS_CLOSED);
	} else if (highThreshold != CONFIG_INVALID_TEMPERATURE_VALUE && temperature > highThreshold) {
		printf("Measured temperature - %d, high threshold - %d - too worm, the window is opening\n", temperature, highThreshold);
		rc_servo_setPosition(SERVO_PORT, SERVO_POS_OPEN);
	} else {
		printf("Measured temperature - %d - is within the thresholds. The window is in mid position\n", temperature);
		rc_servo_setPosition(SERVO_PORT, SERVO_POS_MIDDLE);
	}
}

static void _run(void* params) {
	servoTask_initTask(params);
	
	while (1) {
		servoTask_runTask();
	}
}