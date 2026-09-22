#include "tool_lib.h"

//初始化
//初始化MarkList
bool MarkList_Init(MarkList_InitTypeDef* mark_structure,uint8_t mark_size,uint8_t* mark_list){
    mark_structure->mark_size = mark_size;
    if(mark_list == NULL || mark_size == 0){
        return false;
    }
    mark_structure->mark_list = mark_list;
    return true;
}

//添加元素到list
void MarkList_Add(MarkList_InitTypeDef* mark_structure,uint8_t elem){
    mark_structure->mark_list[elem] = 1;
}
bool MarkList_IsExist(MarkList_InitTypeDef* mark_structure,uint8_t elem){
    if(mark_structure->mark_list[elem] == 0){
        return false;
    }
    return true;
}
//标记数组清空
void MarkList_MarkClear(MarkList_InitTypeDef* mark_structure){
    memset(mark_structure->mark_list,0,mark_structure->mark_size);
}


bool RingList_Init(RingList_InitTypeDef* ring_structure,uint8_t list_size,uint8_t* list){
    if(list == NULL || list_size == 0){
        return false;
    }
    ring_structure->list_size       = list_size;
    ring_structure->ring_list       = list;
    ring_structure->head_pointer    = 0;
    ring_structure->current_pointer = 0;
    ring_structure->list_num        = 0;
    return true;
}

bool RingList_Put(RingList_InitTypeDef* ring_structure,uint8_t element){
    //判断数组是否已满
    if(ring_structure->list_num >= ring_structure->list_size){
        return false;
    }
    //入队列
    ring_structure->ring_list[ring_structure->current_pointer] = element;
    ring_structure->current_pointer = (ring_structure->current_pointer+1)%ring_structure->list_size;
    ring_structure->list_num++;
    return true;
}
int16_t RingList_Pop(RingList_InitTypeDef* ring_structure){
    uint8_t element = 0;
    //判断数组是否为空
    if(ring_structure->list_num == 0){
        return -1;
    }
    //
    element = ring_structure->ring_list[ring_structure->head_pointer];
    ring_structure->ring_list[ring_structure->head_pointer] = 0;
    ring_structure->head_pointer = (ring_structure->head_pointer+1)%ring_structure->list_size;
    ring_structure->list_num--;
    return element;
}
void RingList_ListClear(RingList_InitTypeDef* ring_structure){
    memset(ring_structure->ring_list,0,ring_structure->list_size);
    ring_structure->head_pointer    = 0;
    ring_structure->current_pointer = 0;
    ring_structure->list_num        = 0;
}


