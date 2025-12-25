#ifndef __USART1_H__
#define __USART1_H__

#include "inttypes.h"

void usart1_init(void);
void usart1_send_char(int8_t c);
void usart1_send_string(int8_t *s);
int16_t usart1_get_char(void);

#endif
