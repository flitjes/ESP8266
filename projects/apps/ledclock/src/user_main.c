#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "mem.h"
#include "ws2812.h"

#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/igmp.h"
#include "lwip/udp.h"   
ip_addr_t ip_of_sender;

void ICACHE_FLASH_ATTR handle_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,  ip_addr_t *addr, u16_t port) {
    int length = p->len;
    char* pusrdata = p->payload;
    os_memcpy(&ip_of_sender, addr, sizeof(ip_addr_t));
    os_printf("%s", pusrdata);
    pbuf_free(p);
}

void init_udp() {
    struct ip_addr ipSend;
    struct udp_pcb * pUdpConnection;
    lwip_init();
    pUdpConnection = udp_new();
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

struct udp_pcb *pcb = NULL;
void ReceiveUART(char * val) {
    struct pbuf *p;
    uint16_t value_size = strlen(val) + 1;
    
    if(pcb){
        udp_disconnect(pcb);
        udp_remove(pcb);
    }

    pcb = udp_new();
    p = pbuf_alloc(PBUF_TRANSPORT, value_size, PBUF_RAM);
    
    os_memcpy(p->payload, val, sizeof(char) * value_size);

    pcb->remote_ip = ip_of_sender;
    pcb->remote_port = 12346;
    udp_sendto(pcb, p, IP_ADDR_BROADCAST, 12346);
    os_free(p);
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
    ip_of_sender.addr=0xFFFFFF;
    setap("LedClock");
    init_udp();
}
void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200, ReceiveUART); // baudrate, callback, eolchar, printftouart
    system_init_done_cb(user_done);
}

