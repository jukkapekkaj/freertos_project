/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
//#include <mutex>

// TODO: insert other include files here
#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "heap_lock_monitor.h"

#include "timers.h"
#include "Fmutex.h"
#include "LpcUart.h"
#include "LiquidCrystal.h"

#include "SimpleMenu.h"
#include "TextItem.h"
#include "IntegerItem.h"

#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "memory.h"

#include "GMP252.h"
#include "HMP60.h"
#include "FrontPage.h"
#include "main.h"
#include "ITM_write.h"


#include "PlaintextMQTTExample.h"

// TODO: insert other definitions and declarations here

QueueHandle_t menu_event_queue;
QueueHandle_t greenhouse_status_q;
QueueHandle_t mqtt_information_q;
QueueHandle_t co2_target_values;
TimerHandle_t enable_interrupt_timer;
SemaphoreHandle_t s;

Fmutex debug_guard;

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

LpcUart *dbgu = nullptr;
LiquidCrystal *lcd = nullptr;
SimpleMenu *main_menu = nullptr;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static void new_co2_value(int value){
	DEBUGOUT("\r\nNew co2\r\n");
	int inverted_value = ~value;
	eeprom_write(256, (uint8_t *)&value, sizeof(value));
	eeprom_write(256 + sizeof(value), (uint8_t *)&inverted_value, sizeof(inverted_value));
	xQueueSend(co2_target_values, &value, pdMS_TO_TICKS(50));
}

static void new_wifi_ssid(std::string &text){
	dbgu->write("\r\nNew WIFI ssid\r\n");
	dbgu->write(text.c_str());

	xSemaphoreTake(s, portMAX_DELAY);

	mqtt_information mqtt_info;
	xQueuePeek(mqtt_information_q, &mqtt_info, 0);
	memcpy(mqtt_info.wifi_ssid, text.c_str(), text.length() + 1);
	xQueueOverwrite(mqtt_information_q, &mqtt_info);

	save_string(SSID_MEM_ADDR, text.c_str());

	xSemaphoreGive(s);
}

static void new_wifi_password(std::string &text){
	dbgu->write("\r\nNew WIFI password\r\n");
	dbgu->write(text.c_str());
	xSemaphoreTake(s, portMAX_DELAY);

	mqtt_information mqtt_info;
	xQueuePeek(mqtt_information_q, &mqtt_info, 0);
	memcpy(mqtt_info.wifi_password, text.c_str(), text.length() + 1);
	xQueueOverwrite(mqtt_information_q, &mqtt_info);

	save_string(PASSWORD_MEM_ADDR, text.c_str());

	xSemaphoreGive(s);
}

static void new_broker_address(std::string &text){
	dbgu->write("\r\nNew WIFI broker\r\n");
	dbgu->write(text.c_str());
	xSemaphoreTake(s, portMAX_DELAY);

	mqtt_information mqtt_info;
	xQueuePeek(mqtt_information_q, &mqtt_info, 0);
	memcpy(mqtt_info.broker, text.c_str(), text.length() + 1);
	xQueueOverwrite(mqtt_information_q, &mqtt_info);

	save_string(BROKER_MEM_ADDR, text.c_str());

	xSemaphoreGive(s);
}


void enable_interrupts( TimerHandle_t xTimer ){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	if(!Chip_GPIO_GetPinState(LPC_GPIO, ROTARY_ENCODER_PORT, ROTARY_ENCODER_PIN)){
		MenuItem::menuEvent e = MenuItem::menuEvent::ok;
		xQueueSend(menu_event_queue, &e, 0);
	}
}

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

	ITM_init();
	LpcPinMap none = { .port = -1, .pin = -1}; // unused pin has negative values in it
	LpcPinMap txpin = { .port = 0, .pin = 18 }; // transmit pin that goes to debugger's UART->USB converter
	LpcPinMap rxpin = { .port = 0, .pin = 13 }; // receive pin that goes to debugger's UART->USB converter
	LpcUartConfig cfg = {
			.pUART = LPC_USART0,
			.speed = 115200,
			.data = UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1,
			.rs485 = false,
			.tx = txpin,
			.rx = rxpin,
			.rts = none,
			.cts = none
	};
	dbgu = new LpcUart(cfg);

	DigitalIoPin *rs = new DigitalIoPin(0, 29, DigitalIoPin::output);
	DigitalIoPin *en = new DigitalIoPin(0, 9, DigitalIoPin::output);
	DigitalIoPin *d4 = new DigitalIoPin(0, 10, DigitalIoPin::output);
	DigitalIoPin *d5 = new DigitalIoPin(0, 16, DigitalIoPin::output);
	DigitalIoPin *d6 = new DigitalIoPin(1, 3, DigitalIoPin::output);
	DigitalIoPin *d7 = new DigitalIoPin(0, 0, DigitalIoPin::output);
	lcd = new LiquidCrystal(rs, en, d4, d5, d6, d7);
	// configure display geometry
	lcd->begin(16, 2);
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd->setCursor(0, 0);
	lcd->clear();
	// Print a message to the LCD.
	//lcd->print("                                         ");


	memory_initialize();

}

static void initialize_hardware_for_menu(){
	// Set pins for QEI
	CHIP_SWM_PIN_MOVABLE_T qei_a = SWM_QEI0_PHA_I;
	CHIP_SWM_PIN_MOVABLE_T qei_b = SWM_QEI0_PHB_I;
	Chip_IOCON_PinMuxSet(LPC_IOCON, SIG_A_PORT, SIG_A_PIN, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, SIG_B_PORT, SIG_B_PIN, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_SWM_MovablePortPinAssign(qei_a, SIG_A_PORT, SIG_A_PIN);
	Chip_SWM_MovablePortPinAssign(qei_b, SIG_B_PORT, SIG_B_PIN);

	// Configure QEI
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_QEI);
	Chip_SYSCTL_PeriphReset(RESET_QEI0);

	// Signal has to stay stable for 100us before its accepted
	LPC_QEI->FILTERPHA = 7200;
	LPC_QEI->FILTERPHB = 7200;

	LPC_QEI->MAXPOS = 0xFFFFFFFF;

	// Enable interrupt for encoder clock pulse
	LPC_QEI->IES = (1UL << 5);

	// Clear all pending interrupts from QEI
	LPC_QEI->CLR = 0x0000FFFF;

	NVIC_SetPriority(QEI_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
	NVIC_ClearPendingIRQ(QEI_IRQn);
	NVIC_EnableIRQ(QEI_IRQn);


	/* Initialize PININT driver */
	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	/* Set pin back to GPIO (on some boards may have been changed to something
	   else by Board_Init()) */
	Chip_IOCON_PinMuxSet(LPC_IOCON,
			ROTARY_ENCODER_PORT,
			ROTARY_ENCODER_PIN,
			(IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP ) );

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, ROTARY_ENCODER_PORT, ROTARY_ENCODER_PIN);

	/* Configure interrupt channel for the GPIO pin in INMUX block */
	Chip_INMUX_PinIntSel(0, ROTARY_ENCODER_PORT, ROTARY_ENCODER_PIN);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);

	/* Enable interrupt in the NVIC */
	NVIC_SetPriority(PIN_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
}

static void menu_task(void *pvParameters){
	dbgu->write("menu_task is running\r\n");
	initialize_hardware_for_menu();

	SimpleMenu menu; /* this could also be allocated from the heap */
	FrontPage *front_page = new FrontPage(lcd);
	IntegerItem* co2 = new IntegerItem(lcd, std::string("CO2"), new_co2_value);
	TextItem *broker = new TextItem(lcd, std::string("Broker"), new_broker_address);
	TextItem *ssid = new TextItem(lcd, std::string("SSID"), new_wifi_ssid);
	TextItem *password = new TextItem(lcd, std::string("Password"), new_wifi_password);

	menu.addItem(front_page);
	menu.addItem(broker);
	menu.addItem(ssid);
	menu.addItem(password);
	menu.addItem(co2);

	greenhouse_status data;
	mqtt_information mqtt_info;

	xQueuePeek(greenhouse_status_q, &data, 0);
	xQueuePeek(mqtt_information_q, &mqtt_info, 0);

	co2->setValue(data.target_co2);
	ssid->setValue(std::string(mqtt_info.wifi_ssid));
	password->setValue(std::string(mqtt_info.wifi_password));
	broker->setValue(std::string(mqtt_info.broker));
	//broker->setValue(std::string("Default value"));

	menu.event(MenuItem::show); // display first menu item

	main_menu = &menu;

	MenuItem::menuEvent event;

	greenhouse_status status;

	while(1){
		xQueuePeek(greenhouse_status_q, &status, 0);
		front_page->set_co2(status.co2);
		front_page->set_co2_target(status.target_co2);
		front_page->set_humidity(status.humidity);
		front_page->set_temperature(status.temperature);
		front_page->set_valve(status.valve);
		// Wait for new event for 1 second
		BaseType_t result = xQueueReceive(menu_event_queue, &event, pdMS_TO_TICKS(1000));
		if(result == pdTRUE){
			menu.event(event);
		}
		else {
			event = MenuItem::menuEvent::show;
			menu.event(event);
		}


	}
}

int calculate_valve_percentage(int val){
	static const int max_count = 50;
	static int sum = 0;
	static int count = 0;
	if(val > 0){
		if(sum < max_count){
			sum++;
		}

	}
	else{
		if(sum > 0){
			sum--;
		}
	}
	if(count < max_count){
		count++;
	}

	return sum * 100 / count;
	/*
	static const int max_count = 10;
	static int numbers[max_count] = {0};
	static int write_index = 0;
	static bool wrapped = false;

	numbers[write_index++] = val;
	if(write_index >= max_count){
		write_index = 0;
		wrapped = true;
	}

	int result = 0;
	int limit = write_index;

	if(wrapped){
		limit = max_count;
	}

	int sum = 0;
	char buf[5];
	for(int i = 0; i < limit; i++){
		sum += numbers[i];
		snprintf(buf, 5, "%d ", numbers[i]);
		dbgu->write(buf);
	}
	snprintf(buf, 5, "\r\n");
	dbgu->write(buf);

	return sum * 100 / limit;

*/

}

void control_task(void *pvParameters){

	DigitalIoPin valve(0, 27, DigitalIoPin::output, false);
	valve.write(false);
	HMP60 temp_humidity_sensor;
	GMP252 co2_sensor;

	greenhouse_status status;
	status.co2 = 0;
	status.humidity = 0;
	status.target_co2 = 0;
	status.temperature = 0;
	status.valve = 0;

	int co2_level = 0;
	int humidity = 0;
	int temperature = 0;
	int sensor_error = 0;

	const int buf_size = 50;
	char buf[buf_size];

	const TickType_t keep_valve_closed_time = pdMS_TO_TICKS(10000);
	const TickType_t keep_valve_open_time = pdMS_TO_TICKS(2000);
	const TickType_t delay_between_modbus_reads = pdMS_TO_TICKS(10);

	TickType_t valve_opened = 0;
	TickType_t valve_closed = 0;
	TickType_t time_elapsed = 0;

	while(true){
		// Get new co2 target value from queue if there is one.
		xQueueReceive(co2_target_values, &status.target_co2, 0);

		// Read sensors
		co2_level = co2_sensor.readCO2();
		//snprintf(buf, buf_size, "co2: %d\r\n", co2_level);
		//dbgu->write(buf);
		vTaskDelay(delay_between_modbus_reads);

		sensor_error = co2_sensor.getStatus();
		vTaskDelay(delay_between_modbus_reads);

		// Close the valve if there are any errors with co2 sensor
		if(sensor_error){
			dbgu->write("CO2 sensor error\r\n");
			// Close valve
			status.co2 = -1;
			valve.write(false);
			valve_closed = xTaskGetTickCount();
		}
		else {
			status.co2 = co2_level;
			if(valve.read()) {
				// If the valve is open, check if it has been open for long enough and should be closed
				time_elapsed = xTaskGetTickCount() - valve_opened;
				if(time_elapsed >= keep_valve_open_time){
					//status.valve = 0;
					valve.write(false);
					valve_closed = xTaskGetTickCount();
				}
			}
			else{
				time_elapsed = xTaskGetTickCount() - valve_closed;
				if(time_elapsed >= keep_valve_closed_time){
					int new_val = 0;
					if(co2_level < status.target_co2){
						// Open valve
						valve.write(true);
						valve_opened = xTaskGetTickCount();
						new_val = 1;
					}
					else {
						valve_closed = xTaskGetTickCount();
					}
					status.valve = calculate_valve_percentage(new_val);
					//char b[50];
					//snprintf(b, 50, "Valve opening percentage: %d\r\n", status.valve);
					//dbgu->write(b);
				}
			}
		}


		humidity = temp_humidity_sensor.readHumidity();
		vTaskDelay(delay_between_modbus_reads);
		sensor_error = temp_humidity_sensor.getStatus();
		vTaskDelay(delay_between_modbus_reads);

		status.humidity = sensor_error ? -99 : humidity;

		temperature = temp_humidity_sensor.readTemp();
		vTaskDelay(delay_between_modbus_reads);
		sensor_error = temp_humidity_sensor.getStatus();

		status.temperature = sensor_error ? -99 : temperature;

		// Update status in mailbox
		xQueueOverwrite(greenhouse_status_q, &status);

		vTaskDelay(pdMS_TO_TICKS(1000));

	}
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

void QEI_IRQHandler(void){
	// Clear interrupt bit
	LPC_QEI->CLR = 0x0000FFFFU;

	// Handle every other ISR. Rotary encoder creates two interrupts for one turn.
	static bool handle_next = false;
	if(handle_next){
		handle_next = false;
		MenuItem::menuEvent e;
		// Only value in QEI STAT register is the direction
		if(LPC_QEI->STAT){
			e = MenuItem::menuEvent::up;
		}
		else {
			e = MenuItem::menuEvent::down;
		}
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(menu_event_queue, &e, &higherPriorityTaskWoken);
		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}
	else {
		handle_next = true;
	}
}

void PIN_INT0_IRQHandler(void){
	NVIC_DisableIRQ(PIN_INT0_IRQn);

	// Enable interrupts again after 30ms;
	BaseType_t higherPriorityTaskWoken = pdFALSE;
	xTimerStartFromISR(enable_interrupt_timer, &higherPriorityTaskWoken);
	portYIELD_FROM_ISR(higherPriorityTaskWoken);
}


}
/* end runtime statictics collection */

// This task should be highest priority task so it runs first and loads values from EEPROM
// and places them on mailbox queues
void initialize_mailboxes(void *pvParameters){
	dbgu->write("Initialize_mailbox task is running\r\n");
	char ssid[TEXT_LENGTH] = {0};
	char password[TEXT_LENGTH] = {0};
	char broker[TEXT_LENGTH] = {0};

	mqtt_information mqtt_info;

	load_and_validate_string(SSID_MEM_ADDR, ssid);
	memcpy(mqtt_info.wifi_ssid, ssid, TEXT_LENGTH);

	load_and_validate_string(PASSWORD_MEM_ADDR, password);
	memcpy(mqtt_info.wifi_password, password, TEXT_LENGTH);

	load_and_validate_string(BROKER_MEM_ADDR, broker);
	memcpy(mqtt_info.broker, broker, TEXT_LENGTH);


	xQueueOverwrite(mqtt_information_q, &mqtt_info);


	int co2_value = 0;
	int co2_value_inverted = 0;
	eeprom_read(256, (uint8_t *)&co2_value, sizeof(int));
	eeprom_read(256 + sizeof(int),(uint8_t *) &co2_value_inverted, sizeof(int));

	greenhouse_status data;
	data.co2 = 0;
	data.humidity = 0;
	data.target_co2 = 0;
	data.temperature = 0;
	data.valve = 0;

	if(co2_value == ~co2_value_inverted){
		data.target_co2 = co2_value;
	}

	xQueueSend(co2_target_values, &data.target_co2, 0);
	xQueueOverwrite(greenhouse_status_q, &data);


	dbgu->write("Initialized mailboxes\r\n");
	vTaskSuspend(NULL); // Suspend this task once it has initialized everything

	// Just in case...
	while(true){
		vTaskDelay(pdMS_TO_TICKS(100));
		dbgu->write("initialization task is running\r\n");
	}
}

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */

extern "C" {
  void vStartSimpleMQTTDemo( void ); // ugly - should be in a header
}


int main(void)
{
	prvSetupHardware();
	heap_monitor_setup();

	co2_target_values = xQueueCreate(10, sizeof(int));

	greenhouse_status_q = xQueueCreate(1, sizeof(greenhouse_status));
	mqtt_information_q = xQueueCreate(1, sizeof(mqtt_information));

	menu_event_queue = xQueueCreate(100, sizeof(MenuItem::menuEvent));
	vQueueAddToRegistry(menu_event_queue, "queue");

	s = xSemaphoreCreateMutex();

	xTaskCreate(control_task, "task_control",
					configMINIMAL_STACK_SIZE + 512, NULL, (tskIDLE_PRIORITY + 2UL),
					(TaskHandle_t *) NULL);

	xTaskCreate(menu_task, "task_menu",
				configMINIMAL_STACK_SIZE + 512, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	xTaskCreate(initialize_mailboxes, "task_initialize",
					configMINIMAL_STACK_SIZE + 512, NULL, (tskIDLE_PRIORITY + 3UL),
					(TaskHandle_t *) NULL);


	vStartSimpleMQTTDemo();

	enable_interrupt_timer = xTimerCreate("enable_interrupts", pdMS_TO_TICKS(30), pdFALSE, NULL, enable_interrupts);

	/* Start the scheduler */
	vTaskStartScheduler();


	/* Should never arrive here */
	return 1;
}

