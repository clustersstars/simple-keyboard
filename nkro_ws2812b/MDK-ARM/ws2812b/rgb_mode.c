#include "rgb_mode.h"

ClipTypeDef custom_love_group1;
ClipTypeDef custom_love_group2;
ClipTypeDef custom_love_group3;

static StatusTypeDef Ring_Put(CustomClipListInitDef* list,ClipTypeDef* clip);
static ClipTypeDef* Ring_Peek(CustomClipListInitDef* list);
//片段数组初始化
static CustomClipListInitDef* clipListInit(void);
//片段初始化
static void clipInit(ClipTypeDef* clip,const ColorNode* color_map,const TimeNode* time_map);
//片段配置
static ClipTypeDef* CustomClip_SetConfig(ClipTypeDef* clip,uint8_t run_time,LimitTypeDef limit_type,uint8_t limit_count);
//片段注册
static void clipRegister_Init(void);


void RgbEffectInit(RgbEffectInitTypeDef* rgb_effect){
    static ColorNode static_color_map[RGB_NUM] = {0};
    rgb_effect->color_map = static_color_map;
    static UnionTimeMap static_time_map = {0};
    rgb_effect->time_map = &static_time_map;
    rgb_effect->list     = clipListInit();
    clipRegister_Init();
    defaultMode_Init(rgb_effect);
}

//自定义片段管理初始化
CustomClipListInitDef* clipListInit(void){
    static CustomClipListInitDef list = {0};
    static ClipTypeDef* staric_list[MAX_CLIP_NUM] = {NULL};
    list.clip_list    = staric_list;
    list.current_clip = NULL;
    list.list_num     = 0;
    list.list_pointer = 0;
    return &list;
}

void clipRegister_Init(void){
    clipInit(&custom_love_group1,love_group1_color,love_group1_time);
    clipInit(&custom_love_group2,love_group2_color,love_group2_time);
    clipInit(&custom_love_group3,love_group3_color,love_group3_time);
}

void clipInit(ClipTypeDef* clip,const ColorNode* color_map,const TimeNode* time_map){
    clip->color_map = (ColorNode*)color_map;
    clip->time_map  = (TimeNode*)time_map;
}

//开始ring
StatusTypeDef Ring_Put(CustomClipListInitDef* list,ClipTypeDef* clip){
    if(clip == NULL){
        return EEROR;
    }
    if(list->list_num >= MAX_CLIP_NUM){
        return EEROR;
    }
    list->clip_list[list->list_num] = clip;
    list->list_num++;
    return OK;
}
ClipTypeDef* Ring_Peek(CustomClipListInitDef* list){
    ClipTypeDef* temp = NULL;
    if(list->list_num == 0){
        return temp;
    }
    temp = list->clip_list[list->list_pointer];
    list->list_pointer = (++list->list_pointer)%list->list_num;
    return temp;
} 
void Ring_Clear(CustomClipListInitDef* list){
    memset(list->clip_list,0,sizeof(list->clip_list));
    list->list_num = 0;
    list->list_pointer = 0;
}
//设置片段
ClipTypeDef* CustomClip_SetConfig(ClipTypeDef* clip,uint8_t run_time,LimitTypeDef limit_type,uint8_t limit_count){
    clip->run_time    = run_time;
    clip->limit_type  = limit_type;
    clip->limit_count = limit_count;
    return clip;
}

void standardModeInitMaps(RgbEffectInitTypeDef* instance,uint8_t config){
    if((config & SET_BLACK_COLOR) == SET_BLACK_COLOR){
        memset(instance->color_map,0,COLOR_MAP_SIZE);
    }
    if((config & SET_DEFAULT_COLOR) == SET_DEFAULT_COLOR){
        memset(instance->color_map,instance->default_color,COLOR_MAP_SIZE);
    }
    if((config & SET_MIN_TIME) == SET_MIN_TIME){
        memset(instance->time_map->standard_time_map,0x00,STANDARD_TIME_MAP_SIZE);
    }
    if((config & SET_MAX_TIME) == SET_MAX_TIME){
        memset(instance->time_map->standard_time_map,0xFF,STANDARD_TIME_MAP_SIZE);
    }
}

StatusTypeDef customModeInitMaps(RgbEffectInitTypeDef* instance){
    ClipTypeDef *clip_temp = NULL;
    if(instance == NULL){
        return EEROR;
    }
    if(instance->effect_type == CUSTOM_LIGHTING_EFFECTS){
        clip_temp = Ring_Peek(instance->list);
        if(clip_temp == NULL){
            return EEROR;
        }
        instance->list->current_clip = clip_temp;
        memcpy(instance->color_map,instance->list->current_clip->color_map,COLOR_MAP_SIZE);
        memcpy(instance->time_map,instance->list->current_clip->time_map,CUSTOM_TIME_MAP_SIZE);
        instance->reference_time = HAL_GetTick();
    }
    return OK;
}

void defaultMode_Init(RgbEffectInitTypeDef* instance){
    //初始化默认颜色
    instance->default_color = 5;
    //初始化默认模式
    staticSolidColorMode_Init(instance);
}

//静态单色常亮模式 
void staticSolidColorMode_Init(RgbEffectInitTypeDef* instance){
    if(instance->effect_type != STANDARD_LIGHTING_EFFECTS){
        instance->effect_type = STANDARD_LIGHTING_EFFECTS;
    }
    standardModeInitMaps(instance,SET_DEFAULT_COLOR|SET_MAX_TIME);//初始化maps
    instance->mode = Static_Solid_Mode;
}

//按压点亮
void keyPressMode_Init(RgbEffectInitTypeDef* instance){
    if(instance->effect_type != STANDARD_LIGHTING_EFFECTS){
        instance->effect_type = STANDARD_LIGHTING_EFFECTS;
    }
    standardModeInitMaps(instance,SET_BLACK_COLOR|SET_MIN_TIME);//初始化maps
    instance->mode = Press_Mode;
}

//按压点亮随机颜色
void keyPressRandomMode_Init(RgbEffectInitTypeDef* instance){
    if(instance->effect_type != STANDARD_LIGHTING_EFFECTS){
        instance->effect_type = STANDARD_LIGHTING_EFFECTS;
    }
    standardModeInitMaps(instance,SET_BLACK_COLOR|SET_MIN_TIME);//初始化maps
    instance->mode = Press_Random_Mode;
}

//爱心
void loveMode_Init(RgbEffectInitTypeDef* instance){
    if(instance->effect_type != CUSTOM_LIGHTING_EFFECTS){
        instance->effect_type = CUSTOM_LIGHTING_EFFECTS;
    }
    if(instance->list->list_num > 0){
        Ring_Clear(instance->list);
    }
    loadLoveAnimationClips(instance->list);
    if(customModeInitMaps(instance) == EEROR){
        instance->effect_type = NONE_LIGHTING_EFFECTS;
    } 
    instance->mode = Love_Mode;
}

void loadLoveAnimationClips(CustomClipListInitDef* list){
    //添加love每个片段
    if(Ring_Put(list,CustomClip_SetConfig(&custom_love_group1,23,LIMIT,0)) == EEROR){
        return;
    }
    if(Ring_Put(list,CustomClip_SetConfig(&custom_love_group2,5,LIMIT,0)) == EEROR){
        return;
    }
    if(Ring_Put(list,CustomClip_SetConfig(&custom_love_group3,23,NO_LIMIT,0)) == EEROR){
        return;
    }
}
