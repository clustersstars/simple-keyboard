#ifndef RGB_MODE_H_
#define RGB_MODE_H_
#include "rgb_data.h"
#include "tool_lib.h"
#include "stm32f1xx_hal.h"
#include <string.h>
  
#define COLOR_MAP_SIZE         82
#define STANDARD_TIME_MAP_SIZE 328
#define CUSTOM_TIME_MAP_SIZE   164
#define MAX_CLIP_NUM           4

typedef enum{
    Static_Solid_Mode = (uint8_t)0,
    Press_Mode,
    Press_Random_Mode,
    Love_Mode
}RGB_Mode;

typedef enum{
    SET_BLACK_COLOR   = (uint8_t)1,
    SET_DEFAULT_COLOR = 2,
    SET_MIN_TIME      = 4,
    SET_MAX_TIME      = 8,
}ConfigCmdTypeDef;

typedef enum{
    READ_TIME   = (uint8_t)1,
    WRITE_COLOR = 2,
    WRITE_TIME  = 4,
}ConfigOpTypeDef;

typedef enum{
    LIMIT = 0,
    NO_LIMIT
}LimitTypeDef;

typedef enum{
    STANDARD_LIGHTING_EFFECTS = (uint8_t)0,
    CUSTOM_LIGHTING_EFFECTS,
    NONE_LIGHTING_EFFECTS
}ModeEffectTypeDef;

typedef union{
    uint32_t standard_time_map[RGB_NUM];
    TimeNode custom_time_map[RGB_NUM];
}UnionTimeMap;

typedef struct{
    LimitTypeDef limit_type;
    uint8_t      limit_count;
    uint8_t      run_time;
    ColorNode    *color_map;
    TimeNode     *time_map;
}ClipTypeDef;

typedef struct{
    uint8_t      list_num;
    uint8_t      list_pointer;
    ClipTypeDef  *current_clip;
    ClipTypeDef  **clip_list;
}CustomClipListInitDef;

typedef struct{
    RGB_Mode               mode;
    ModeEffectTypeDef      effect_type;
    uint8_t                default_color;
    uint32_t               reference_time;
    ColorNode              *color_map;
    UnionTimeMap           *time_map;
    CustomClipListInitDef  *list;
}RgbEffectInitTypeDef;


void RgbEffectInit(RgbEffectInitTypeDef* rgb_effect);
void standardModeInitMaps(RgbEffectInitTypeDef* instance,uint8_t config);
StatusTypeDef customModeInitMaps(RgbEffectInitTypeDef* instance);

//上电默认模式
void defaultMode_Init(RgbEffectInitTypeDef* instance);
//标准
void staticSolidColorMode_Init(RgbEffectInitTypeDef* instance);
void keyPressMode_Init(RgbEffectInitTypeDef* instance);
void keyPressRandomMode_Init(RgbEffectInitTypeDef* instance);
//自定义
void loveMode_Init(RgbEffectInitTypeDef* instance);
void loadLoveAnimationClips(CustomClipListInitDef* list);
#endif
