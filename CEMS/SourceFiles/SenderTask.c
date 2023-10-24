#include <SenderTask.h>
#include <stdio.h>
#include <stddef.h>
#include <status_leds.h>
#include <stdint.h>
#include <task.h>

#define TASK_NAME "SenderTask"
#define TASK_PRIORITY configMAX_PRIORITIES - 2
#define LORA_appEUI "F2DDE2E826DE9BA5"
#define LORA_appKEY "FA15F6404AD2D77F878514403C7422DD"

static void _run(void* params);

static QueueHandle_t _senderQueue;

void senderTask_create(QueueHandle_t senderQueue) {
	_senderQueue = senderQueue;
	
	xTaskCreate(_run,
	TASK_NAME,
	configMINIMAL_STACK_SIZE,
	NULL,
	TASK_PRIORITY,
	NULL
	);
}

void senderTask_initTask(void* params) {
	//initialize the port
}

void senderTask_runTask() {
// 	lora_driver_payload_t uplinkPayload;
// 	xQueueReceive(_senderQueue, &uplinkPayload, portMAX_DELAY);
// 	lora_driver_returnCode_t status;
// 	if ((status = lora_driver_sendUploadMessage(false, &uplinkPayload)) == LORA_MAC_TX_OK){
// 		puts("Message sent - no downlink");
// 	} else if (status == LORA_MAC_RX){
// 		puts("Message sent - with downlink");
// 	} else {
// 		puts("Message not sent");
// 	}
}

static void _run(void* params) {
	senderTask_initTask(params);
	
	while (1) {
		senderTask_runTask();
	}
}
