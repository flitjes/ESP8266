#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ws2812.h"
static ETSTimer tickTimer;
#define LEDS 8
char buffer1[LEDS*3] = { 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF,
			 0xFF, 0xFF, 0xFF };

char buffer2[LEDS*3] = { 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00 };

uint8_t toggle = 0;
void ReceiveUART(char * val) {
	os_printf("Received stuff: %s \r\n", val);
}

void tickCb() {
	os_timer_disarm(&tickTimer);
	ets_wdt_disable();
	os_intr_lock();

	os_printf("Tick\r\n");
	if(toggle == 0){
		WS2812OutBuffer(buffer1, sizeof(buffer1), 100);
		toggle = 1;
	} 
	if(toggle == 1){
		WS2812OutBuffer(buffer2, sizeof(buffer2), 100);
		toggle = 0;
	} 
	os_intr_unlock();
	ets_wdt_enable();
	os_timer_arm(&tickTimer, 1000, 0);
	
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200, ReceiveUART); // baudrate, callback, eolchar, printftouart
	os_printf("Starting \r\n");
 	
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 1000, 0);
}

