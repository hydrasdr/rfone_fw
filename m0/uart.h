/* Serial pass-through for a GNSS module (or anything serial) */
#ifndef __HYDRASDR_UART_H__
#define __HYDRASDR_UART_H__

#include <stdint.h>

#define HYDRASDR_UART_DEFAULT_BAUD (9600)

int hydrasdr_uart_available(void);
void hydrasdr_uart_init(uint32_t baud);
void hydrasdr_uart_flush_rx(void);
uint32_t hydrasdr_uart_read(uint8_t* dst, uint32_t max);  /* main loop only */
uint32_t hydrasdr_uart_write(const uint8_t* src, uint32_t len); /* returns bytes accepted */
uint32_t hydrasdr_uart_baud(void);

#endif
