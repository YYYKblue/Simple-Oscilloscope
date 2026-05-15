# STM32F103C8T6 简易数字示波器

本项目是一个基于 **STM32F103C8T6** 的简易数字示波器，使用 **STM32 HAL 库**开发，开发环境为 **Keil5 / STM32CubeMX**。项目通过 **TIM3 定时触发 ADC1**，并使用 **DMA** 将采样数据搬运到内存缓冲区，实现稳定的定时采样；随后对采样数据进行电压换算、FFT 测频、波形显示、频谱显示和参数显示。

项目软件代码部分由本人开发，包含示波器核心采样显示逻辑、OLED 显示驱动适配、软件 I2C、FFT 测频、按键控制、旋转编码器调节、AHT20 温湿度显示和菜单界面等内容。

---

## 项目功能

- 基于 STM32F103C8T6 的简易数字示波器系统
- ADC1 单通道采样，采样输入引脚为 `PA1 / ADC1_IN1`
- TIM3 定时器触发 ADC，实现稳定采样周期
- DMA1_Channel1 自动搬运 ADC 数据，降低 CPU 负担
- 每帧采样 256 点数据
- 支持 ADC 原始值到实际输入电压的换算
- 支持 FFT 测频
- 支持频谱显示
- 支持 OLED 时域波形显示
- 支持频率、最大值、最小值、峰峰值、占空比、波形类型、采样率显示
- 支持自动调节采样率和显示幅度
- 支持按键切换波形图、数据图、频谱图和暂停状态
- 支持旋转编码器调节显示周期数、垂直幅度和垂直偏移
- 支持 OLED 菜单界面
- 集成 AHT20 温湿度显示页面
- 集成 OLED 开机动画和作者信息页

---

## 开发环境

| 项目 | 内容 |
|---|---|
| 主控芯片 | STM32F103C8T6 |
| 开发库 | STM32 HAL Library |
| IDE | Keil uVision5 |
| 配置工具 | STM32CubeMX |
| 工程文件 | `MDK-ARM/oscilloscope.uvprojx` |
| CubeMX 配置文件 | `oscilloscope.ioc` |
| 显示屏 | 128 × 64 OLED |
| 通信方式 | 软件 I2C |
| 温湿度传感器 | AHT20 |

---

## 硬件资源分配

| 外设 / 模块 | 引脚 / 资源 | 说明 |
|---|---|---|
| ADC 输入 | `PA1 / ADC1_IN1` | 示波器模拟信号输入 |
| ADC | `ADC1` | 单通道采样 |
| DMA | `DMA1_Channel1` | ADC 数据搬运到内存 |
| 采样定时器 | `TIM3` | 通过 TRGO 触发 ADC |
| OLED SDA | `PB6` | 软件 I2C 数据线 |
| OLED SCL | `PB7` | 软件 I2C 时钟线 |
| AHT20 | `PB6 / PB7` | 与 OLED 共用软件 I2C 总线 |
| 旋转编码器 A/B | `PA8 / PA9` | TIM1 编码器模式 |
| 编码器读取定时器 | `TIM2` | 周期性读取编码器变化量 |
| 菜单按键 | `PB12 / PB13` | 菜单页面切换 |
| KEY0 | `PA0` | 波形图 / 数据图切换，频谱模式下退出频谱 |
| KEY2 | `PA2` | 暂停 / 继续 |
| KEY3 | `PA3` | 频谱图切换 |
| KEY4 | `PA4` | 自动调节 |
| KEY5 | `PA5` | 选择编码器调节对象 |
| 指示灯 | `PC13` | 板载 LED 状态指示 |
| 扩展 LED | `PB0 / PB1` | 程序运行或按键反馈 |

---

## 工程结构

```text
oscilloscope/
├── oscilloscope.ioc                 # STM32CubeMX 配置文件
├── 原理图                            #原理图
├── Core/
│   ├── Inc/                          # 头文件
│   │   ├── app_main.h                # 示波器核心逻辑声明
│   │   ├── FFT.h                     # FFT 相关声明
│   │   ├── oled.h                    # OLED 显示驱动声明
│   │   ├── sw_i2c.h                  # 软件 I2C 声明
│   │   ├── aht20.h                   # AHT20 传感器声明
│   │   ├── key_control.h             # 菜单按键控制声明
│   │   └── ...
│   │
│   └── Src/                          # 源文件
│       ├── main.c                    # 主函数、菜单页面调度
│       ├── app_main.c                # 示波器核心业务逻辑
│       ├── FFT.c                     # 快速傅里叶变换实现
│       ├── oled.c                    # OLED 显示驱动
│       ├── font.c                    # 字库数据
│       ├── sw_i2c.c                  # 软件 I2C 实现
│       ├── aht20.c                   # AHT20 温湿度读取
│       ├── key_control.c             # 菜单按键控制
│       ├── app_drivers.c             # LED、编码器等辅助驱动
│       ├── adc.c                     # ADC 初始化
│       ├── dma.c                     # DMA 初始化
│       ├── tim.c                     # TIM1/TIM2/TIM3 初始化
│       ├── gpio.c                    # GPIO 初始化
│       └── stm32f1xx_it.c            # 中断服务与回调逻辑
│
├── Drivers/                          # CMSIS 与 STM32 HAL 驱动库
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
│
└── MDK-ARM/
    ├── oscilloscope.uvprojx          # Keil5 工程文件
    ├── startup_stm32f103xb.s         # 启动文件
    └── DebugConfig/
```

---

## 核心模块说明

### 1. `main.c`

`main.c` 是程序入口，主要负责：

- HAL 库初始化
- 系统时钟初始化
- GPIO、DMA、ADC、TIM 等外设初始化
- AHT20 初始化
- 调用 `App_Main_Init()` 完成应用层初始化
- 根据 `g_menuState` 显示不同页面

菜单页面包括：

| 页面编号 | 页面内容 |
|---:|---|
| `0` | 主菜单 |
| `1` | 温湿度页面 |
| `2` | 示波器页面 |
| `3` | 作者信息页面 |

当 `g_menuState == 2` 时，主循环会调用：

```c
App_Main_Loop();
```

进入示波器主逻辑。

---

### 2. `app_main.c`

`app_main.c` 是整个项目的核心文件，主要实现：

- 应用层初始化
- TIM3 采样频率设置
- ADC + DMA 采样
- ADC 数值到实际电压的换算
- OLED 波形显示
- OLED 参数显示
- OLED 频谱显示
- FFT 测频
- 自动采样率调节
- 波形类型判断
- 示波器主循环调度

主要数据缓冲区：

```c
uint16_t ADCData[256];   // ADC 原始采样数据
float VOL[256];          // 换算后的电压数据，也会被复用为 FFT 幅值谱
complex data[256];       // FFT 复数输入/输出数组
```

主要状态变量：

```c
volatile uint8_t SHOW;        // 波形图 / 数据图切换
volatile uint8_t STOP;        // 暂停 / 继续
volatile uint8_t SPEC;        // 频谱模式
volatile uint8_t RIGHT;       // 自动调节状态
volatile uint8_t CODE_Choice; // 编码器当前调节对象
```

---

### 3. `FFT.c`

`FFT.c` 实现复数运算和 256 点 FFT。项目使用 FFT 完成两个功能：

1. **测量输入信号频率**  
   通过寻找频谱中幅值最大的频点，计算输入信号主频。

2. **频谱显示和波形判断**  
   通过分析基波和谐波分量，在 OLED 上显示频谱，并辅助判断正弦波、三角波、方波和锯齿波。

测频公式：

```text
frequency = index_MAX × Fre_Sam / ADCDataLength
```

其中：

| 变量 | 含义 |
|---|---|
| `index_MAX` | FFT 幅值最大频点编号 |
| `Fre_Sam` | 当前采样率 |
| `ADCDataLength` | FFT 点数，本项目为 256 |

---

### 4. `oled.c` 和 `font.c`

OLED 显示模块采用显存缓冲区方式：

```text
OLED_NewFrame()   清空显存
OLED_Draw...      绘制文字、点、线、图形
OLED_ShowFrame()  刷新到 OLED 屏幕
```

本项目 OLED 主要用于显示：

- 菜单页面
- 温湿度页面
- 示波器时域波形
- 示波器参数页面
- FFT 频谱页面
- 作者信息页面
- 开机动画

---

### 5. `sw_i2c.c`

项目使用软件 I2C 驱动 OLED 和 AHT20。

```text
PB6 -> SDA
PB7 -> SCL
```

软件 I2C 的优点是灵活，不依赖 STM32 硬件 I2C 外设；

注：本项目最初使用硬件I2C，但是由于电路板设计错误，改成软件模拟I2C。

---

### 6. `aht20.c`

`aht20.c` 负责读取 AHT20 温湿度传感器数据，并在菜单的温湿度页面中显示当前环境温度和湿度。

---

### 7. `stm32f1xx_it.c`

该文件包含中断服务函数和 HAL 回调函数。项目中比较关键的回调包括：

- ADC + DMA 采样完成回调
- 外部按键中断回调
- TIM2 定时器回调

ADC 采样完成回调：

```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        HAL_TIM_Base_Stop(&htim3);
        ADC_Sign = 1;
    }
}
```

该回调在 DMA 搬运满 256 个 ADC 数据后触发，用于停止 TIM3 并通知主程序当前一帧采样完成。

---

## ADC + DMA + TIM3 采样原理

本项目的采样链路是整个示波器的核心：

```text
TIM3 定时产生 Update 事件
↓
TIM3 TRGO 触发 ADC1
↓
ADC1 采样 PA1 模拟输入
↓
DMA1_Channel1 将 ADC 转换结果搬运到 ADCData[]
↓
搬运满 256 个点后触发采样完成回调
↓
停止 TIM3，置位 ADC_Sign
↓
主程序将 ADC 原始值转换为实际电压
```

### TIM3 触发 ADC

ADC1 配置为外部触发模式：

```c
hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
```

TIM3 配置为 Update 事件输出 TRGO：

```c
sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
```

因此，TIM3 每更新一次，就会触发 ADC1 采样一次。

---

### 采样率设置

采样率由 `Set_SamplingFre_HAL(float fs)` 设置。该函数通过修改 TIM3 的 `PSC` 和 `ARR` 控制 TIM3 更新频率。

定时器采样率关系：

```text
Fs = 72 MHz / (PSC × ARR)
```

代码会根据目标采样率自动寻找合适的 `PSC` 和 `ARR`，并保证 `ARR` 不超过 16 位定时器范围。

---

### DMA 搬运

DMA 配置为普通模式：

```c
hdma_adc1.Init.Mode = DMA_NORMAL;
```

每次采样函数启动后，DMA 会依次把 ADC 结果写入：

```text
ADCData[0]
ADCData[1]
ADCData[2]
...
ADCData[255]
```

当 256 个点全部采集完成后，触发 DMA 传输完成中断，最终进入 `HAL_ADC_ConvCpltCallback()`。

---

### 电压换算

ADC 原始值转换为 ADC 引脚电压：

```text
vadc = ADCData / 4095 × 3.3
```

随后根据前端调理电路的偏置和比例系数换算为实际输入电压：

注：此外围偏置电路设计存在问题，且放大器及相关电阻存在误差，最终偏置公式根据实际情况修改，但仍存在误差，此为可优化点之一。

```c
Vin = (vadc - 1.61f) / 0.1625f;
```

其中：

| 参数 | 含义 |
|---|---|
| `1.61f` | 零点偏置电压 |
| `0.1625f` | 输入电压到 ADC 电压的缩放系数 |

实际量程与精度取决于前端硬件调理电路和标定结果。

---

## FFT 测频原理

项目每次采集 256 点电压数据后，将其作为 FFT 输入：

```c
data[count].real = VOL[count];
data[count].imag = 0;
fft(ADCDataLength, data);
```

FFT 将时域波形转换为频域数据。转换后，每个频点对应一个复数结果，代码通过复数求模得到幅值：

```c
sqrt(real * real + imag * imag)
```

然后跳过直流分量，只在正频率范围内寻找最大幅值点：

```c
for (count = 1; count < ADCDataLength / 2; count++)
```

找到最大幅值点后，用下面公式换算出实际频率：

```text
frequency = index_MAX × Fre_Sam / 256
```

因此，FFT 在本项目中的作用是：

> 将 ADC 采集到的时域波形转换为频域频谱，再通过寻找主频峰值实现输入信号频率测量。

---

## 显示模式

示波器页面支持三种主要显示模式：

| 模式 | 显示内容 |
|---|---|
| 时域波形图 | 显示输入信号随时间变化的波形 |
| 参数数据页 | 显示频率、最大值、最小值、峰峰值、占空比、波形类型和采样率 |
| 频谱图 | 显示 FFT 后的频域幅值分布 |

时域波形显示中，程序会寻找波形穿越显示中点的上升沿作为绘制起点，从而实现简易触发效果，减少波形左右漂移。

---

## 按键与编码器说明

### 菜单按键

| 按键 | 引脚 | 功能 |
|---|---|---|
| 菜单上翻 | `PB12` | 页面编号增加 |
| 菜单下翻 | `PB13` | 页面编号减少 |

### 示波器控制按键

| 按键 | 引脚 | 功能 |
|---|---|---|
| KEY0 | `PA0` | 波形图 / 数据图切换；频谱模式下退出频谱 |
| KEY2 | `PA2` | 暂停 / 继续 |
| KEY3 | `PA3` | 进入 / 退出频谱显示 |
| KEY4 | `PA4` | 触发自动调节 |
| KEY5 | `PA5` | 选择编码器调节对象 |

### 旋转编码器

旋转编码器接入 `PA8 / PA9`，使用 TIM1 编码器模式。TIM2 周期性读取编码器增量。

| `CODE_Choice` | 编码器调节对象 |
|---:|---|
| `0` | `WaveNum`，屏幕显示周期数 |
| `1` | `VPP`，垂直显示电压范围 |
| `2` | `MIN_WAVE`，垂直显示偏移 |

---

## 自动调节逻辑

按下自动调节按键后，程序会：

1. 使用 FFT 测量当前输入信号频率；
2. 根据测得频率调整采样率；
3. 使一周期大约包含 16 个采样点；
4. 重新采样一帧波形；
5. 根据当前波形最大值和最小值自动调整 OLED 垂直显示范围。

核心目标是让波形在 OLED 屏幕上显示得更加稳定、完整、易观察。

---

## 编译与烧录

1. 使用 Keil5 打开工程文件：

   ```text
   MDK-ARM/oscilloscope.uvprojx
   ```

2. 确认芯片型号为：

   ```text
   STM32F103C8T6
   ```

3. 编译工程。

4. 使用 ST-Link、J-Link 或其他下载器烧录程序。

5. 连接 OLED、按键、编码器、AHT20 和模拟输入信号。

6. 上电后进入 OLED 菜单，通过 `PB12 / PB13` 切换到示波器页面。

---

## 当前项目特点

- 采样链路使用硬件定时器触发，采样周期稳定；
- DMA 自动搬运采样数据，减少 CPU 干预；
- 使用 FFT 实现频率测量和频谱显示；
- 通过 OLED 菜单组织多个功能页面；
- 支持按键和编码器交互，具备较完整的人机交互逻辑；
- 代码模块划分清晰，便于后续维护和扩展。

---

## 后续优化方向

- 外围偏置电路由sj现学现卖（doge），选型设计存在一些小问题，后续可以进行优化，原理图已上传。
- 最后成品没有做到小巧，后续可以进行3d打印外壳，电路板优化设计，做到小巧便携
- 示波器绘制更倾向于周期信号，对于非周期信号，绘制效果稍差，后续可以优化代码
- 频谱分析，波形判断默认信号为周期信号，波形判断只能判断正弦波，方波，三角波，锯齿波。如有需要，可以进行代码优化

---

## 项目说明

本项目作为 STM32 嵌入式综合实践项目，涉及 ADC、DMA、定时器、外部中断、OLED 显示、软件 I2C、FFT 算法和人机交互等内容。项目重点不只是实现基本波形显示，而是围绕简易示波器的采样、分析、显示和交互流程，完成了一个较完整的小型嵌入式仪器系统。

## 致谢
- 感谢sj的硬件方面支持
- 感谢keysking的hal库教学视频（全b站最好的stm32教学视频），该项目还用到了他的取模助手以及oled驱动
- 示波器部分参考b站DIO-KING的标准库代码
