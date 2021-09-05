/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "extint.h"

/* Hardware simulator utility functions */
#include "HW_access.h"

/* SERIAL SIMULATOR CHANNEL TO USE */
#define COM_CH (0)

/* TASK PRIORITIES */
#define	TASK_SERIAL_SEND_PRI		(2 + tskIDLE_PRIORITY  )
#define TASK_SERIAL_REC_PRI			(3+ tskIDLE_PRIORITY )
#define	SERVICE_TASK_PRI		(1+ tskIDLE_PRIORITY )

/* TASKS: FORWARD DECLARATIONS */
void prvSerialReceiveTask_0(void* pvParameters);
void prvSerialReceiveTask_1(void* pvParameters);

SemaphoreHandle_t RXC_BS_0, RXC_BS_1;

/* SERIAL SIMULATOR CHANNEL TO USE */
#define COM_CH_0 (0)
#define COM_CH_1 (1)

/* TASKS: FORWARD DECLARATIONS */
void led_bar_tsk(void* pvParameters);
void SerialSend_Task(void* pvParameters);

void vApplicationIdleHook(void);

/* TRASNMISSION DATA - CONSTANT IN THIS APPLICATION */
const char trigger[] = "Pozdrav svima\n";
unsigned volatile t_point;

/* RECEPTION DATA BUFFER */
#define R_BUF_SIZE (32)
uint8_t r_buffer[R_BUF_SIZE];
uint8_t r_buffer1[R_BUF_SIZE];

uint8_t pomocni[R_BUF_SIZE];


uint8_t r[R_BUF_SIZE];

uint16_t jedinica_a, jedinica_b, jedinica_c, jedinica_d, jedinica_e;
uint16_t stotina_a, stotina_b, stotina_c, stotina_d, stotina_e;
uint16_t desetica_a, stotina_b, stotina_c, stotina_d, stotina_e, hiljada_c;





unsigned volatile r_point;

/* 7-SEG NUMBER DATABASE - ALL HEX DIGITS */
static const unsigned char hexnum[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
								0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 
	0x15, 0x6d};

/* GLOBAL OS-HANDLES */
SemaphoreHandle_t LED_INT_BinarySemaphore;
SemaphoreHandle_t TBE_BinarySemaphore;
SemaphoreHandle_t RXC_BinarySemaphore;
TimerHandle_t tH1;
TimerHandle_t checkIdleCounterTimer;
static QueueHandle_t queue_a;
static QueueHandle_t queue_b;
static QueueHandle_t queue_c;
static QueueHandle_t queue_d;
static QueueHandle_t queue_e;


uint64_t idleHookCounter;


uint32_t OnLED_ChangeInterrupt() {
	// Ovo se desi kad neko pritisne dugme na LED Bar tasterima
	BaseType_t xHigherPTW = pdFALSE;

	xSemaphoreGiveFromISR(LED_INT_BinarySemaphore, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}



static uint32_t prvProcessTBEInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	xSemaphoreGiveFromISR(TBE_BinarySemaphore, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}


/* RXC - RECEPTION COMPLETE - INTERRUPT HANDLER */
static uint32_t prvProcessRXCInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	if (get_RXC_status(0) != 0)
		xSemaphoreGiveFromISR(RXC_BS_0, &xHigherPTW);

	if (get_RXC_status(1) != 0)
		xSemaphoreGiveFromISR(RXC_BS_1, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}



void led_bar_tsk(void* pvParams) {
	uint8_t start_local = 0;
	uint8_t d;
	int a2, b2, c2, d2, e2;

	for (;;) {
		// Ovo postane dostupno kad neko pritisne taster
		xSemaphoreTake(LED_INT_BinarySemaphore, portMAX_DELAY);

		xQueueReceive(queue_a, &a2, pdMS_TO_TICKS(20));
		xQueueReceive(queue_b, &b2, pdMS_TO_TICKS(20));
		xQueueReceive(queue_c, &c2, pdMS_TO_TICKS(20));
		xQueueReceive(queue_d, &d2, pdMS_TO_TICKS(20));
		xQueueReceive(queue_e, &e2, pdMS_TO_TICKS(20));

		uint16_t jedinica_a = a2 % (uint16_t)10;
		printf("jedinicaa je: %d\n", jedinica_a);
		uint16_t desetica_a = (a2 / (uint16_t)10) % (uint16_t)10;
		printf("deseticaa je: %d\n", desetica_a);
		uint16_t stotina_a = (a2 / (uint16_t)100) % (uint16_t)10;
		printf("stotinaa je: %d\n", stotina_a);
		uint16_t hiljada_a = (a2 / (uint16_t)1000);
		printf("hiljadaa je: %d\n", hiljada_a);

		uint16_t jedinica_b = (b2 % (uint16_t)10);
		printf("jedinicab je: %d\n", jedinica_b);
		uint16_t desetica_b = (b2 / (uint16_t)10) % (uint16_t)10;
		printf("deseticab je: %d\n", desetica_b);
		uint16_t stotina_b = b2 / (uint16_t)100;
		printf("stotinab je: %d\n", stotina_b);

		uint16_t jedinica_c = c2 % (uint16_t)10;
		printf("jedinicac je: %d\n", jedinica_c);
		uint16_t desetica_c = (c2 / (uint16_t)10) % (uint16_t)10;
		printf("deseticac je: %d\n", desetica_c);
		uint16_t stotina_c = (c2 / (uint16_t)100) % (uint16_t)10;
		printf("stotinac je: %d\n", stotina_c);
		uint16_t hiljada_c = (c2 / (uint16_t)1000);
		printf("hiljadac je: %d\n", hiljada_c);

		uint16_t jedinica_d = (d2 % (uint16_t)10);
		printf("jedinicad je: %d\n", jedinica_d);
		uint16_t desetica_d = (d2 / (uint16_t)10) % (uint16_t)10;
		printf("deseticad je: %d\n", desetica_d);
		uint16_t stotina_d = d2 / (uint16_t)100;
		printf("stotinad je: %d\n", stotina_d);

		uint16_t jedinica_e = (e2 % (uint16_t)10);
		printf("jedinicae je: %d\n", jedinica_e);
		uint16_t desetica_e = (e2 / (uint16_t)10) % (uint16_t)10;
		printf("deseticae je: %d\n", desetica_e);
		uint16_t stotina_e = e2 / (uint16_t)100;
		printf("stotinae je: %d\n", stotina_e);



		

		if (get_LED_BAR(0, &d) != 0) {
			printf("Greska\n");
		} //ocitaj stanje prvog stubca led bara


		if ((d & (uint8_t)0x01) != (uint8_t)0) { //proveri da li je pritisnut prvi prekidac na led baru, ako jeste, ukljuci sistem, ako nije, iskljucen sistem
			if (set_LED_BAR(1, 0x01) != 0) {
				printf("Greska\n");
			}
			start_local = 1;
			select_7seg_digit(0);
			set_7seg_digit(hexnum[16]);

			select_7seg_digit(2);
			set_7seg_digit(hexnum[hiljada_a]);
			select_7seg_digit(3);
			set_7seg_digit(hexnum[stotina_a]);
			select_7seg_digit(4);
			set_7seg_digit(hexnum[desetica_a]);
			select_7seg_digit(5);
			set_7seg_digit(hexnum[jedinica_a]);

			select_7seg_digit(7);
			set_7seg_digit(hexnum[stotina_b]);
			select_7seg_digit(8);
			set_7seg_digit(hexnum[desetica_b]);
			select_7seg_digit(9);
			set_7seg_digit(hexnum[jedinica_b]);

			
		}
		else {
			if (set_LED_BAR(1, 0x00) != 0) {
				printf("Greska\n");
			}
			start_local = 0;
		}


		if ((d & (uint8_t)0x02) != (uint8_t)0) { 
			select_7seg_digit(0);
			set_7seg_digit(hexnum[13]);
			set_LED_BAR(1, 0x01);

			select_7seg_digit(2);
			set_7seg_digit(hexnum[hiljada_c]);
			select_7seg_digit(3);
			set_7seg_digit(hexnum[stotina_c]);
			select_7seg_digit(4);
			set_7seg_digit(hexnum[desetica_c]);
			select_7seg_digit(5);
			set_7seg_digit(hexnum[jedinica_c]);

			select_7seg_digit(7);
			set_7seg_digit(hexnum[stotina_d]);
			select_7seg_digit(8);
			set_7seg_digit(hexnum[desetica_d]);
			select_7seg_digit(9);
			set_7seg_digit(hexnum[jedinica_d]);
		}



		if ((d & (uint8_t)0x04) != (uint8_t)0) { 
			select_7seg_digit(0);
			set_7seg_digit(hexnum[17]);
			set_LED_BAR(1, 0x01);

			select_7seg_digit(2);
			set_7seg_digit(hexnum[hiljada_c]);
			select_7seg_digit(3);
			set_7seg_digit(hexnum[stotina_c]);
			select_7seg_digit(4);
			set_7seg_digit(hexnum[desetica_c]);
			select_7seg_digit(5);
			set_7seg_digit(hexnum[jedinica_c]);

			select_7seg_digit(7);
			set_7seg_digit(hexnum[stotina_e]);
			select_7seg_digit(8);
			set_7seg_digit(hexnum[desetica_e]);
			select_7seg_digit(9);
			set_7seg_digit(hexnum[jedinica_e]);
		}
	
	}

}

void prvSerialReceiveTask_0(void* pvParameters)
{

	uint8_t pomocni[1] = { 0 };

	int a;
	char c = 0;
	int i, y = 0;
	uint8_t cc1 = 0;
	static uint8_t loca = 0;
	while (1)
	{
		xSemaphoreTake(RXC_BS_0, portMAX_DELAY);
		get_serial_character(COM_CH_0, &cc1);
		printf("Kanal 0: %c\n", cc1);

		if (cc1 == 0x00) // ako je primljen karakter 0, inkrementira se vrednost u GEX formatu na ciframa 5 i 6
		{
			r_point = 0;
			
		}
		else if (cc1 == 0xff)// za svaki KRAJ poruke, prikazati primljenje bajtove direktno na displeju 3-4
		{

			int a = r_buffer[0];
			int b = r_buffer[1];
			int c = r_buffer[2];
			int d = r_buffer[3];
			int e = r_buffer[4];
			int cnovo = c * 10;




			if (r_buffer[0] >= 110) {

				printf("pomocni je: %d\n", r_buffer[0]);
				set_LED_BAR(2, 0xFF);
				vTaskDelay(100);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);

			}
			else
				set_LED_BAR(2, 0x00);



			if (r_buffer[1] >= 90) {

				printf("pomocni je: %d\n", r_buffer[0]);
				set_LED_BAR(2, 0xFF);
				vTaskDelay(100);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);

			}
			else
				set_LED_BAR(2, 0x00);


			if (cnovo >= 3000 || cnovo <= 1500) {

				printf("pomocni je: %d\n", r_buffer[0]);
				set_LED_BAR(2, 0xFF);
				vTaskDelay(100);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);

			}
			else
				set_LED_BAR(2, 0x00);


			if (r_buffer[3] >= 5) {

				printf("pomocni je: %d\n", r_buffer[0]);
				set_LED_BAR(2, 0xFF);
				vTaskDelay(100);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);

			}
			else
				set_LED_BAR(2, 0x00);




			if (r_buffer[4] >= 4) {

				printf("pomocni4 je: %d\n", r_buffer[1]);
				set_LED_BAR(2, 0xFF);
				vTaskDelay(100);
				set_LED_BAR(2, 0x00);
				vTaskDelay(100);
				set_LED_BAR(2, 0xFF);

			}
			else
				set_LED_BAR(2, 0x00);


		
		xQueueSend(queue_a, &a, 0U);
		xQueueSend(queue_b, &b, 0U);
		xQueueSend(queue_c, &cnovo, 0U);
		xQueueSend(queue_d, &d, 0U);
		xQueueSend(queue_e, &e, 0U);

			printf("a je: %d\n", a);
			printf("b je: %d\n", b);
			printf("cnovo je :%d", cnovo);

			
		}


		else if (r_point < R_BUF_SIZE)// pamti karaktere izmedju 0 i FF
		{
			r_buffer[r_point++] = cc1;
	

		}

	}
}

void prvSerialReceiveTask_1(void* pvParameters)
{

	int a1,b1,c1,d1,e1;
	uint8_t cc = 0;

	while (1)
	{
		xSemaphoreTake(RXC_BS_1, portMAX_DELAY);
		get_serial_character(COM_CH_1, &cc);
		printf("Kanal 1: %c\n", cc);

		if (cc == 0xef) // ako je primljen karakter 0, inkrementira se vrednost u GEX formatu na ciframa 5 i 6
		{
			r_point = 0;
		
		}
		else if (cc == 0x0d)// za svaki KRAJ poruke, prikazati primljenje bajtove direktno na displeju 3-4
		{

			
		}
		else if (r_point < R_BUF_SIZE)// pamti karaktere izmedju 0 i FF
		{
			r_buffer1[r_point++] = cc;

			xQueueReceive(queue_a, &a1, pdMS_TO_TICKS(20));
			xQueueReceive(queue_b, &b1, pdMS_TO_TICKS(20));
			xQueueReceive(queue_c, &c1, pdMS_TO_TICKS(20));
			xQueueReceive(queue_d, &d1, pdMS_TO_TICKS(20));
			xQueueReceive(queue_e, &e1, pdMS_TO_TICKS(20));

			uint16_t jedinica_a = a1 % (uint16_t)10;
			printf("jedinicaa je: %d\n", jedinica_a);
			uint16_t desetica_a = (a1 / (uint16_t)10) % (uint16_t)10;
			printf("deseticaa je: %d\n", desetica_a);
			uint16_t stotina_a = (a1 / (uint16_t)100) % (uint16_t)10;
			printf("stotinaa je: %d\n", stotina_a);
			uint16_t hiljada_a = (a1 / (uint16_t)1000);
			printf("hiljadaa je: %d\n", hiljada_a);

			uint16_t jedinica_b = (b1 % (uint16_t)10);
			printf("jedinicab je: %d\n", jedinica_b);
			uint16_t desetica_b = (b1 / (uint16_t)10) % (uint16_t)10;
			printf("deseticab je: %d\n", desetica_b);
			uint16_t stotina_b = b1 / (uint16_t)100;
			printf("stotinab je: %d\n", stotina_b);

			uint16_t jedinica_c = c1 % (uint16_t)10;
			printf("jedinicac je: %d\n", jedinica_c);
			uint16_t desetica_c = (c1 / (uint16_t)10) % (uint16_t)10;
			printf("deseticac je: %d\n", desetica_c);
			uint16_t stotina_c = (c1 / (uint16_t)100) % (uint16_t)10;
			printf("stotinac je: %d\n", stotina_c);
			uint16_t hiljada_c = (c1 / (uint16_t)1000);
			printf("hiljadac je: %d\n", hiljada_c);

			uint16_t jedinica_d = (d1 % (uint16_t)10);
			printf("jedinicad je: %d\n", jedinica_d);
			uint16_t desetica_d = (d1 / (uint16_t)10) % (uint16_t)10;
			printf("deseticad je: %d\n", desetica_d);
			uint16_t stotina_d = d1 / (uint16_t)100;
			printf("stotinad je: %d\n", stotina_d);

			uint16_t jedinica_e = (e1 % (uint16_t)10);
			printf("jedinicae je: %d\n", jedinica_e);
			uint16_t desetica_e = (e1 / (uint16_t)10) % (uint16_t)10;
			printf("deseticae je: %d\n", desetica_e);
			uint16_t stotina_e = e1 / (uint16_t)100;
			printf("stotinae je: %d\n", stotina_e);

		

			if (r_buffer1[0] == 'M') {
				select_7seg_digit(0);
				set_7seg_digit(hexnum[16]);

				select_7seg_digit(2);
				set_7seg_digit(hexnum[hiljada_a]);
				select_7seg_digit(3);
				set_7seg_digit(hexnum[stotina_a]);
				select_7seg_digit(4);
				set_7seg_digit(hexnum[desetica_a]);
				select_7seg_digit(5);
				set_7seg_digit(hexnum[jedinica_a]);

				select_7seg_digit(7);
				set_7seg_digit(hexnum[stotina_b]);
				select_7seg_digit(8);
				set_7seg_digit(hexnum[desetica_b]);
				select_7seg_digit(9);
				set_7seg_digit(hexnum[jedinica_b]);
				
				set_LED_BAR(1, 0x01);
			}

			else if (r_buffer1[0] == 'S') {
				select_7seg_digit(0);
				set_7seg_digit(hexnum[17]);

				select_7seg_digit(2);
				set_7seg_digit(hexnum[hiljada_c]);
				select_7seg_digit(3);
				set_7seg_digit(hexnum[stotina_c]);
				select_7seg_digit(4);
				set_7seg_digit(hexnum[desetica_c]);
				select_7seg_digit(5);
				set_7seg_digit(hexnum[jedinica_c]);

				select_7seg_digit(7);
				set_7seg_digit(hexnum[stotina_e]);
				select_7seg_digit(8);
				set_7seg_digit(hexnum[desetica_e]);
				select_7seg_digit(9);
				set_7seg_digit(hexnum[jedinica_e]);
				set_LED_BAR(1, 0x01);
			}
			
			else if (r_buffer1[0] == 'D') {
				select_7seg_digit(0);
				set_7seg_digit(hexnum[13]);

				select_7seg_digit(2);
				set_7seg_digit(hexnum[hiljada_c]);
				select_7seg_digit(3);
				set_7seg_digit(hexnum[stotina_c]);
				select_7seg_digit(4);
				set_7seg_digit(hexnum[desetica_c]);
				select_7seg_digit(5);
				set_7seg_digit(hexnum[jedinica_c]);

				select_7seg_digit(7);
				set_7seg_digit(hexnum[stotina_d]);
				select_7seg_digit(8);
				set_7seg_digit(hexnum[desetica_d]);
				select_7seg_digit(9);
				set_7seg_digit(hexnum[jedinica_d]);
				set_LED_BAR(1, 0x01);
			}

		}
	}
}


void SerialSend_Task(void* pvParameters)
{
	configASSERT(pvParameters);
	t_point = 0;
	for (;;)
	{
		
		if (t_point > (sizeof(trigger) - 1))
			t_point = 0;
		send_serial_character(0, trigger[t_point]);
		vTaskDelay(10);
		send_serial_character(1, trigger[t_point++]+1);
		//xSemaphoreTake(TBE_BinarySemaphore, portMAX_DELAY);// kada se koristi predajni interapt
		vTaskDelay(pdMS_TO_TICKS(200)); // kada se koristi vremenski delay }
	}
}


/* MAIN - SYSTEM STARTUP POINT */
void main_demo(void)
{
	
	init_serial_uplink(COM_CH); // inicijalizacija serijske TX na kanalu 0
	init_serial_downlink(COM_CH);// inicijalizacija serijske RX na kanalu 0
	init_serial_uplink(1); // inicijalizacija serijske TX na kanalu 0
	init_serial_downlink(1);// ini
	
	init_7seg_comm();
	init_LED_comm();

	/* ON INPUT CHANGE INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_OIC, OnLED_ChangeInterrupt);
	/* SERIAL RECEPTION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);

	/* Create LED interrapt semaphore */
	LED_INT_BinarySemaphore = xSemaphoreCreateBinary();
	RXC_BinarySemaphore = xSemaphoreCreateBinary();
	TBE_BinarySemaphore = xSemaphoreCreateBinary();

	/* create a led bar TASK */
	xTaskCreate(led_bar_tsk, "ST", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);

	/* SERIAL RECEIVER TASK */
	xTaskCreate(prvSerialReceiveTask_0, "SR0", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_REC_PRI, NULL);

	/* SERIAL RECEIVER TASK */
	xTaskCreate(prvSerialReceiveTask_1, "SR1", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_REC_PRI, NULL);
	/* Create TBE semaphores - serial transmit comm */
	RXC_BS_0 = xSemaphoreCreateBinary();
	RXC_BS_1 = xSemaphoreCreateBinary();
	r_point = 0;

	queue_a = xQueueCreate(10, sizeof(double)); 
	if (queue_a == NULL)
	{
		printf("Greska prilikom kreiranja\n");
	}

	queue_b = xQueueCreate(10, sizeof(double));
	if (queue_b == NULL)
	{
		printf("Greska prilikom kreiranja\n");
	}

	queue_c = xQueueCreate(10, sizeof(double));
	if (queue_c == NULL)
	{
		printf("Greska prilikom kreiranja\n");
	}

	queue_d = xQueueCreate(10, sizeof(double));
	if (queue_d == NULL)
	{
		printf("Greska prilikom kreiranja\n");
	}


	queue_e = xQueueCreate(10, sizeof(double));
	if (queue_e == NULL)
	{
		printf("Greska prilikom kreiranja\n");
	}



	/* SERIAL TRANSMITTER TASK */
	configASSERT(xTaskCreate(SerialSend_Task, "STx", configMINIMAL_STACK_SIZE, (void*)1, TASK_SERIAL_SEND_PRI, NULL));


	vTaskStartScheduler();

	for (;;) {}
}

void vApplicationIdleHook(void) {

	idleHookCounter++;
}
