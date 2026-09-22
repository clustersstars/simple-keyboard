#ifndef RGB_DATA_H_
#define RGB_DATA_H_
#include <stdint.h>
#include "rgb_def.h"

#define COLOR_NUMBER 8 
#define RGB {0,0}
#define ColorNode uint8_t
 
//struct
typedef struct{
    uint8_t offset;
    uint8_t interval; //0<= Duration <= 2^8-1;(100ms)
}TimeNode;
//rgb_reset
extern const uint16_t rgb_reset[RGB_COL_NUM*24+1];
//color
extern const uint16_t* colorList[COLOR_NUMBER];
extern const uint16_t blue[24];
extern const uint16_t cyan[24];
extern const uint16_t orange[24];
extern const uint16_t pink[24];
extern const uint16_t red[24];
extern const uint16_t violet[24];
extern const uint16_t white[24];
extern const uint16_t black[24];
//map
extern const ColorNode love_group1_color[RGB_NUM];
extern const TimeNode love_group1_time[RGB_NUM];
extern const ColorNode love_group2_color[RGB_NUM];
extern const TimeNode love_group2_time[RGB_NUM];
extern const ColorNode love_group3_color[RGB_NUM];
extern const TimeNode love_group3_time[RGB_NUM];
#endif
