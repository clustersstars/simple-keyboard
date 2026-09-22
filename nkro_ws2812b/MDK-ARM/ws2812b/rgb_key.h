#ifndef RGB_KEY_H_
#define RGB_KEY_H_
#include "rgb_mode.h"

#define        KB_B                0x05
#define        KB_C                0x06
#define        KB_O                0x12
#define        KB_P                0x13
#define        KB_R                0x15
#define        KB_V                0x19
#define        KB_W                0x1A
#define        KB_1                0x1E
#define        KB_2                0x1F
#define        KB_3                0x20
#define        KB_4                0x21
#define        KB_5                0x22
#define        KB_0                0x27


typedef enum{
    TURN_ON_REQUEST = (uint8_t)1,
    TURN_OFF_REQUEST
}SwitchRequest;

typedef struct{
    uint8_t  key;
    RGB_Mode mode;
    void (*Mode_Init_CallBack)(RgbEffectInitTypeDef* instance);
}KeyModeCfg;

typedef struct{
    uint8_t key;
    uint8_t color;
}KeyColorCfg;

typedef struct{
    uint8_t       key;
    SwitchRequest switch_request;
}KeySwitchCfg;

typedef struct{
    KeyModeCfg* keyModeCfg_Arr;
    KeyModeCfg* (*getKeyModeCfg)(uint8_t key);
    KeyColorCfg* keyColorCfg_Arr;
    KeyColorCfg* (*getKeyColorCfg)(uint8_t key);
    KeySwitchCfg* keySwitchCfg_Arr;
    KeySwitchCfg* (*getKeySwitchCfg)(uint8_t key);
}KeyBindTypeDef;

void KeyBindInit(KeyBindTypeDef* key_bind);
#endif
