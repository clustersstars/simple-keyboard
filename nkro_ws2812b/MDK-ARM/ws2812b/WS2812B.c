#include "WS2812B.h"
#include "random.h"
#include "stm32f1xx_hal.h"
#include "uart_slave.h"
//extern
extern DMA_HandleTypeDef hdma_memtomem_dma1_channel4;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;//定时器
//定义常量
const uint8_t RGB_NUMBER[6] = {14,15,15,14,14,10}; //每行灯的数量
//定义变量
uint8_t rgb_row_head_index_list[6] = {0};

RGB_InitTypeDef ws_rgb;
RgbEffectInitTypeDef rgb_effect;
KeyBindTypeDef key_bind;
Tim_Inf tim_inf;
RGB_PWM_TypeDef Rfun; 

uint16_t COLOR_ROW_DATA[RGB_COL_NUM*24+1] = {0}; //每行rgb的数据+1位的复位码     
uint16_t* buffer_color_channel1 = NULL;
uint16_t* buffer_color_channel2 = NULL;
TIM_HandleTypeDef* delay_tim = &htim4;
DMA_HandleTypeDef* DMA_MEMTOMEM = &hdma_memtomem_dma1_channel4;   
uint32_t RGB_PWM_CHANNEL_1 = TIM_CHANNEL_1;
uint32_t RGB_PWM_CHANNEL_2 = TIM_CHANNEL_2;
uint32_t RGB_PWM_CHANNEL_3 = TIM_CHANNEL_1;
uint32_t RGB_PWM_CHANNEL_4 = TIM_CHANNEL_3;
uint32_t RGB_PWM_CHANNEL_5 = TIM_CHANNEL_4;
uint32_t RGB_PWM_CHANNEL_6 = TIM_CHANNEL_3;
TIM_HandleTypeDef* RGB_PWM_TIM1 = &htim2;
TIM_HandleTypeDef* RGB_PWM_TIM2 = &htim2;
TIM_HandleTypeDef* RGB_PWM_TIM3 = &htim3;  
TIM_HandleTypeDef* RGB_PWM_TIM4 = &htim3;
TIM_HandleTypeDef* RGB_PWM_TIM5 = &htim3;
TIM_HandleTypeDef* RGB_PWM_TIM6 = &htim2;

uint8_t reset_color_flag = 1;
uint8_t count = 0;

//函数声明
void DMA_Init_Callback(void);
void DMA_MEMTOMEN_CpltCallback(DMA_HandleTypeDef * hdma);
void TIM_PWM_Channel1_Start_Callback(uint32_t* data,uint16_t size);
void TIM_PWM_Channel2_Start_Callback(uint32_t* data,uint16_t size);
void TIM_PWM_Channel3_Start_Callback(uint32_t* data,uint16_t size);
void TIM_PWM_Channel4_Start_Callback(uint32_t* data,uint16_t size);
void TIM_PWM_Channel5_Start_Callback(uint32_t* data,uint16_t size);
void TIM_PWM_Channel6_Start_Callback(uint32_t* data,uint16_t size);
void TIM_PWM_Channel1_Stop_Callback(void);
void TIM_PWM_Channel2_Stop_Callback(void);
void TIM_PWM_Channel3_Stop_Callback(void);
void TIM_PWM_Channel4_Stop_Callback(void);
void TIM_PWM_Channel5_Stop_Callback(void);
void TIM_PWM_Channel6_Stop_Callback(void);

static void headIndexListInit(void);
static void rgbRun(void);
static void RGB_PreloadColor(uint8_t row,uint8_t col);
static inline void RGB_LoadColor(uint8_t row,uint8_t col);
static uint8_t getColorCode(uint8_t row,uint8_t col);
static inline uint8_t getRgbColor(uint8_t index);
static inline void setRgbColor(uint8_t index,uint8_t color_code);
static inline uint32_t getRgbTime(uint8_t index);
static inline void setRgbTime(uint8_t index);
static inline TimeNode* getCustomTime(uint8_t index);
static inline void setRunState(RGB_RunState RunState);

//初始化函数
void WS2812_Init(void){
    ws_rgb.state_ctrl.rgb_state = RGB_STATE_UNENABLE; 
    ws_rgb.state_ctrl.pwm_state = PWM_READY;
    ws_rgb.RGB_ACTIVE_ENABLE_TIME = 0;
    
    RGB_Delay_Init();
    headIndexListInit();
    RGB_PWM_Register_Init();
    DMA_Init_Callback();//DMA初始化
    ws_rgb.rgb_effect = &rgb_effect;
    RgbEffectInit(ws_rgb.rgb_effect);
    ws_rgb.key_bind = &key_bind;
    KeyBindInit(ws_rgb.key_bind);
    
    staticSolidColorMode_Init(ws_rgb.rgb_effect);
}

void DMA_Init_Callback(void){
    HAL_DMA_RegisterCallback(DMA_MEMTOMEM,HAL_DMA_XFER_CPLT_CB_ID,DMA_MEMTOMEN_CpltCallback);
}

void RGB_PWM_Register_Init(void){
    RGB_PWM_RegisterCallback(RGB_PWM_Channel1_CB_ID,TIM_PWM_Channel1_Start_Callback,TIM_PWM_Channel1_Stop_Callback);
    RGB_PWM_RegisterCallback(RGB_PWM_Channel2_CB_ID,TIM_PWM_Channel2_Start_Callback,TIM_PWM_Channel2_Stop_Callback);
    RGB_PWM_RegisterCallback(RGB_PWM_Channel3_CB_ID,TIM_PWM_Channel3_Start_Callback,TIM_PWM_Channel3_Stop_Callback);
    RGB_PWM_RegisterCallback(RGB_PWM_Channel4_CB_ID,TIM_PWM_Channel4_Start_Callback,TIM_PWM_Channel4_Stop_Callback);
    RGB_PWM_RegisterCallback(RGB_PWM_Channel5_CB_ID,TIM_PWM_Channel5_Start_Callback,TIM_PWM_Channel5_Stop_Callback);
    RGB_PWM_RegisterCallback(RGB_PWM_Channel6_CB_ID,TIM_PWM_Channel6_Start_Callback,TIM_PWM_Channel6_Stop_Callback);
}

//delay
void RGB_Delay_Init(void){
    tim_inf.Tim = delay_tim;
}

//初始化行的首索引
void headIndexListInit(void){        
    uint8_t temp = 0;
    for(uint8_t row=0;row<=RGB_ROW_NUM-1;row++){
        rgb_row_head_index_list[row] = temp;
        temp += RGB_NUMBER[row]; 
    }
}

//函数(function)
//运行rgb
void rgbStart(void){
    if(!(ws_rgb.state_ctrl.rgb_state == RGB_STATE_ENABLE || ws_rgb.state_ctrl.rgb_state == RGB_STATE_READY_UNENABLE)){
        return;
    }
    //关闭灯光
    if(ws_rgb.state_ctrl.rgb_state == RGB_STATE_READY_UNENABLE){
        if(ws_rgb.state_ctrl.run_state == RGB_IDLE){
            ws_rgb.state_ctrl.rgb_state = RGB_STATE_UNENABLE;
        }
    }
    //判断键盘灯光是否进入节能
    if(ALLOW_RGB_SLEEP && (HAL_GetTick()-ws_rgb.RGB_ACTIVE_ENABLE_TIME>RGB_AFTER_SLEEP_TIME)){
        reset_color_flag = 1;
        if(ws_rgb.state_ctrl.run_state == RGB_IDLE){
            ws_rgb.state_ctrl.rgb_state = RGB_STATE_SLEEP;
        }
    }
    //换行
    if(ws_rgb.state_ctrl.run_state == RGB_IDLE){ 
        ws_rgb.select_rgb_ctrl.select_row = (ws_rgb.select_rgb_ctrl.select_row+1)%RGB_ROW_NUM;
        if(reset_color_flag){
            setRunState(RESET_ARRAY_FILL_START);
            count++;
            if(count == RGB_ROW_NUM+1){
                count = 0;
                reset_color_flag = 0;
            }
        }else{
            setRunState(COLOR_ARRAY_FILL_START);
        }
    }
    //判断自定义模式,运行时间是否超出预定值
    if(ws_rgb.rgb_effect->effect_type == CUSTOM_LIGHTING_EFFECTS){
        if(HAL_GetTick() - ws_rgb.rgb_effect->reference_time > (uint32_t)(ws_rgb.rgb_effect->list->current_clip->run_time * RGB_CUSTOM_INTERVAL)){
            if(ws_rgb.rgb_effect->list->current_clip->limit_type == NO_LIMIT){
                ws_rgb.rgb_effect->reference_time = HAL_GetTick();
            }else{
                if(ws_rgb.rgb_effect->list->current_clip->limit_count > 0){
                    ws_rgb.rgb_effect->list->current_clip->limit_count--;
                    ws_rgb.rgb_effect->reference_time = HAL_GetTick(); 
                }else{
                    if(ws_rgb.rgb_effect->list->list_num == 1){
                        ws_rgb.rgb_effect->reference_time = HAL_GetTick(); 
                        return;
                    }
                    if(ws_rgb.rgb_effect->list->list_num  >= 2){
                        customModeInitMaps(ws_rgb.rgb_effect);
                    }
                }
            }
        }
    }   
    rgbRun();
}

void rgbRun(void){
    switch (ws_rgb.state_ctrl.run_state)
    {
        case RGB_IDLE:break;
        case COLOR_ARRAY_FILL_START:{
            //提前填充前两个颜色
            RGB_PreloadColor(ws_rgb.select_rgb_ctrl.select_row,ws_rgb.select_rgb_ctrl.select_col);
            HAL_DMA_Start_IT(DMA_MEMTOMEM,(uint32_t)buffer_color_channel1,(uint32_t)COLOR_ROW_DATA[ws_rgb.select_rgb_ctrl.select_row*24],24); 
            ws_rgb.state_ctrl.run_state = WAIT_COLOR_ARRAY_FILL;
        }break;
        case WAIT_COLOR_ARRAY_FILL:{
            if(DMA_MEMTOMEM->State == HAL_DMA_STATE_READY){
                //dam_mem填充完成
                ws_rgb.state_ctrl.run_state = SEND_COLOR_PWM;
                ws_rgb.select_rgb_ctrl.select_col = 0;
            }
        }break;
        case SEND_COLOR_PWM:{
            DMA_PWM_StartTransmitData(ws_rgb.select_rgb_ctrl.select_row,(uint32_t*)COLOR_ROW_DATA,sizeof(COLOR_ROW_DATA)/sizeof(uint16_t));
            ws_rgb.state_ctrl.pwm_state = PWM_BUSY; 
            ws_rgb.state_ctrl.dma_state = DMA_BUSY;
            ws_rgb.state_ctrl.run_state = WAIT_COLOR_PWM_DMA_FINISH;
        }break;
        case WAIT_COLOR_PWM_DMA_FINISH:{
            if(ws_rgb.state_ctrl.dma_state == DMA_READY){
                ws_rgb.state_ctrl.run_state = SEND_COLOR_PWM_FINISH;
            }
        }break;
        case SEND_COLOR_PWM_FINISH:{
            ws_rgb.state_ctrl.run_state = COLOR_DISPLAY_DELAY_START;
        }break;
        case COLOR_DISPLAY_DELAY_START:{
            if(RGB_Delay() == OK){
                ws_rgb.state_ctrl.run_state = WAIT_COLOR_DISPLAY_DELAY_OVER;
            }
        }break;
        case WAIT_COLOR_DISPLAY_DELAY_OVER:{
            if(tim_inf.Tim->State == HAL_TIM_STATE_READY){
                ws_rgb.state_ctrl.run_state = RESET_ARRAY_FILL_START;
                DMA_PWM_StopTransmitData(ws_rgb.select_rgb_ctrl.select_row);
                ws_rgb.state_ctrl.pwm_state = PWM_READY;
                memset(COLOR_ROW_DATA,0,sizeof(COLOR_ROW_DATA));
            }
        }break;
        case RESET_ARRAY_FILL_START:{
            ws_rgb.state_ctrl.run_state = WAIT_RESET_ARRAY_FILL;
        }break;
        case WAIT_RESET_ARRAY_FILL:{    
            ws_rgb.state_ctrl.run_state = SEND_RESET_PWM;
        }break;
        case SEND_RESET_PWM:{
            DMA_PWM_StartTransmitData(ws_rgb.select_rgb_ctrl.select_row,(uint32_t*)rgb_reset,sizeof(rgb_reset)/sizeof(uint16_t));
            ws_rgb.state_ctrl.pwm_state = PWM_BUSY; 
            ws_rgb.state_ctrl.dma_state = DMA_BUSY;
            ws_rgb.state_ctrl.run_state = WAIT_RESET_PWM_DMA_FINISH;
        }break;
        case WAIT_RESET_PWM_DMA_FINISH:{
            if(ws_rgb.state_ctrl.dma_state == DMA_READY){
                ws_rgb.state_ctrl.run_state = SEND_RESET_PWM_FINISH;
            }
        }
        case SEND_RESET_PWM_FINISH:{
            ws_rgb.state_ctrl.run_state = RESET_DISPLAY_DELAY_START;
        }break;
        case RESET_DISPLAY_DELAY_START:{
            if(RGB_Delay() == OK){
                ws_rgb.state_ctrl.run_state = WAIT_RESET_DISPLAY_DELAY_OVER;
            }
        }break;
        case WAIT_RESET_DISPLAY_DELAY_OVER:{
            if(tim_inf.Tim->State == HAL_TIM_STATE_READY){
                ws_rgb.state_ctrl.run_state = RGB_IDLE;
                DMA_PWM_StopTransmitData(ws_rgb.select_rgb_ctrl.select_row);
                ws_rgb.state_ctrl.pwm_state = PWM_READY;
            }
        }break;
        default:
            break;
    }
}

//rgb模式预装填begin
void RGB_PreloadColor(uint8_t row,uint8_t col){
    buffer_color_channel1 = (uint16_t *)colorList[getColorCode(row,col)];
    buffer_color_channel2 = (uint16_t *)colorList[getColorCode(row,col)];
}

inline void RGB_LoadColor(uint8_t row,uint8_t col){
    if(col%2 == 0){
        buffer_color_channel1 = (uint16_t *)colorList[getColorCode(row,col)];
    }else{
        buffer_color_channel2 = (uint16_t *)colorList[getColorCode(row,col)];
    }
}

uint8_t getColorCode(uint8_t row,uint8_t col){
    uint8_t position = rgb_row_head_index_list[row]+col;
    uint8_t color_code = getRgbColor(position);
    if(color_code != 0){
        //检查时间标志
        switch (ws_rgb.rgb_effect->effect_type)
        {
            case STANDARD_LIGHTING_EFFECTS:{
                if(HAL_GetTick() - RGB_DEFAULT_LIGHT_TIME > getRgbTime(position)){
                    //将color置0
                    rgb_effect.color_map[position] = 0;
                    color_code = 0;
                }
            }break;
            case CUSTOM_LIGHTING_EFFECTS:{
                uint32_t offset_time = HAL_GetTick() - ws_rgb.rgb_effect->reference_time;
                if(offset_time < (getCustomTime(position)->offset * RGB_CUSTOM_INTERVAL) \
                || offset_time > (getCustomTime(position)->offset + getCustomTime(position)->interval) * RGB_CUSTOM_INTERVAL){
                    color_code = 0; 
                }
            }break;
            default:{
                color_code = 0;
            }break;
        }
    }
    return color_code;
}

void setMode(uint8_t key){
    void * temp_pointer = NULL;
    RGB_STATE temp_state = ws_rgb.state_ctrl.rgb_state;
    //查key是否存在
    if((temp_pointer = ws_rgb.key_bind->getKeyModeCfg(key)) != NULL){
        ws_rgb.state_ctrl.rgb_state = RGB_STATE_PAUSE;
        ((KeyModeCfg*)temp_pointer)->Mode_Init_CallBack(ws_rgb.rgb_effect);
        ws_rgb.state_ctrl.rgb_state = temp_state;
        return;
    }
    if((temp_pointer = ws_rgb.key_bind->getKeyColorCfg(key)) != NULL){
        ws_rgb.state_ctrl.rgb_state = RGB_STATE_PAUSE;
        ws_rgb.rgb_effect->default_color = ((KeyColorCfg*)temp_pointer)->color;
        if(ws_rgb.rgb_effect->mode == Static_Solid_Mode || ws_rgb.rgb_effect->mode == Press_Mode){
            memset(ws_rgb.rgb_effect->color_map,ws_rgb.rgb_effect->default_color,COLOR_MAP_SIZE);
        }
        ws_rgb.state_ctrl.rgb_state = temp_state;
        return;
    }
    if((temp_pointer = ws_rgb.key_bind->getKeySwitchCfg(key)) != NULL){
        if(((KeySwitchCfg*)temp_pointer)->switch_request == TURN_ON_REQUEST){
            ws_rgb.state_ctrl.rgb_state = RGB_STATE_ENABLE;
        }else{
            ws_rgb.state_ctrl.rgb_state = RGB_STATE_READY_UNENABLE;
        }
    }
}

//位置点亮及重置睡眠时间
void setPosition(uint8_t position){
    uint8_t row = position >> 4;
    uint8_t col = position & 0x0F;
    //rgb状态未使能直接忽略
    if(ws_rgb.state_ctrl.rgb_state == RGB_STATE_UNENABLE){ 
        return;
    }
    if(ws_rgb.rgb_effect->mode == Press_Mode || ws_rgb.rgb_effect->mode == Press_Random_Mode){
        if(HAL_GetTick() -  RGB_DEFAULT_LIGHT_TIME > getRgbTime(rgb_row_head_index_list[row]+col)){
            if(ws_rgb.rgb_effect->mode == Press_Random_Mode){
                setRgbColor(rgb_row_head_index_list[row]+col,(Get_RandValue()%(COLOR_NUMBER-1))+1);
            }else{
                setRgbColor(rgb_row_head_index_list[row]+col,ws_rgb.rgb_effect->default_color);
            }
        }
        setRgbTime(rgb_row_head_index_list[row]+col);
    }
    RGB_Reset_SleepTime();//重置睡眠时间
}

StatusTypeDef RGB_Delay(void){ 
    StatusTypeDef status = EEROR;
    if(tim_inf.Tim->State == HAL_TIM_STATE_READY){
        if(HAL_TIM_Base_Start_IT(delay_tim) == HAL_OK){
            status = OK;
        }
    }
    return status;
}

void RGB_Reset_SleepTime(void){
    ws_rgb.RGB_ACTIVE_ENABLE_TIME = HAL_GetTick();
    if(ws_rgb.state_ctrl.rgb_state == RGB_STATE_SLEEP){
        ws_rgb.state_ctrl.rgb_state = RGB_STATE_ENABLE;
    }
}

void DMA_PWM_StartTransmitData(uint8_t row,uint32_t* data,uint16_t size){
    switch (row)
    {
        case 0:{
            Rfun.TIM_PWM_Channel1_Start_Callback(data,size);
        }break;
        case 1:{
            Rfun.TIM_PWM_Channel2_Start_Callback(data,size);
        }break;
        case 2:{
            Rfun.TIM_PWM_Channel3_Start_Callback(data,size);
        }break;
        case 3:{
            Rfun.TIM_PWM_Channel4_Start_Callback(data,size);
        }break;
        case 4:{
            Rfun.TIM_PWM_Channel5_Start_Callback(data,size);
        }break;
        case 5:{
            Rfun.TIM_PWM_Channel6_Start_Callback(data,size);
        }break;
        default:{ws_rgb.state_ctrl.pwm_state = PWM_READY;}break;
    }
}

void DMA_PWM_StopTransmitData(uint8_t row){
    switch (row)
    {
        case 0:{
            Rfun.TIM_PWM_Channel1_Stop_Callback();
        }break;
        case 1:{
            Rfun.TIM_PWM_Channel2_Stop_Callback();
        }break;
        case 2:{
            Rfun.TIM_PWM_Channel3_Stop_Callback();
        }break;
        case 3:{
            Rfun.TIM_PWM_Channel4_Stop_Callback();
        }break;
        case 4:{
            Rfun.TIM_PWM_Channel5_Stop_Callback();
        }break;
        case 5:{
            Rfun.TIM_PWM_Channel6_Stop_Callback();
        }break;
        default:break;
    }
}

inline uint8_t getRgbColor(uint8_t index){
    return rgb_effect.color_map[index];
}

inline void setRgbColor(uint8_t index,uint8_t color_code){
    rgb_effect.color_map[index] = color_code;
}

inline uint32_t getRgbTime(uint8_t index){
    return rgb_effect.time_map->standard_time_map[index];
}

inline void setRgbTime(uint8_t index){
    rgb_effect.time_map->standard_time_map[index] = HAL_GetTick();
}

inline TimeNode* getCustomTime(uint8_t index){
    return &rgb_effect.time_map->custom_time_map[index];
}

inline void setRunState(RGB_RunState RunState){
    ws_rgb.state_ctrl.run_state = RunState;
}

//callback
void DMA_MEMTOMEN_CpltCallback(DMA_HandleTypeDef * hdma){
    if(DMA_MEMTOMEM->State == HAL_DMA_STATE_BUSY){
        HAL_DMA_Abort_IT(DMA_MEMTOMEM); //停止DMA
    }
    if(ws_rgb.select_rgb_ctrl.select_col >= RGB_NUMBER[ws_rgb.select_rgb_ctrl.select_row]){
        return;
    }
    if(ws_rgb.select_rgb_ctrl.select_col%2 == 0){
        HAL_DMA_Start_IT(DMA_MEMTOMEM,(uint32_t)buffer_color_channel1,(uint32_t)&COLOR_ROW_DATA[ws_rgb.select_rgb_ctrl.select_col*24],24);
    }else{
        HAL_DMA_Start_IT(DMA_MEMTOMEM,(uint32_t)buffer_color_channel2,(uint32_t)&COLOR_ROW_DATA[ws_rgb.select_rgb_ctrl.select_col*24],24);
    }

    ws_rgb.select_rgb_ctrl.select_col = (ws_rgb.select_rgb_ctrl.select_col+1)%RGB_NUMBER[ws_rgb.select_rgb_ctrl.select_row];
    if(ws_rgb.select_rgb_ctrl.select_col != 0){
        //填充下一颜色
        RGB_LoadColor(ws_rgb.select_rgb_ctrl.select_row,ws_rgb.select_rgb_ctrl.select_col);
    }else{
        //填充完成
        ws_rgb.select_rgb_ctrl.select_col = RGB_NUMBER[ws_rgb.select_rgb_ctrl.select_row];
    }
}
void RGB_PWM_RegisterCallback(RGB_PWM_CallbackIdTypeDef ID,void(* pCallback)(uint32_t* data,uint16_t size),void(* dCallback)(void)){
    switch (ID)
    {
        case RGB_PWM_Channel1_CB_ID:{
            Rfun.TIM_PWM_Channel1_Start_Callback = pCallback;
            Rfun.TIM_PWM_Channel1_Stop_Callback = dCallback;
        }break;
        case RGB_PWM_Channel2_CB_ID:{
            Rfun.TIM_PWM_Channel2_Start_Callback = pCallback;
            Rfun.TIM_PWM_Channel2_Stop_Callback = dCallback;
        }break;
        case RGB_PWM_Channel3_CB_ID:{
            Rfun.TIM_PWM_Channel3_Start_Callback = pCallback;
            Rfun.TIM_PWM_Channel3_Stop_Callback = dCallback;
        }break;
        case RGB_PWM_Channel4_CB_ID:{
            Rfun.TIM_PWM_Channel4_Start_Callback = pCallback;
            Rfun.TIM_PWM_Channel4_Stop_Callback = dCallback;
        }break;
        case RGB_PWM_Channel5_CB_ID:{
            Rfun.TIM_PWM_Channel5_Start_Callback = pCallback;
            Rfun.TIM_PWM_Channel5_Stop_Callback = dCallback;
        }break;
        case RGB_PWM_Channel6_CB_ID:{
            Rfun.TIM_PWM_Channel6_Start_Callback = pCallback;
            Rfun.TIM_PWM_Channel6_Stop_Callback = dCallback;
        }break;
        default:
            break;
        }
}

void TIM_PWM_Channel1_Start_Callback(uint32_t* data,uint16_t size){
    HAL_TIM_PWM_Start_DMA(RGB_PWM_TIM1,RGB_PWM_CHANNEL_1,data,size);
}
void TIM_PWM_Channel2_Start_Callback(uint32_t* data,uint16_t size){
    HAL_TIM_PWM_Start_DMA(RGB_PWM_TIM2,RGB_PWM_CHANNEL_2,data,size);
}
void TIM_PWM_Channel3_Start_Callback(uint32_t* data,uint16_t size){
    HAL_TIM_PWM_Start_DMA(RGB_PWM_TIM3,RGB_PWM_CHANNEL_3,data,size);
}
void TIM_PWM_Channel4_Start_Callback(uint32_t* data,uint16_t size){
    HAL_TIM_PWM_Start_DMA(RGB_PWM_TIM4,RGB_PWM_CHANNEL_4,data,size);
}
void TIM_PWM_Channel5_Start_Callback(uint32_t* data,uint16_t size){
    HAL_TIM_PWM_Start_DMA(RGB_PWM_TIM5,RGB_PWM_CHANNEL_5,data,size);
}
void TIM_PWM_Channel6_Start_Callback(uint32_t* data,uint16_t size){
    HAL_TIM_PWM_Start_DMA(RGB_PWM_TIM6,RGB_PWM_CHANNEL_6,data,size);
}

void TIM_PWM_Channel1_Stop_Callback(void){
    HAL_TIM_PWM_Stop(RGB_PWM_TIM1,RGB_PWM_CHANNEL_1);
}
void TIM_PWM_Channel2_Stop_Callback(void){
    HAL_TIM_PWM_Stop(RGB_PWM_TIM2,RGB_PWM_CHANNEL_2);
}
void TIM_PWM_Channel3_Stop_Callback(void){
    HAL_TIM_PWM_Stop(RGB_PWM_TIM3,RGB_PWM_CHANNEL_3);
}
void TIM_PWM_Channel4_Stop_Callback(void){
    HAL_TIM_PWM_Stop(RGB_PWM_TIM4,RGB_PWM_CHANNEL_4);
}
void TIM_PWM_Channel5_Stop_Callback(void){
    HAL_TIM_PWM_Stop(RGB_PWM_TIM5,RGB_PWM_CHANNEL_5);
}
void TIM_PWM_Channel6_Stop_Callback(void){
    HAL_TIM_PWM_Stop(RGB_PWM_TIM6,RGB_PWM_CHANNEL_6);
}

