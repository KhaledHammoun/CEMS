#include <stdio.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
// Drivers
#include <hih8120.h>

#include <ATMEGA_FreeRTOS.h>

#include <FreeRTOSTraceDriver.h>
#include <stdio_driver.h>
#include <serial.h>
#include <timers.h>
#include <task.h>
#include <queue.h>

QueueHandle_t sendQueue;
QueueHandle_t receiveQueue;

void receiveTask( void *pvParameters );
void sendTask( void *pvParameters );
void inputTask( void *pvParameters );
void printTask( void *pvParameters );

typedef struct Message Message;
struct Message
{
	uint16_t data;
	char hammingCode;
};

typedef struct bus_descr_t
{
	volatile uint8_t* ddr;
	volatile uint8_t* pin;
	volatile uint8_t* port;
	uint8_t bit;
} bus_descr_t;

static bus_descr_t bus2[] = {
	{&DDRD, &PIND, &PORTD, PD7},
	{&DDRB, &PINB, &PORTB, PB7},
	{&DDRG, &PING, &PORTG, PG5},
	{&DDRB, &PINB, &PORTB, PB5},
	{&DDRB, &PINB, &PORTB, PB6},
	{&DDRB, &PINB, &PORTB, PB4},
	{&DDRH, &PINH, &PORTH, PH6},
	{&DDRE, &PINE, &PORTE, PE3}
};

#define RECEIVE_PERIOD 700
#define QUEUE_LENGTH 10
#define QUEUE_ITEM_SIZE sizeof(struct Message)
#define SENDER_DELAY 200
#define RECEIVER_DELAY 223

// returns a new char with the first bit set to the value of the bit in position pos of the data parameter
char bitAtPos(char data, int pos){
	char mask = 0x01;
	return (data >> (pos)) & mask;
}

// returns a char with the least 4 bits as the hamming bits of the data
char getHammingBits(char data){
	char bit1 = bitAtPos(data, 0) ^ bitAtPos(data, 1) ^ bitAtPos(data, 3) ^ bitAtPos(data, 4) ^ bitAtPos(data, 6);
	char bit2 = (bitAtPos(data, 0) ^ bitAtPos(data, 2) ^ bitAtPos(data, 3) ^ bitAtPos(data, 5) ^ bitAtPos(data, 6)) << 1;
	char bit3 = (bitAtPos(data, 1) ^ bitAtPos(data, 2) ^ bitAtPos(data, 3) ^ bitAtPos(data, 7)) << 2;
	char bit4 = (bitAtPos(data, 4) ^ bitAtPos(data, 5) ^ bitAtPos(data, 6) ^ bitAtPos(data, 7)) << 3;
	return bit4 | bit3 | bit2 | bit1;
}

void initialiseSystem()
{
	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	trace_init();
	hih8120_initialise();
	
	//Port initialization
	for (int i = 0; i < 8; i++){
		*(bus2[i].ddr) &= ~(_BV(bus2[i].bit));
		*bus2[i].port |= _BV(bus2[i].bit);
	}
	
	//PortC all data OUT and set to zero
	DDRC = 0b11111111;
	PORTC = 0b00000000;
	
	//Task initialization
	xTaskCreate(
	receiveTask,
	"receive",
	configMINIMAL_STACK_SIZE,
	NULL,
	4,
	NULL
	);
	
	xTaskCreate(
	sendTask,
	"send",
	configMINIMAL_STACK_SIZE,
	NULL,
	2,
	NULL
	);
	
	xTaskCreate(
	inputTask,
	"read",
	configMINIMAL_STACK_SIZE,
	NULL,
	3,
	NULL
	);
	
	xTaskCreate(
	printTask,
	"print",
	configMINIMAL_STACK_SIZE,
	NULL,
	1,
	NULL
	);
	
	//Queue initialization
	sendQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
	receiveQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
}
/*-----------------------------------------------------------*/

int main(void)
{
	initialiseSystem();
	vTaskStartScheduler();
}

void receiveTask (void * pvParameters)
{
	//picoscope
	#if (configUSE_APPLICATION_TASK_TAG == 1)
	vTaskSetApplicationTaskTag(NULL, (void *) 10);
	#endif
	
	TickType_t last_wake_time = xTaskGetTickCount();
	
	struct Message message;
	for(;;)
	{
		xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(RECEIVER_PERIOD));
		uint8_t recieved = 0x00;
		
		for (int i = 0; i<8; i++){
			uint8_t bit = (*bus2[i].pin >> bus2[i].bit) & 0b0001;
			recieved = recieved | (bit << i);
		}
		
		if (recieved != 0x00){
			message.data = recieved;
			vTaskDelay(pdMS_TO_TICKS(RECEIVER_DELAY));
			recieved = 0x00;
			for (int i = 0; i<8; i++){
				uint8_t bit = (*bus2[i].pin >> bus2[i].bit) & 0b0001;
				recieved = recieved | (bit << i);
			}
			
			message.hammingCode = recieved;
			
			if (message.hammingCode == getHammingBits(message.data))
			{
				xQueueSend(receiveQueue,(void*)&message, pdMS_TO_TICKS(50));
			}
			else{
				message.data = '<';
				xQueueSend(receiveQueue,(void*)&message, pdMS_TO_TICKS(50));
				
				message.data = 'E';
				xQueueSend(receiveQueue,(void*)&message, pdMS_TO_TICKS(50));
				
				message.data = 'R';
				xQueueSend(receiveQueue,(void*)&message, pdMS_TO_TICKS(50));
				xQueueSend(receiveQueue,(void*)&message, pdMS_TO_TICKS(50));
				message.data = '>';
				xQueueSend(receiveQueue,(void*)&message, pdMS_TO_TICKS(50));
			}
			vTaskDelay(pdMS_TO_TICKS(RECEIVER_DELAY));
		}
	}
}


void sendTask(void * pvParameters)
{
	//picoscope
	#if (configUSE_APPLICATION_TASK_TAG == 1)
	vTaskSetApplicationTaskTag(NULL, (void *) 1);
	#endif
	
	struct Message message;
	
	for(;;)
	{
		if( xQueueReceive( sendQueue, &message, portMAX_DELAY )){
			PORTC = message.data;
			vTaskDelay(pdMS_TO_TICKS(SENDER_DELAY));
			
			PORTC = message.hammingCode; 
			vTaskDelay(pdMS_TO_TICKS(SENDER_DELAY));
			
			PORTC = 0x00;
			vTaskDelay(pdMS_TO_TICKS(SENDER_DELAY));
		}
	}
}

void inputTask(void * pvParameters)
{
	//picoscope
	#if (configUSE_APPLICATION_TASK_TAG == 1)
	vTaskSetApplicationTaskTag(NULL, (void *) 1);
	#endif
	
	TickType_t last_wake_time = xTaskGetTickCount();
	
	struct Message message;
	
	for(;;)
	{
		xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(700));

		if (hih8120_wakeup() == HIH8120_OK) {
			vTaskDelay(pdMS_TO_TICKS(50));
			
			if (hih8120_measure() == HIH8120_OK) {
				int16_t humidity = hih8120_getHumidity();
				message.data = humidity;
			
				message.hammingCode = getHammingBits(humidity);
				xQueueSend(sendQueue,(void*)&message, pdMS_TO_TICKS(50));
			}
		}	
	}
}

void printTask(void * pvParameters)
{
	//picoscope
	#if (configUSE_APPLICATION_TASK_TAG == 1)
	vTaskSetApplicationTaskTag(NULL, (void *) 5);
	#endif
	
	struct Message message;
	
	for(;;)
	{
		// Blocks when nothing in queue
		if( xQueueReceive( receiveQueue, &message, portMAX_DELAY )){
			printf("%c", message.data);
		}
	}
}
