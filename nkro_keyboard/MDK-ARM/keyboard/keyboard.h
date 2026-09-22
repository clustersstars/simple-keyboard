#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#define KEYBOARD_RGB 
#include "stm32f1xx_hal.h"
#include <stdlib.h>
#include <stdio.h>
#include "tool_lib.h"
#include "nkro.h"

#define        KB_ERROR            0x01
#define        KB_A                0x04
#define        KB_B                0x05
#define        KB_C                0x06
#define        KB_D                0x07
#define        KB_E                0x08
#define        KB_F                0x09
#define        KB_G                0x0A
#define        KB_H                0x0B
#define        KB_I                0x0C
#define        KB_J                0x0D
#define        KB_K                0x0E
#define        KB_L                0x0F
#define        KB_M                0x10
#define        KB_N                0x11
#define        KB_O                0x12
#define        KB_P                0x13
#define        KB_Q                0x14
#define        KB_R                0x15
#define        KB_S                0x16
#define        KB_T                0x17
#define        KB_U                0x18
#define        KB_V                0x19
#define        KB_W                0x1A
#define        KB_X                0x1B
#define        KB_Y                0x1C
#define        KB_Z                0x1D
#define        KB_1                0x1E
#define        KB_2                0x1F
#define        KB_3                0x20
#define        KB_4                0x21
#define        KB_5                0x22
#define        KB_6                0x23
#define        KB_7                0x24
#define        KB_8                0x25
#define        KB_9                0x26
#define        KB_0                0x27
#define        KB_ENTER            0x28
#define        KB_ESC                0x29
#define        KB_BACKSPACE        0x2A
#define        KB_TAB                0x2B
#define        KB_SPACE            0x2C
#define        KB_MIUS                0x2D    //-_
#define        KB_EQUAL            0x2E    //=+
#define        KB_LSQUBRACKET        0x2F    //[{
#define        KB_RSQUBRACKET        0x30    //]}
#define        KB_BACKSLASH        0x31    //\|
#define        KB_SEMICOLON        0x33    //;:
#define        KB_APOSTROPHE        0x34    //'"
#define        KB_TILDE            0x35    //`~
#define        KB_COMMA            0x36    //,<
#define        KB_FULLSTOP            0x37    //.>
#define        KB_SLASH            0x38    ///?
#define        KB_CAPSLOCK            0x39   
#define        KB_F1                0x3A
#define        KB_F2                0x3B
#define        KB_F3                0x3C
#define        KB_F4                0x3D
#define        KB_F5                0x3E
#define        KB_F6                0x3F
#define        KB_F7                0x40
#define        KB_F8                0x41
#define        KB_F9                0x42
#define        KB_F10                0x43
#define        KB_F11                0x44
#define        KB_F12                0x45
#define        KB_PRTSCR            0x46
#define        KB_SCOLLLOCK        0x47
#define        KB_PAUSE            0x48
#define        KB_INSERT            0x49
#define        KB_HOME                0x4A
#define        KB_PAGEUP            0x4B
#define        KB_DELETE            0x4C
#define        KB_END                0x4D
#define        KB_PAGEDOWN            0x4E
#define        KB_RARROW            0x4F
#define        KB_LARROW            0x50
#define        KB_DARROW            0x51
#define        KB_UARROW            0x52
#define        KB_NUMLOCK            0x53
#define        KB_PAD_DIV            0x54    ///
#define        KB_PAD_MUL            0x55    //*
#define        KB_PAD_MINUS        0x56    //-
#define        KB_PAD_PLUS           0x57    //+
#define        KB_PAD_ENTER        0x58
#define        KB_PAD_1            0x59
#define        KB_PAD_2            0x5A
#define        KB_PAD_3            0x5B
#define        KB_PAD_4            0x5C
#define        KB_PAD_5            0x5D
#define        KB_PAD_6            0x5E
#define        KB_PAD_7            0x5F
#define        KB_PAD_8            0x60
#define        KB_PAD_9            0x61
#define        KB_PAD_0            0x62
#define        KB_PAD_DEL           0x63
#define        KB_APPLICATION        0x65  //RWin右边键
#define        Left_Ctrl	         0xE0	
#define        Left_Shift         0xE1	
#define        Left_Alt	          0xE2	
#define        Left_GUI	          0xE3	
#define        Right_Ctrl          0xE4	
#define        Right_Shift         0xE5	
#define        Right_Alt	         0xE6	
#define        Right_GUI	         0xE7
#define        Fn                  0xE8

#define COL0 0
#define COL1 1
#define COL2 2
#define COL3 3
#define COL4 4
#define COL5 5
#define COL6 6
#define COL7 7
#define COL8 8
#define COL9 9
#define COL10 10
#define COL11 11
#define COL12 12
#define COL13 13
#define COL14 14

#define RGB_SWITCH_CLOSE           KB_0
#define RGB_SWITCH_OPEN            KB_1
//
#define RGB_MODE_STATIC            KB_2 
#define RGB_MODE_PRESS             KB_3
#define RGB_MODE_PRESS_RANDOM      KB_4
#define RGB_MODE_LOVE              KB_5
//
#define RGB_COLOR_WHITE            KB_W
#define RGB_COLOR_RED              KB_R
#define RGB_COLOR_ORANGE           KB_O
#define RGB_COLOR_PINK             KB_P
#define RGB_COLOR_CYAN             KB_C
#define RGB_COLOR_VIOLET           KB_V
#define RGB_COLOR_BLUE             KB_B
//
//
#define KEYBOARDROW  6              //键盘行数
#define KEYBOARDCOL  16             //键盘列数
#define K_MAX_THRESHOLD 100         //按键相同次数(阈值)

//键盘状态
typedef enum{  
  KEYBOARD_IDLE,
  KEYBOARD_READY_SCAN,
  KEYBOARD_READY_FINISH,
  KEYBOARD_SCAN_RUNNING,
  KEYBOARD_SCAN_OVER,
}Keyboard_State;

typedef enum{
  NO_EFFECT = (uint8_t)0,
  PENDING_EFFECT,
  EFFECT
}EffectState;

typedef enum{
    SWITCH_TYPE = (uint8_t)0,
    COLOR_TYPE,
    MODE_TYPE,
}RGB_CommandTypeDef;

typedef struct{
  uint32_t    time;
  uint8_t     rgb_command;
  EffectState effect_state;
  uint8_t     repeat_transmit_count;
}RGB_Mode;

typedef struct {
  uint8_t  pending_quantity;
  uint8_t  last_time_command;
  RGB_Mode rgb_mode[3];
}ModeManage;

typedef struct{
  volatile uint8_t check;
  volatile uint8_t temp_check;
  volatile uint8_t function_key_flag;
  volatile uint8_t function_key;
  NKRO_BufferManage  nkro_buffer_manage;
}BufferManage;

typedef struct{
  uint8_t          *usb_report;
  NKRO_Report       nkro_report;
}ReportManage;

typedef struct RewindBuffer_InitTypeDef{
  uint8_t  buffer_size;
  uint8_t  read_pointer;
  uint8_t  write_pointer;
  uint8_t* buffer;
}RewindBuffer_InitTypeDef;

#ifdef KEYBOARD_RGB

#define K_POSITION_LIST_SIZE 15     

//键盘按键位置缓冲
typedef struct{
  uint8_t pos_top;
  volatile uint8_t list[K_POSITION_LIST_SIZE];
}PosList;

typedef struct{
  uint8_t pos_update_flag;
  PosList pos_list;
  RingList_InitTypeDef ring_structure;
}PosManage;

#endif

typedef struct{
  MarkList_InitTypeDef     *mark_list;
  RewindBuffer_InitTypeDef *rewindbuf;
  //键盘发送
  BufferManage *buffer_manage;
  ReportManage *report_manage;
  // //键盘按键缓冲
  #ifdef KEYBOARD_RGB
  //rgb模式管理
  ModeManage *mode_manage;
  //键盘按键位置存储
  PosManage  *position_manage;
  #endif
}TypeManage;

extern Keyboard_State KeyboardState;//键盘状态
extern TIM_HandleTypeDef *keyboard_tim_1ms;     //键盘扫描频率1ms定时器
extern TypeManage type_manage;

void KeyboardInit(void); //键盘初始化
void setKeyboardRowPin(GPIO_PinState PinState);//对所有的行置位
void keyboardStart(void); //开启键盘扫描
uint8_t getKey(uint16_t GPIO_Pin);
 
#ifdef KEYBOARD_RGB
void setMode(uint8_t command);
void resetMode(uint8_t command);
void transmitCommand(void);
void transmitPosition(void);
#endif

bool RewindBuf_Init(TypeManage *type_manage,RewindBuffer_InitTypeDef *rewindbuf);
bool RewindBuf_Write(struct RewindBuffer_InitTypeDef *rewindbuf,uint8_t elem);
int  RewindBuf_Read(struct RewindBuffer_InitTypeDef *rewindbuf);
void RewindBuf_Rewind(struct RewindBuffer_InitTypeDef *rewindbuf);
void RewindBuf_Clear(struct RewindBuffer_InitTypeDef *rewindbuf);
bool RewindBuf_Is_Full(struct RewindBuffer_InitTypeDef *rewindbuf);
bool RewindBuf_Is_Empty(struct RewindBuffer_InitTypeDef *rewindbuf);
#endif

