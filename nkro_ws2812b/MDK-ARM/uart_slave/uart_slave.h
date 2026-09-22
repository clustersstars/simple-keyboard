#ifndef UART_SLAVE_H_
#define UART_SLAVE_H_
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "tool_lib.h"
//#define 
#define COMMAND_PATTERN "<%hhu>"
#define KEY_POSITION_PATTERN "[%hhu]"

// extern uint8_t ReceiveBuffer[100];
extern uint8_t TempBuffer[100];
extern uint8_t TempBufferSize;
extern uint8_t TempBuffer_Finish_Flag;
extern UART_HandleTypeDef* uart_slave;
bool Uart_Init(void);
void BufMove(uint8_t* Arr,uint8_t size);
void ReceiveBufferParse(void (*setMode)(uint8_t),void (*setPosition)(uint8_t));
void returnACK(uint8_t ACK);
#endif

