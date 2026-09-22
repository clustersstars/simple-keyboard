# simple-keyboard
DIY 双MCU机械键盘固件。
- 主控MCU：`nkro_keyboard`，STM32，USB NKRO键盘，负责按键扫描、USB HID上报
- 灯光从机MCU：`nkro_ws2812b`，STM32，驱动WS2812B RGB灯，通过UART和主控通信

## 硬件架构
双MCU独立运行，串口通信。
主控负责按键检测、USB键盘报告；从机只管理RGB灯光。

## 开发环境
- Keil MDK5
- STM32 HAL库
- USB HID协议

## 编译说明
1. nkro_keyboard：打开MDK工程，编译烧录主控MCU
2. nkro_ws2812b：打开MDK工程，编译烧录灯光MCU

## 通讯协议
UART，主控下发灯光指令给从机。

## License
MIT
