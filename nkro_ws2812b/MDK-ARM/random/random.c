#include"random.h"
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef* random = &hadc1;
const uint32_t RANDOM_INIT_SEED[SEED_NUM] = {0x7F3A9B2C,0x1E5D8F4A,0x9C4B6D3E,0x3A7C9E5B,0xD2F8A1E4,0x5B6C8D2F,0x8E4F3A7B,0x2C9D5E1A,0xF1A3B7C9,0x4D6E8F2B};
uint32_t Generate_RandomSeed(void);
//random
void RGB_Srand_Init(void){
    srand(Generate_RandomSeed());
}
int Get_RandValue(void){
    return rand();
}
uint32_t Generate_RandomSeed(void){
    uint32_t seed = 0;
    HAL_ADCEx_Calibration_Start(random);
    for(int i=0;i<2;i++){
        HAL_ADC_Start(&hadc1);
			seed |= HAL_ADC_GetValue(random) << (16*(1-i));
    }
    seed |= RANDOM_INIT_SEED[seed%10];
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}
