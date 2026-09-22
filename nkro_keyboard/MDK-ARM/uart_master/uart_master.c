#include "uart_master.h"
#include "stdio.h"
#include "string.h"
#include "tool_lib.h"
// extern
extern UART_HandleTypeDef huart1;
// //变量声明
static char str_buffer[6]={0};
uint8_t ReceiveBuffer[100]={0};
uint8_t Receive_Finish_Flag;
uint8_t ReceiveBufferSize;
UART_HandleTypeDef *uart_master = &huart1;
char temp_buffer[6] = {0};


void send(uint8_t value,uint8_t flag){
    if(flag){
        sprintf(temp_buffer,KEY_COMMAND_PATTERN,value);
    }else{
        sprintf(temp_buffer,KEY_POSITION_PATTERN,value);
    }
    HAL_UART_Transmit(uart_master,(const uint8_t*)temp_buffer,strlen(temp_buffer)+1,1000);
    while(uart_master->gState != HAL_UART_STATE_READY);
    memset(temp_buffer,0,sizeof(temp_buffer)); 
}

void ReceiveBufferParse(void (*cb)(uint8_t)){
    uint8_t str_head = 0;
    uint8_t str_end = 0;
    uint8_t str_len = 0;
    uint8_t analysis_value = 0;
    for(int i=0;i<ReceiveBufferSize;i++){
        if(ReceiveBuffer[i] == '<'){
            str_head = i+1;
            str_end = 0;
            continue;
        }
        if(ReceiveBuffer[i] == '>'){
            if(str_head == 0){
                continue;
            }
            str_end = i+1;
        }
        if(str_end >= 3){
            str_len = str_end-str_head+1;
            if(str_len < 3){
                str_head = 0;
                str_end = 0;
                str_len = 0;
                continue;
            }
            strncpy(str_buffer,(const char *)ReceiveBuffer+str_head-1,str_len);
            str_buffer[str_len] = '\0';
            sscanf(str_buffer,KEY_COMMAND_PATTERN,&analysis_value);
            memset(str_buffer,0,sizeof(str_buffer));
            //
            (*cb)(analysis_value);
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    if(huart == uart_master){
        if(uart_master->ErrorCode == HAL_UART_ERROR_ORE){
            __HAL_UART_CLEAR_OREFLAG(uart_master);
            HAL_UARTEx_ReceiveToIdle_IT(&huart1,ReceiveBuffer,100);
        }
    }
}

