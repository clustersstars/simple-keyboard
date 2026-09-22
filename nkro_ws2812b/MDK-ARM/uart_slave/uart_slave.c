#include "UART_SLAVE.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;
UART_HandleTypeDef* uart_slave = &huart2;
uint8_t TempBuffer[100]    = {0};
uint8_t ReceiveBuffer[255] = {0};
uint8_t buffer[10]         = {0};
RingList_InitTypeDef ring_structure;
uint8_t TempBufferSize         = 0;
uint8_t TempBuffer_Finish_Flag = 0;
uint8_t Receive_Position_Flag  = 0;
uint8_t position_count         = 0;

static uint8_t parsePostionCode(uint8_t* postion_string);
static uint8_t parseCommandCode(uint8_t* command_string);
static void returnACK(uint8_t CName);

bool Uart_Init(void){
  RingList_Init(&ring_structure,255,ReceiveBuffer);
  return OK;
}

uint8_t parsePostionCode(uint8_t* postion_string){
  uint8_t data_parse = 0;
  sscanf((const char*)postion_string,KEY_POSITION_PATTERN,&data_parse);
  return data_parse;
}
uint8_t parseCommandCode(uint8_t* command_string){
  uint8_t data_parse = 0;
  sscanf((const char*)command_string,COMMAND_PATTERN,&data_parse);
  return data_parse;
}

void BufMove(uint8_t* Arr,uint8_t size){
  for(uint8_t i=0;i<size;i++){
    if(RingList_Put(&ring_structure,Arr[i]) == false){
      break;
    }
  }
}

void ReceiveBufferParse(void (*setMode)(uint8_t),void (*setPosition)(uint8_t)){
  int16_t data                     = 0;
  int8_t buffer_parse_status       = -1; //-1空，0命令，1位置
  uint8_t buffer_parse_finish_flag = 0;  //0未完成,1完成
  uint8_t buffer_top               = 0;
  uint8_t data_len                 = ring_structure.list_num;
  for(int i=0;i<data_len;i++){
    if((data = RingList_Pop(&ring_structure)) == -1){
      break;
    }
    if((uint8_t)data == '<'){
      if(buffer_parse_status != -1){
        memset(buffer,0,sizeof(buffer));
        buffer_top = 0;
      }
      buffer_parse_status = 0;
      buffer[buffer_top++] = '<';
    }else if((uint8_t)data == '>'){
      if(buffer_parse_status != 0){
        memset(buffer,0,sizeof(buffer));
        buffer_top = 0;
        continue;
      }
      buffer[buffer_top++] = '>';
      buffer[buffer_top] = '\0';
      buffer_parse_finish_flag = 1;
    }else if((uint8_t)data == '['){
      if(buffer_parse_status != -1){
        memset(buffer,0,sizeof(buffer));
        buffer_top = 0;
      }
      buffer_parse_status = 1;
      buffer[buffer_top++] = '[';
    }else if((uint8_t)data == ']'){
      if(buffer_parse_status != 1){
        memset(buffer,0,sizeof(buffer));
        buffer_top = 0;
        continue;
      }
      buffer[buffer_top++] = ']';
      buffer[buffer_top] = '\0';
      buffer_parse_finish_flag = 1;
    }else{
      if(buffer_parse_status == -1){
        continue;
      }
      buffer[buffer_top++] = (uint8_t)data;
    }
    if(buffer_parse_finish_flag){
      if(buffer_parse_status){
        setPosition(parsePostionCode(buffer));
      }else if(buffer_parse_status == 0){
        setMode(parseCommandCode(buffer));
        returnACK(parseCommandCode(buffer));
      }else{
        memset(buffer,0,sizeof(buffer));
        buffer_top = 0;
      }
      buffer_parse_finish_flag = 0;
    }
  }
}

void returnACK(uint8_t ACK){
  uint8_t command[10] = {0};
  sprintf((char*)command,COMMAND_PATTERN,ACK);
  HAL_UART_Transmit(uart_slave,(const uint8_t*)command,(uint16_t)(strlen((char*)command)+1),1000);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
  if(huart == uart_slave){
    if(uart_slave->ErrorCode == HAL_UART_ERROR_ORE){
      //清除ore标志
      __HAL_UART_CLEAR_OREFLAG(uart_slave);
      HAL_UARTEx_ReceiveToIdle_IT(uart_slave,TempBuffer,100);
    }
  }
}
