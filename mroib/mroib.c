#include "openiboot.h"
#include "util.h"
#include "usb.h"
#include "hardware/s5l8900.h"


static void control_received(uint32_t token) {
}
static void data_received(uint32_t token) {
}
static void control_sent(uint32_t token) {
}
static void data_sent(uint32_t token) {
}

static int USB_BYTES_AT_A_TIME = 0;

static uint8_t send_buffer[32] __attribute__((aligned(DMA_ALIGN)));
static uint8_t recv_buffer[32] __attribute__((aligned(DMA_ALIGN)));

static void enumerate_handler(USBInterface* interface) {
    usb_add_endpoint(interface, 1, USBIn, USBInterrupt);
    usb_add_endpoint(interface, 2, USBOut, USBInterrupt);
}

static void start_handler() {
    if(usb_get_speed() == USBHighSpeed) {
        USB_BYTES_AT_A_TIME = 512;
    } else {
        USB_BYTES_AT_A_TIME = 0x80;
    }

    usb_receive_interrupt(2, recv_buffer, 1);
}

static void mr_setup() {
    static Boolean did_mr_setup = FALSE;
    if(did_mr_setup) return;
    did_mr_setup = TRUE;

    usb_setup();
    usb_install_ep_handler(4, USBOut, control_received, 0);
    usb_install_ep_handler(2, USBOut, data_received, 0);
    usb_install_ep_handler(3, USBIn, control_sent, 0);
    usb_install_ep_handler(1, USBIn, data_sent, 0);
    usb_start(enumerate_handler, start_handler);
}

static int mr_getc() {
    return 0;
}

static void mr_putc(char c) {

}

extern uint32_t serial_init[];
extern uint32_t serial_putc[];
extern uint32_t serial_getc[];

__attribute__((constructor))
static void init() {
    serial_init[0] = 0xe51ff004;
    serial_init[1] = (uint32_t) &mr_setup;
    serial_putc[0] = 0x47184b00;
    serial_putc[1] = (uint32_t) &mr_putc;
    serial_getc[0] = 0x47184b00;
    serial_getc[1] = (uint32_t) &mr_getc;
}
