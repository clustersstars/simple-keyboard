#ifndef TOOL_LIB_H_
#define TOOL_LIB_H_
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef enum{
    OK = (uint8_t)0,
    EEROR,
    BUSY,
    TIMEOUT
}StatusTypeDef;

typedef enum{
    Equal = (uint8_t)0,
    No_Equal
}CompareStatusTypeDef;

//创建一个标记数组初始化
typedef struct{
    uint8_t mark_size;//标记数组大小
    uint8_t *mark_list;//标记数组指针
}MarkList_InitTypeDef;

//创建一个环形数组
typedef struct{
    uint8_t list_size;//数组的大小
    uint8_t head_pointer;//头位置
    uint8_t current_pointer;//当前位置
    uint8_t *ring_list;//数组指针
    uint8_t list_num;//已存储数量
}RingList_InitTypeDef;

//extern 
bool MarkList_Init(MarkList_InitTypeDef* mark_structure,uint8_t mark_size,uint8_t* mark_list);
void MarkList_Add(MarkList_InitTypeDef* mark_structure,uint8_t elem);
bool MarkList_IsExist(MarkList_InitTypeDef* mark_structure,uint8_t elem);
void MarkList_MarkClear(MarkList_InitTypeDef* mark_structure);

bool RingList_Init(RingList_InitTypeDef* ring_structure,uint8_t list_size,uint8_t* list);
bool RingList_Put(RingList_InitTypeDef* ring_structure,uint8_t element);
int16_t RingList_Pop(RingList_InitTypeDef* ring_structure);
void RingList_ListClear(RingList_InitTypeDef* ring_structure);
#endif
