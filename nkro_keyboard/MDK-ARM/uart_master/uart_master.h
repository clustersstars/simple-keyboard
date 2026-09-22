#ifndef UART_MASTER_H_
#define UART_MASTER_H_
#include <stdint.h>
#include <string.h>
#include "stm32f1xx_hal.h"

#define UART_COMMAND_RECEIVE_ACK_TIMEOUT 100   //ms
#define UART_MAX_COMAND_REPEAT_COUNT 10

#define KEY_COMMAND_PATTERN "<%hhu>"
#define KEY_POSITION_PATTERN "[%hhu]"

//extern
extern uint8_t ReceiveBuffer[100];       //接受信息的buffer
extern uint8_t Receive_Finish_Flag;      //接受信息完成标志
extern uint8_t ReceiveBufferSize;        //接受信息的长度
void ReceiveBufferParse(void (*cb)(uint8_t));
void send(uint8_t value,uint8_t flag);
extern UART_HandleTypeDef *uart_master; 
#endif
