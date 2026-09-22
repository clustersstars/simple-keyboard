#include "rgb_key.h"
static KeyModeCfg* getKeyModeCfg(uint8_t key);
static KeyColorCfg* getKeyColorCfg(uint8_t key);
static KeySwitchCfg* getKeySwitchCfg(uint8_t key);
KeyColorCfg key_color_arr[7] = { 
    {KB_R,1},{KB_O,5},{KB_P,4},{KB_C,2},{KB_V,3},{KB_B,6},{KB_W,7}
};

KeySwitchCfg key_switch_arr[2] = {     
    {KB_1,TURN_ON_REQUEST},{KB_0,TURN_OFF_REQUEST} 
};

KeyModeCfg key_mode_arr[4] = {
    {KB_2,Static_Solid_Mode,staticSolidColorMode_Init},
    {KB_3,Press_Mode,keyPressMode_Init},
    {KB_4,Press_Random_Mode,keyPressRandomMode_Init},
    {KB_5,Love_Mode,loveMode_Init}
};


void KeyBindInit(KeyBindTypeDef* key_bind){
    key_bind->keyModeCfg_Arr   = key_mode_arr;
    key_bind->keyColorCfg_Arr  = key_color_arr;
    key_bind->keySwitchCfg_Arr = key_switch_arr;
    key_bind->getKeyModeCfg    = getKeyModeCfg;
    key_bind->getKeyColorCfg   = getKeyColorCfg;
    key_bind->getKeySwitchCfg  = getKeySwitchCfg;
}

KeyModeCfg* getKeyModeCfg(uint8_t key){
    KeyModeCfg* keyModeCfg;
    switch (key)
    {
        case KB_2:{
            keyModeCfg = &key_mode_arr[0];
        }break;
        case KB_3:{
            keyModeCfg = &key_mode_arr[1];
        }break;
        case KB_4:{
            keyModeCfg = &key_mode_arr[2];
        }break;
        case KB_5:{
            keyModeCfg = &key_mode_arr[3];
        }break;
        default:{
            keyModeCfg = NULL;
        }break;
    }
    return keyModeCfg;
}

KeyColorCfg* getKeyColorCfg(uint8_t key){
    KeyColorCfg* keyColorCfg;
    switch (key)
    {
        case KB_R:{
            keyColorCfg = &key_color_arr[0];
        }break;
        case KB_O:{
            keyColorCfg = &key_color_arr[1];
        }break;
        case KB_P:{
            keyColorCfg = &key_color_arr[2];
        }break;
        case KB_C:{
            keyColorCfg = &key_color_arr[3];
        }break;
        case KB_V:{
            keyColorCfg = &key_color_arr[4];
        }break;
        case KB_B:{
            keyColorCfg = &key_color_arr[5];
        }break;
        case KB_W:{
            keyColorCfg = &key_color_arr[6];
        }break;
        default:{
            keyColorCfg = NULL;
        }break;
    }
    return keyColorCfg;
}

KeySwitchCfg* getKeySwitchCfg(uint8_t key){
    KeySwitchCfg* keySwitchCfg = NULL;
    if(key == KB_1){
        keySwitchCfg = &key_switch_arr[0];
    }
    if(key == KB_0){
        keySwitchCfg = &key_switch_arr[1];
    }
    return keySwitchCfg;
}

