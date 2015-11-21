#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ws2812.h"

#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/udp.h"   

#define LEDS 5

char buf_cache[LEDS * 3];
char buf_cache_count; 

char buffer1[LEDS*3] = { 0x00, 0x00, 0xFF, 
			 0x00, 0x00, 0xFF,
			 0x00, 0x00, 0xFF,
			 0x00, 0x00, 0xFF,
			 0x00, 0x00, 0xFF};/*,
			 0x00, 0x00, 0xFF,
			 0x00, 0x00, 0xFF,
			 0x00, 0x00, 0xFF,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00,
			 0xFF, 0x00, 0x00 };*/


void ICACHE_FLASH_ATTR handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
    int length = p->len;
    char* pusrdata = p->payload;
    os_printf("Received udp data:\n\r");
    os_printf("%s\r", pusrdata);
    pusrdata += strlen("leddata,");
    
    for(int i = 0; i < (LEDS * 3); i++){
        os_printf("%x", pusrdata+i);
    }
    os_printf("\n");
    memcpy(buffer1, pusrdata, LEDS * 3 );
    pbuf_free(p);
}

void init_udp() {
    struct ip_addr ipSend;
    lwip_init();
    struct udp_pcb * pUdpConnection = udp_new();
    IP4_ADDR(&ipSend, 255, 255, 255, 255);
    pUdpConnection->multicast_ip = ipSend;
    pUdpConnection->remote_ip = ipSend;
    pUdpConnection->remote_port = 12345;
    if(pUdpConnection == NULL) {
        os_printf("\nCould not create new udp socket... \n");
    }
    int err = udp_bind(pUdpConnection, IP_ADDR_ANY, 12345);
    udp_recv(pUdpConnection, handle_udp_recv, pUdpConnection);
}

static ETSTimer tickTimer;

void ReceiveUART(char * val) {
	os_printf("Received stuff:\r\n");
	os_printf("%s \r", val);
}

void tickCb() {
	os_timer_disarm(&tickTimer);
	ets_wdt_disable();
	os_intr_lock();

    os_printf("Data\r\n");
	WS2812OutBuffer(buffer1, sizeof(buffer1), 1);
	
    os_intr_unlock();
	ets_wdt_enable();
	os_timer_arm(&tickTimer, 1000, 0);
	
}

char ssidcpy[32] = { 0 };

void setap(char * ssid) {
    static struct softap_config apconf;
    wifi_set_opmode(STATIONAP_MODE);
    wifi_softap_get_config(&apconf);
    os_memset(apconf.ssid, 0, 32);
    os_strncpy((char*) apconf.ssid, ssid, 32);
    apconf.authmode = AUTH_OPEN;
    apconf.max_connection = 20;
    apconf.ssid_hidden = 0;
    apconf.ssid_len = os_strlen(ssid);
    apconf.channel = 1;

    wifi_softap_set_config(&apconf);
}

void ICACHE_FLASH_ATTR user_done(void) {
	os_printf("Starting \r\n");
    setap("LedClock");
    init_udp();
	os_timer_disarm(&tickTimer);
	os_timer_setfn(&tickTimer, tickCb, NULL);
	os_timer_arm(&tickTimer, 100, 0);

}
void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200, ReceiveUART); // baudrate, callback, eolchar, printftouart
    system_init_done_cb(user_done);
}

