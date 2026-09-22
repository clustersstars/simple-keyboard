#include "keyboard.h"
#include "uart_master.h"
#include "usbd_hid.h"
//extern引用
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
//常量声明
const uint8_t keyMap[KEYBOARDROW][KEYBOARDCOL] = {
  //ROW1
  {KB_ESC,0x01,KB_F1,KB_F2,KB_F3,KB_F4,0x01,KB_F5,KB_F6,KB_F7,KB_F8,KB_F9,KB_F10,KB_F11,KB_F12,KB_DELETE},
  //ROW2
  {KB_TILDE,KB_1,KB_2,KB_3,KB_4,KB_5,KB_6,KB_7,KB_8,KB_9,KB_0,KB_MIUS,KB_EQUAL,0x01,KB_BACKSPACE,KB_PAGEUP},
  //ROW3
  {KB_TAB,0x01,KB_Q,KB_W,KB_E,KB_R,KB_T,KB_Y,KB_U,KB_I,KB_O,KB_P,KB_LSQUBRACKET,KB_RSQUBRACKET,KB_BACKSLASH,KB_PAGEDOWN},
  //ROW4
  {KB_CAPSLOCK,0x01,KB_A,KB_S,KB_D,KB_F,KB_G,KB_H,KB_J,KB_K,KB_L,KB_SEMICOLON,KB_APOSTROPHE,KB_ENTER,0x01,KB_HOME},
  //ROW5
  {0x01,Left_Shift,KB_Z,KB_X,KB_C,KB_V,KB_B,KB_N,KB_M,KB_COMMA,KB_FULLSTOP,KB_SLASH,0x01,Right_Shift,KB_UARROW,KB_END},
  //ROW6
  {Left_Ctrl,Left_GUI,0x01,Left_Alt,0x01,0x01,KB_SPACE,0x01,0x01,0x01,Right_Alt,Right_Ctrl,Fn,KB_LARROW,KB_DARROW,KB_RARROW},
};
const uint8_t keyPosMap[KEYBOARDROW][KEYBOARDCOL] = {
  //ROW1
  {COL0,0,COL1,COL2,COL3,COL4,0,COL5,COL6,COL7,COL8,COL9,COL10,COL11,COL12,COL13},
  //ROW2
  {COL0,COL1,COL2,COL3,COL4,COL5,COL6,COL7,COL8,COL9,COL10,COL11,0,COL12,COL13},
  //ROW3
  {COL0,0,COL1,COL2,COL3,COL4,COL5,COL6,COL7,COL8,COL9,COL10,COL11,COL12,COL13,COL14},
  //ROW4
  {COL0,0,COL1,COL2,COL3,COL4,COL5,COL6,COL7,COL8,COL9,COL10,COL11,COL12,0,COL13},
  //ROW5
  {0,COL0,COL1,COL2,COL3,COL4,COL5,COL6,COL7,COL8,COL9,COL10,0,COL11,COL12,COL13},
  //ROW6
  {COL0,COL1,0,COL2,0,0,COL3,0,0,0,COL4,COL5,COL6,COL7,COL8,COL9},
};

#ifdef KEYBOARD_RGB

const uint8_t rgb_commands[13] = {
  RGB_COLOR_BLUE,
  RGB_COLOR_CYAN,
  RGB_COLOR_ORANGE,
  RGB_COLOR_PINK,
  RGB_COLOR_RED,
  RGB_COLOR_VIOLET,
  RGB_COLOR_WHITE,
  RGB_SWITCH_OPEN,
  RGB_MODE_STATIC,
  RGB_MODE_PRESS,
  RGB_MODE_PRESS_RANDOM,
  RGB_MODE_LOVE,
  RGB_SWITCH_CLOSE
};

const RGB_CommandTypeDef rgb_types[13] = {
  COLOR_TYPE,
  COLOR_TYPE,
  COLOR_TYPE,
  COLOR_TYPE,
  COLOR_TYPE,
  COLOR_TYPE,
  COLOR_TYPE,
  SWITCH_TYPE,
  MODE_TYPE,
  MODE_TYPE,
  MODE_TYPE,
  MODE_TYPE,
  SWITCH_TYPE
};

#endif

//变量声明
static volatile int SCAN_ROW; //扫描到行
static volatile uint8_t k_threshold; //按键稳定阈值
static uint16_t ROW_GPIO_PIN[KEYBOARDROW] = {ROW1_Pin,ROW2_Pin,ROW3_Pin,ROW4_Pin,ROW5_Pin,ROW6_Pin};
static GPIO_TypeDef* ROW_GPIOx[KEYBOARDROW] = {ROW1_GPIO_Port,ROW2_GPIO_Port,ROW3_GPIO_Port,ROW4_GPIO_Port,ROW5_GPIO_Port,ROW6_GPIO_Port};
static GPIO_TypeDef* COL_GPIOx[KEYBOARDCOL] = {
  GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,GPIOC,GPIOC,GPIOC,GPIOC,GPIOC,GPIOB,GPIOB,GPIOB
};

static const uint8_t group_mask_to_offset[8] = {0,1,0,2,0,0,0,3};

Keyboard_State           KeyboardState;   //键盘状态
TypeManage               type_manage;
MarkList_InitTypeDef     mark_list;
RewindBuffer_InitTypeDef rewindbuf;
BufferManage             buffer_manage;
ReportManage             report_manage;
#ifdef KEYBOARD_RGB
ModeManage               mode_manage;
PosManage                position_manage;
#endif

TIM_HandleTypeDef *keyboard_tim_1ms = &htim4;   

static bool BufferManage_Init(TypeManage* type_manage,BufferManage *buffer_manage);
static bool ReportManage_Init(TypeManage* type_manage,ReportManage *report_manage);
#ifdef KEYBOARD_RGB

static bool ModeManage_Init(TypeManage* type_manage,ModeManage* mode_manage);
static bool PositionManage_Init(TypeManage* type_manage,PosManage* position_manage);
static void addPosList(uint8_t position);
static void clearPosList(PosManage* position_manage);
static int16_t verifyCommand(uint8_t command);

#endif
static void scanKeyboardRow(void);//行扫描
static bool checkCompare(BufferManage* buffer_manage);
static void updateReport(TypeManage* type_manage);
static void push(TypeManage* type_manage,uint8_t key,uint8_t position);
static void clear(TypeManage* type_manage);

static void clearBuffer(BufferManage* buffer_manage);

//function keyboard
void KeyboardInit(void){
  SCAN_ROW = 0;
  k_threshold = K_MAX_THRESHOLD;
  KeyboardState = KEYBOARD_IDLE;
  //初始化标记数组
  static uint8_t buf[16] = {0};
  type_manage.mark_list = &mark_list;
  MarkList_Init(&mark_list,16,buf);
  //初始化RewindBuf
  RewindBuf_Init(&type_manage,&rewindbuf);
  //初始化按键缓冲
  BufferManage_Init(&type_manage,&buffer_manage);
  //初始化USB报告
  ReportManage_Init(&type_manage,&report_manage);

  #ifdef KEYBOARD_RGB
  //初始化rgb模式
  ModeManage_Init(&type_manage,&mode_manage);
  //初始化按键位置存储
  PositionManage_Init(&type_manage,&position_manage);
  #endif

  HAL_TIM_Base_Start_IT(keyboard_tim_1ms);
}
//init 
bool BufferManage_Init(TypeManage* type_manage,BufferManage *buffer_manage){
  if(type_manage == NULL || buffer_manage == NULL){
    return false;
  }
  buffer_manage->check             = 0;
  buffer_manage->temp_check        = 0;
  buffer_manage->function_key_flag = 0;
  buffer_manage->function_key      = 0;
  if(!NKRO_BufferManage_Init(&buffer_manage->nkro_buffer_manage)){
    return false;
  }
  type_manage->buffer_manage     = buffer_manage;
  return true;
}
bool ReportManage_Init(TypeManage* type_manage,ReportManage *report_manage){
  if(type_manage == NULL || report_manage == NULL){
    return false;
  }
  if(!NKRO_Report_Init(&report_manage->nkro_report)){
    return false;
  }
  report_manage->usb_report    = report_manage->nkro_report.nkro_report_channel1;
  type_manage->report_manage = report_manage;
  return true;
}

#ifdef KEYBOARD_RGB
bool ModeManage_Init(TypeManage* type_manage,ModeManage* mode_manage){
  if(type_manage == NULL || mode_manage == NULL){
    return false;
  }
  //初始化rgb_switch
  mode_manage->rgb_mode[0].rgb_command = RGB_SWITCH_CLOSE;
  mode_manage->rgb_mode[0].effect_state = EFFECT;
  mode_manage->rgb_mode[0].time = 0;
  mode_manage->rgb_mode[0].repeat_transmit_count = 0;
  //初始化rgb_color
  mode_manage->rgb_mode[1].rgb_command = RGB_COLOR_ORANGE;
  mode_manage->rgb_mode[1].effect_state = EFFECT;
  mode_manage->rgb_mode[1].time = 0;
  mode_manage->rgb_mode[1].repeat_transmit_count = 0;
  //初始化rgb_mode
  mode_manage->rgb_mode[2].rgb_command = RGB_MODE_STATIC;
  mode_manage->rgb_mode[2].effect_state = EFFECT;
  mode_manage->rgb_mode[2].time = 0;
  //初始化待处理命令数量
  mode_manage->pending_quantity = 0;
  //初始化上次命令
  mode_manage->last_time_command = 0;
  type_manage->mode_manage = mode_manage;
  return true;
}

bool PositionManage_Init(TypeManage* type_manage,PosManage* position_manage){
  if(type_manage == NULL || position_manage == NULL){
    return false;
  }
  position_manage->pos_list.pos_top = 0;
  memset((uint8_t*)position_manage->pos_list.list,0,sizeof(position_manage->pos_list.list));
  position_manage->pos_update_flag = 0;
  static uint8_t ring_list[15] = {0};
  RingList_Init(&position_manage->ring_structure,15,ring_list);
  type_manage->position_manage = position_manage;
  return true;
}
#endif

void keyboardStart(void){
  //键盘空闲
  if(KeyboardState == KEYBOARD_IDLE){
  }
  //扫描准备
  if(KeyboardState == KEYBOARD_READY_SCAN){
    setKeyboardRowPin(GPIO_PIN_SET);
    KeyboardState = KEYBOARD_READY_FINISH;
  }
  //准备完成 -> 开始扫描
  if(KeyboardState == KEYBOARD_READY_FINISH){
    KeyboardState = KEYBOARD_SCAN_RUNNING;
    scanKeyboardRow();
    KeyboardState = KEYBOARD_SCAN_OVER;
  }
  //扫描结束
  if(KeyboardState == KEYBOARD_SCAN_OVER){
    //判断按键是否变化
    if(checkCompare(type_manage.buffer_manage)){
      if(k_threshold > 0){
        k_threshold--;
      }
    }else{
      //重赋新校验值
      type_manage.buffer_manage->check = type_manage.buffer_manage->temp_check;
      //重置阈值
      k_threshold = K_MAX_THRESHOLD;
      #ifdef KEYBOARD_RGB
      //位置更新标志置位
      type_manage.position_manage->pos_update_flag = 1;
      #endif
    }
    if(k_threshold == 0){
      //更新报表
      updateReport(&type_manage);
      if(type_manage.buffer_manage->check == 0){//判断键盘是否进入空闲状态
        KeyboardState = KEYBOARD_IDLE;
        //恢复行的初始状态(低电平)
        setKeyboardRowPin(GPIO_PIN_RESET); 
        return;
      }
      #ifdef KEYBOARD_RGB
      if(type_manage.position_manage->pos_update_flag){//将pos_list的值转移到ring_list
        for(uint8_t i=0;i<type_manage.position_manage->pos_list.pos_top;i++){
          if(RingList_Put(&type_manage.position_manage->ring_structure,type_manage.position_manage->pos_list.list[i]) == false){
            break;
          }
        }
        type_manage.position_manage->pos_update_flag = 0;
      }
      #endif
    }
    //清理数据缓冲
    clear(&type_manage);
    KeyboardState = KEYBOARD_READY_FINISH;
  }
}

void setKeyboardRowPin(GPIO_PinState PinState){//将键盘行全部置1,或置0
  for(int row=0;row<KEYBOARDROW;row++){
    HAL_GPIO_WritePin(ROW_GPIOx[row],ROW_GPIO_PIN[row],PinState);
  } 
}
void scanKeyboardRow(void){
  uint8_t temp_col = 0;
  KeyboardState = KEYBOARD_SCAN_RUNNING;
  SCAN_ROW = 0;
  for(int row = 0;row<KEYBOARDROW;row++){
    RewindBuf_Rewind(type_manage.rewindbuf);
    HAL_GPIO_WritePin(ROW_GPIOx[row],ROW_GPIO_PIN[row],GPIO_PIN_RESET); //将该行拉低
    for(uint8_t key=0;key<type_manage.rewindbuf->write_pointer;key++){
      temp_col = RewindBuf_Read(type_manage.rewindbuf);//获取标记的列
      if((COL_GPIOx[temp_col]->IDR & (uint32_t)(1 << temp_col)) == (uint32_t)GPIO_PIN_RESET){ //判断该列是否为低电平
        push(&type_manage,keyMap[SCAN_ROW][temp_col],SCAN_ROW*16+keyPosMap[SCAN_ROW][temp_col]);
      }
    }
    HAL_GPIO_WritePin(ROW_GPIOx[row],ROW_GPIO_PIN[row],GPIO_PIN_SET);   //恢复拉高
    SCAN_ROW = (SCAN_ROW+1) % KEYBOARDROW;//更新扫描行
  }
}

uint8_t getKey(uint16_t GPIO_Pin){
  uint8_t base  = 0;
  uint8_t gmask = 0;
  if(GPIO_Pin < 0x0100){      //gpio_pin 0~7
    if(GPIO_Pin < 0x0010){    //gpio_pin 0~3
      base = 0;
      gmask = GPIO_Pin;
    }else{                    //gpio_pin 4~7
      base = 4;
      gmask = GPIO_Pin >> 4;
    }
  }else{                      //gpio_pin 8~15                 
    if(GPIO_Pin < 0x1000){    //gpio_pin 8~11
      base = 8;
      gmask = GPIO_Pin >> 8;  
    }else{                    //gpio_pin 12~15
      base = 12;
      gmask = GPIO_Pin >> 12;
    }
  }
  return base+group_mask_to_offset[gmask-1];
}

void push(TypeManage* type_manage,uint8_t key,uint8_t position){
  if(key == 0xE8){
    type_manage->buffer_manage->function_key_flag = 1;
    return;
  }
  type_manage->buffer_manage->function_key = key;
  type_manage->buffer_manage->temp_check ^= key;
  type_manage->buffer_manage->nkro_buffer_manage.push(&type_manage->buffer_manage->nkro_buffer_manage,key);
  #ifdef KEYBOARD_RGB
  //添加按键位置入栈
  addPosList(position);
  #endif
}

void clear(TypeManage* type_manage){
  RewindBuf_Clear(type_manage->rewindbuf);
  MarkList_MarkClear(type_manage->mark_list);
  clearBuffer(type_manage->buffer_manage);
  #ifdef KEYBOARD_RGB
  clearPosList(type_manage->position_manage);
  #endif
}

void updateReport(TypeManage* type_manage){
  if(type_manage->buffer_manage->function_key_flag && type_manage->buffer_manage->function_key != 0){
    #ifdef KEYBOARD_RGB
    setMode(type_manage->buffer_manage->function_key);
    #endif
    return;
  }
  type_manage->report_manage->usb_report = type_manage->report_manage->nkro_report.fillReport(&type_manage->buffer_manage->nkro_buffer_manage,&type_manage->report_manage->nkro_report);
}

void clearBuffer(BufferManage* buffer_manage){
  buffer_manage->function_key_flag = 0;
  buffer_manage->function_key      = 0;
  buffer_manage->temp_check        = 0;
  buffer_manage->nkro_buffer_manage.clearBuffer(buffer_manage->nkro_buffer_manage.nkro_instant_buffer);
}

bool checkCompare(BufferManage* buffer_manage){
  if(buffer_manage->check != buffer_manage->temp_check){
    return false;
  }
  return true;
}

//
bool RewindBuf_Init(TypeManage *type_manage,RewindBuffer_InitTypeDef *rewindbuf){
  if(type_manage == NULL || rewindbuf == NULL){
    return false;
  }
  rewindbuf->buffer_size = KEYBOARDCOL;
  static uint8_t buffer[KEYBOARDCOL] = {0};
  rewindbuf->buffer        = buffer;
  rewindbuf->read_pointer  = 0;
  rewindbuf->write_pointer = 0;
  type_manage->rewindbuf = rewindbuf;
  return true;
}

bool RewindBuf_Write(struct RewindBuffer_InitTypeDef *rewindbuf,uint8_t elem){
  if(RewindBuf_Is_Full(rewindbuf)){
    return false;
  }
  rewindbuf->buffer[rewindbuf->write_pointer] = elem;
  rewindbuf->write_pointer++;
  return true;
}

int RewindBuf_Read(struct RewindBuffer_InitTypeDef *rewindbuf){
  if(rewindbuf->read_pointer > rewindbuf->write_pointer){
    return -1;
  }
  return rewindbuf->buffer[rewindbuf->read_pointer++];
}

void RewindBuf_Rewind(struct RewindBuffer_InitTypeDef *rewindbuf){
  rewindbuf->read_pointer = 0;
}

void RewindBuf_Clear(struct RewindBuffer_InitTypeDef *rewindbuf){
  rewindbuf->read_pointer = 0;
  rewindbuf->write_pointer = 0;
}

bool RewindBuf_Is_Full(struct RewindBuffer_InitTypeDef *rewindbuf){
  if(rewindbuf->write_pointer < rewindbuf->buffer_size){
    return false;
  }
  return true;
}

bool RewindBuf_Is_Empty(struct RewindBuffer_InitTypeDef *rewindbuf){
  if(rewindbuf->write_pointer != 0){
    return false;
  }
  return true;
}
#ifdef KEYBOARD_RGB
void setMode(uint8_t command){
  int type_index = 0;
  //判断命令是否设置重复
  if(command == type_manage.mode_manage->last_time_command){
    return;
  }
  //判断命令是否正确
  if((type_index = verifyCommand(command)) == -1){
    return;
  }
  //设置命令
  if(type_manage.mode_manage->rgb_mode[rgb_types[type_index]].rgb_command == command){
    return;
  }
  type_manage.mode_manage->rgb_mode[rgb_types[type_index]].rgb_command = command;
  type_manage.mode_manage->rgb_mode[rgb_types[type_index]].effect_state = NO_EFFECT;
  type_manage.mode_manage->rgb_mode[rgb_types[type_index]].repeat_transmit_count = 0;
  type_manage.mode_manage->rgb_mode[rgb_types[type_index]].time = HAL_GetTick();
  type_manage.mode_manage->last_time_command = command;
  type_manage.mode_manage->pending_quantity++;
}

void resetMode(uint8_t command){
  int type_index = 0;
  if((type_index = verifyCommand(command)) == -1){
    return;
  }
  type_manage.mode_manage->rgb_mode[rgb_types[type_index]].effect_state = EFFECT;
  if(type_manage.mode_manage->pending_quantity > 0){
    type_manage.mode_manage->pending_quantity--;
  }
}

void addPosList(uint8_t position){
  if(type_manage.position_manage->pos_list.pos_top >= K_POSITION_LIST_SIZE){
    return;
  }
  type_manage.position_manage->pos_list.list[type_manage.position_manage->pos_list.pos_top] = position;
  type_manage.position_manage->pos_list.pos_top++;
}

void clearPosList(PosManage* position_manage){
  position_manage->pos_list.pos_top = 0;
  memset((uint8_t*)position_manage->pos_list.list,0,sizeof(position_manage->pos_list));
}

void transmitCommand(void){
  if(type_manage.mode_manage->pending_quantity > 0){
    for(uint8_t i=0;i<3;i++){
      if(type_manage.mode_manage->rgb_mode[i].effect_state == NO_EFFECT){
        //发送命令
        send(type_manage.mode_manage->rgb_mode[i].rgb_command,1);
        type_manage.mode_manage->rgb_mode[i].effect_state = PENDING_EFFECT;
      }
    }
  }
}

void transmitPosition(void){
  if(type_manage.position_manage->ring_structure.list_num > 0){
    for(uint8_t i=0;i<type_manage.position_manage->ring_structure.list_num;i++){
      send(RingList_Pop(&type_manage.position_manage->ring_structure),0);
    }
  }
}

//如果找到返回索引，没找到返回-1
int16_t verifyCommand(uint8_t command){
  int16_t left = 0;
  int16_t right = sizeof(rgb_commands)/sizeof(uint8_t) - 1;
  int16_t mid = 0;
  while (left <= right) {
        mid = left + (right - left) / 2;
        if (rgb_commands[mid] == command) {
            return mid;
        } else if (rgb_commands[mid] < command) {
            // 中间值 < 目标值，去右半部分查找
            left = mid + 1;
        } else {
            // 中间值 > 目标值，去左半部分查找
            right = mid - 1;
        }
    }
    // 循环结束未找到  
    return -1;
}
#endif
