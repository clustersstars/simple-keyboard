#include "nkro.h"

static void push(NKRO_BufferManage* buffer_manage,uint8_t key);
static void clearBuffer(NKRO_Buffer* nkro_buffer);
static uint8_t* fillReport(NKRO_BufferManage* buffer_manage,NKRO_Report* report);

bool NKRO_BufferManage_Init(NKRO_BufferManage* buffer_manage){
    if(buffer_manage == NULL){
        return false;
    }
    static NKRO_Buffer nkro_instant_buffer                        = {0};
    buffer_manage->nkro_instant_buffer                            = &nkro_instant_buffer;
    static uint8_t static_instant_buffer[NKRO_REPORT_BUFFER_SIZE] = {0};
    buffer_manage->nkro_instant_buffer->buffer_size               = NKRO_REPORT_BUFFER_SIZE;
    buffer_manage->nkro_instant_buffer->buffer                    = static_instant_buffer;
    buffer_manage->push        = push;
    buffer_manage->clearBuffer = clearBuffer;
    return true;
}

bool NKRO_Report_Init(NKRO_Report* report){
    if(report == NULL){
        return false;
    }
    static uint8_t static_channel1[NKRO_REPORT_BUFFER_SIZE] = {0};
    static uint8_t static_channel2[NKRO_REPORT_BUFFER_SIZE] = {0};
    report->nkro_report_channel1 = static_channel1;
    report->nkro_report_channel2 = static_channel2;
    report->channel_selected = 0;
    report->fillReport = fillReport;
    return true;
}

void clearBuffer(NKRO_Buffer* nkro_buffer){
    memset(nkro_buffer->buffer,0,nkro_buffer->buffer_size);
}

void push(NKRO_BufferManage* buffer_manage,uint8_t key){
    if(key >= 0xE0 && key <= 0xE7){
        buffer_manage->nkro_instant_buffer->buffer[0] |= (1<<(key-0xE0));
    }else{
        buffer_manage->nkro_instant_buffer->buffer[1 + (key / 8)] |= 1 << (key % 8);
    }
}

uint8_t* fillReport(NKRO_BufferManage* buffer_manage,NKRO_Report* report){
    uint8_t *temp = NULL;
    if(report->channel_selected){
        memcpy(report->nkro_report_channel1,buffer_manage->nkro_instant_buffer->buffer,NKRO_REPORT_BUFFER_SIZE);
        // memset(report->nkro_report_channel2,0,NKRO_REPORT_BUFFER_SIZE);
        temp = report->nkro_report_channel1;
    }else{
        memcpy(report->nkro_report_channel2,buffer_manage->nkro_instant_buffer->buffer,NKRO_REPORT_BUFFER_SIZE);
        // memset(report->nkro_report_channel1,0,NKRO_REPORT_BUFFER_SIZE);
        temp = report->nkro_report_channel2;
    }
    report->channel_selected = !report->channel_selected;
    return temp;
}
