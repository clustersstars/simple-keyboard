# simple-keyboard
DIY 双MCU机械键盘固件

## 项目简介
双STM32架构：
- nkro_keyboard：主控MCU，实现按键扫描、USB NKRO键盘HID上报
- nkro_ws2812b：灯光从机MCU，WS2812 RGB灯带控制，UART与主控通信

## 开发环境
- IDE：Keil MDK5
- 库：STM32 HAL库
- 芯片：STM32F1系列

## 编译&烧录
1. 主控固件：`nkro_keyboard/MDK-ARM/nkro_keyboard.uvprojx`
   编译，烧录到主控STM32
2. 灯光固件：`nkro_ws2812b/MDK-ARM/nkro_ws2812b.uvprojx`
   编译，烧录灯光从机STM32

## 通讯协议
UART串口通信，主控下发RGB灯光指令至从机。

## 硬件说明
双MCU独立工作，串口互联；主控负责USB键盘，从机专职灯光
。
## 硬件信息
- PCB工程源文件（嘉立创EDA可直接打开修改）
- BOM物料清单（元件型号、封装、数量）
- gerber.zip 光绘文件包，可直接提交PCB工厂打板

## License
MIT
