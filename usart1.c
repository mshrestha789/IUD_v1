#include "usart1.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx.h"

void usart1_init(void);
void usart1_send_char(int8_t c);
void usart1_send_string(int8_t *s);
int16_t usart1_get_char(void);
	


void usart1_init(void)
{
	// Enable clock for GPIOB
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	// Enable clock for USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// Connect PB6 to USART1_Tx
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	// Connect PB7 to USART1_Rx
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	// Initialization of GPIOB
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  // GPIOB_6(TX) and GPIOB7(RX)
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Initialization of USART1
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = (9600);
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);

	// Enable USART1
	USART_Cmd(USART1, ENABLE);
}

void usar1_send_char(int8_t c)
{
    // Wait until transmit data register is empty
    while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    // Send a char using USART1
    USART_SendData(USART1, c);
}

void usart1_send_string(int8_t *s)
{
    // Send a string
    while (*s)
    {
        usar1_send_char(*s++);
    }
//	usar1_send_char(0x0d);
}

int16_t usart1_get_char(void)
{
    // Wait until data is received
    //while (!USART_GetFlagStatus(USART1, USART_FLAG_RXNE));
    // Read received char
    if(!USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
	{
		return 0;
	}
	else
	{
		return USART_ReceiveData(USART1);
	}
}

