#ifndef _NKRO_H_
#define _NKRO_H_
#include "tool_lib.h"

#define NKRO_REPORT_BUFFER_SIZE 14 //size = 14字节

typedef struct{
  uint8_t buffer_size;
  uint8_t* buffer;
}NKRO_Buffer;

typedef struct NKRO_BufferManage{
  NKRO_Buffer *nkro_instant_buffer;
  void (*push)(struct NKRO_BufferManage* buffer_manage,uint8_t key);
  void (*clearBuffer)(NKRO_Buffer* buffer);
}NKRO_BufferManage;

typedef struct NKRO_Report{
  uint8_t* nkro_report_channel1;   //缓冲通道1
  uint8_t* nkro_report_channel2;   //缓冲通道2
  volatile uint8_t channel_selected;   //通道选择
  uint8_t* (*fillReport)(NKRO_BufferManage* buffer_manage,struct NKRO_Report* report);
}NKRO_Report;

bool NKRO_BufferManage_Init(NKRO_BufferManage* buffer_manage);
bool NKRO_Report_Init(NKRO_Report* report);
#endif
