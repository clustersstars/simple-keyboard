#ifndef WS2812B_H_
#define WS2812B_H_
#include "stm32f1xx_hal.h"       
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rgb_key.h"

//rgb设置
#define RGB_DEFAULT_LIGHT_TIME  3000    //单位毫秒(rgb灯亮的时间)
#define ALLOW_RGB_SLEEP         1       //允许rgb休眠
#define RGB_AFTER_SLEEP_TIME    15000   //rgb空闲15s后休眠
#define COLOR_NUMBER            8      
#define RGB_CUSTOM_INTERVAL     100

typedef enum{
    RGB_STATE_UNENABLE = 0,
    RGB_STATE_READY_UNENABLE,
    RGB_STATE_ENABLE,
    RGB_STATE_SLEEP,
    RGB_STATE_PAUSE
}RGB_STATE;

typedef enum{
    PWM_READY = 0,
    PWM_BUSY
    // PWM_FINISH
}PWM_STATE;

// typedef enum{
//     MEM_DMA_IDLE = 0,
//     MEM_DMA_READY_DATA,
//     MEM_DMA_DATA_READY_FINISH,
// }MEM_DMA_STATE;

typedef enum{
    DMA_READY = (uint8_t)0,
    DMA_BUSY
    // PWM_DMA_FINISH
}DMA_STATE;

// typedef enum{
//     // BUFFER_ENABLE = (uint8_t)0,
//     BUFFER_NOT_FILLED = (uint8_t)0,
//     BUFFER_IS_FILLED,
//     // BUFFER_COLOR_IS_COMPLETE, //颜色缓冲数据已完成
//     // BUFFER_UNENABLE
// }BUFFER_FILL_COLOR_STATE;

typedef enum{
    RGB_IDLE = (uint8_t)0,
    COLOR_ARRAY_FILL_START,
    WAIT_COLOR_ARRAY_FILL,
    SEND_COLOR_PWM,
    WAIT_COLOR_PWM_DMA_FINISH,
    SEND_COLOR_PWM_FINISH,
    COLOR_DISPLAY_DELAY_START,
    WAIT_COLOR_DISPLAY_DELAY_OVER,

    RESET_ARRAY_FILL_START,
    WAIT_RESET_ARRAY_FILL,
    SEND_RESET_PWM,
    WAIT_RESET_PWM_DMA_FINISH,
    SEND_RESET_PWM_FINISH,
    RESET_DISPLAY_DELAY_START,
    WAIT_RESET_DISPLAY_DELAY_OVER,
}RGB_RunState;

typedef struct{
    RGB_STATE               rgb_state;                   //rgb灯控的状态
    RGB_RunState            run_state;
    PWM_STATE               pwm_state;                   //pwm的状态
    // MEM_DMA_STATE           mem_dma_state;               //dma的状态
    // BUFFER_FILL_COLOR_STATE buffer_fill_state; //
    DMA_STATE               dma_state;
}StateCtrlTypeDef;

typedef struct{
    uint8_t select_row;                    //选中rgb的行
    uint8_t select_col;                    //该行rgb的传输索引
}SelectRgbCtrlTypeDef;

typedef struct{
    uint32_t              RGB_ACTIVE_ENABLE_TIME;        //rgb enable 的激活时间
    StateCtrlTypeDef      state_ctrl;
    SelectRgbCtrlTypeDef  select_rgb_ctrl;
    RgbEffectInitTypeDef  *rgb_effect;
    KeyBindTypeDef        *key_bind;
}RGB_InitTypeDef;

//延时函数结构体
typedef enum{
    TIM_IDLE = 0,
    TIM_BUSY,
}TIM_STATE;

typedef struct{
    TIM_HandleTypeDef* Tim;
}Tim_Inf;
//
typedef enum{
    RGB_PWM_Channel1_CB_ID = 0,
    RGB_PWM_Channel2_CB_ID,
    RGB_PWM_Channel3_CB_ID,
    RGB_PWM_Channel4_CB_ID,
    RGB_PWM_Channel5_CB_ID,
    RGB_PWM_Channel6_CB_ID,
}RGB_PWM_CallbackIdTypeDef;

typedef struct{
    void     (*TIM_PWM_Channel1_Start_Callback)(uint32_t* data,uint16_t size);
    void     (*TIM_PWM_Channel2_Start_Callback)(uint32_t* data,uint16_t size);
    void     (*TIM_PWM_Channel3_Start_Callback)(uint32_t* data,uint16_t size);
    void     (*TIM_PWM_Channel4_Start_Callback)(uint32_t* data,uint16_t size);
    void     (*TIM_PWM_Channel5_Start_Callback)(uint32_t* data,uint16_t size);
    void     (*TIM_PWM_Channel6_Start_Callback)(uint32_t* data,uint16_t size);
    void     (*TIM_PWM_Channel1_Stop_Callback)(void);
    void     (*TIM_PWM_Channel2_Stop_Callback)(void);
    void     (*TIM_PWM_Channel3_Stop_Callback)(void);
    void     (*TIM_PWM_Channel4_Stop_Callback)(void);
    void     (*TIM_PWM_Channel5_Stop_Callback)(void);
    void     (*TIM_PWM_Channel6_Stop_Callback)(void);
}RGB_PWM_TypeDef;


//extern
extern const uint8_t KEY_NUMBER[6]; //键盘每个行数的按键个数
extern RGB_InitTypeDef ws_rgb; //rgb灯初始化
extern Tim_Inf tim_inf; //延时函数信息
extern TIM_HandleTypeDef* delay_tim; //定时器
extern DMA_HandleTypeDef* DMA_MEMTOMEM; //RAM与ROM的DMA通道
extern RGB_PWM_TypeDef Rfun; 
extern TIM_HandleTypeDef* RGB_PWM_TIM1;
extern uint32_t RGB_PWM_CHANNEL_1;
extern TIM_HandleTypeDef* RGB_PWM_TIM2;
extern uint32_t RGB_PWM_CHANNEL_2;
extern TIM_HandleTypeDef* RGB_PWM_TIM3;
extern uint32_t RGB_PWM_CHANNEL_3;
extern TIM_HandleTypeDef* RGB_PWM_TIM4;
extern uint32_t RGB_PWM_CHANNEL_4;
extern TIM_HandleTypeDef* RGB_PWM_TIM5;
extern uint32_t RGB_PWM_CHANNEL_5;
extern TIM_HandleTypeDef* RGB_PWM_TIM6;
extern uint32_t RGB_PWM_CHANNEL_6;

//函数声明
void WS2812_Init(void); //初始化hal_rgb
void RGB_Delay_Init(void);
StatusTypeDef RGB_Delay(void);  //延时1ms
void rgbStart(void);
void RGB_Srand_Init(void);
void RGB_PWM_RegisterCallback(RGB_PWM_CallbackIdTypeDef ID,void(* pCallback)(uint32_t* data,uint16_t size),void(* dCallback)(void));
void RGB_PWM_Register_Init(void);
void DMA_PWM_StartTransmitData(uint8_t row,uint32_t* data,uint16_t size);
void DMA_PWM_StopTransmitData(uint8_t row);
void RGB_Reset_SleepTime(void);
void setMode(uint8_t mode);
void setPosition(uint8_t position);

#endif
