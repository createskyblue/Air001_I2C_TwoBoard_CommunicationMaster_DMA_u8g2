/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) AirM2M.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "String.h"
#include "u8g2.h"
/* Private define ------------------------------------------------------------*/
#define I2C_ADDRESS      0xA0               /* 本机地址0xA0 */
#define I2C_SPEEDCLOCK   2000000             /* 通讯速度*/
#define I2C_DUTYCYCLE    I2C_DUTYCYCLE_16_9 /* 占空比 */
/* Private variables ---------------------------------------------------------*/
u8g2_t u8g2;
I2C_HandleTypeDef I2cHandle;
const uint8_t main_font_16[708] U8G2_FONT_SECTION("main_font_16") = 
  "\32\0\3\2\4\4\3\2\5\16\17\0\376\13\376\14\377\0\64\0\0\0\204\60\15\246\25K\213\222\320\307"
  "$\312$\0\61\11\245\66c\354O\203\0\62\15\246\25k\312\222,\15{\34\206\0\70\21\246\25gH"
  "\262QL\206(QBqK\206\4A\23\247\374.M\302$L\262(\213\262A\312\222\60\321\4U\24"
  "\247\364bR\302$L\302$L\302$L\302$\223\206\4g\21\226\5g\321\222,\311\222!\33\222P"
  "L\206\4i\12\244\26'\307\264\236\206\0r\14w\24c\222\226(n\34$\0\0\0\0\4\377\377N"
  "-\32\353\356\67\307\322\341\61\24C\61\24C\61\34\36\302j\216\345X\216\245\0R\250%\355\355\37I"
  "\207,Gsl\30r$\33\206(\213\322,J\263$\312*Y\224\15C\22\246I\30\247E\5T\10"
  "\37\355\355;\7\223\34\312bU\322!m\30rn\303\61\7\302\34\10s \34\216\71\20\1Vh*"
  "\355\355gP\6)K\262(K\262hP\6\35\313\241,\32\36\302,V%\35R\6e\220\262$\213"
  "\262$\213\6eP\0[W\35\355\355\67\207\343\341A\7u\60\32\6\35\314\301\34\214\207\207\70Gs"
  "\64\307f\0[\231\36\355\355\67\207\343\341A\7\325r\16<Ha),\15\7),\205\245\341 \345"
  "P\2^v\37\355\355+\7\322:\220\245\303AL\343\64N\343\64\33\36\262\264\234\306a\16d\71\222"
  "\1^\223#\355\355;\207\343\341%\314\201\60\7\222\341\220d\71\22ei\64\14R\234\306i\62\34\222"
  "\70\314\201\20b\20!\355\355\37\251cQ\16\245\303K\234\306Q\64HmQ[\224d\245$kJ\22"
  "-i\315\326\2bk \355\355+Gs\64\33\336\201(\207\352P%\7\42i\70\344P\35\252Cu"
  "\250\64\34t(c\317%\355\355++f\305hxH\262\60G\243a\310\242\66%\252L\303\220Em"
  "Q[\324\26\15C\62\245\11\0e\207\37\355\355\67\207s\64\36\36\262\64Ns \313\221,\207\222\34"
  "\314\301$\207\262X\225tHmK\42\355\355\37M\242A\213\222\254\230\264hIOI\307\244S\322S"
  "\322\307,\251%\241\30eZ\347\1u\37 \355\355;G\262\34\311rd\70Da\16\204q\232\243\71"
  "\62\334\221\34\315\321\34\215\207\207\0u\61\32\353\356\67\307r,\35\36C\61\24C\61\34\36\302P\14"
  "\305P\14\207\7\201\352\33\353\356\63\207r\340;\244C\303C\16\351\320\360\220C:\244C\303C\16\5"
  "\213\325\42\355\355'\7\222,\216\312\71\360\16es\32-i\26\245Y\224f\305\244\226D\322RJF"
  "\35\15\0";

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//U8g2显示驱动API移植
uint8_t u8x8_byte_hw_i2c_dma(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
  static uint8_t buf[128];
  static int p;
  switch(msg)
  {
    //初始化
    case U8X8_MSG_BYTE_INIT:
      break;
    //开始传输
    case U8X8_MSG_BYTE_START_TRANSFER: {
      p = 0;
    }break;
    //传输数据
    case U8X8_MSG_BYTE_SEND:
      //判断是否超出缓冲区大小
      if(p + arg_int > sizeof(buf)) {
        Error_Handler();
      }
      //将数据拷贝到缓冲区
      memcpy(buf + p, (uint8_t *)arg_ptr, arg_int);
      p+=arg_int;

      break;
    //传输结束
    case U8X8_MSG_BYTE_END_TRANSFER:{
        /*I2C主机DMA方式发送*/
        while (HAL_I2C_Master_Transmit_DMA(&I2cHandle, 0x3C << 1, buf, p) != HAL_OK)
        {
          Error_Handler();
        }
        /*判断当前I2C状态*/
        while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY);
    }break;
    default:return 0;
  }
  return 1;
}

uint8_t u8x8_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,U8X8_UNUSED void *arg_ptr){
  switch (msg){
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        HAL_Delay(1);
        break;
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;
  }
  return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
  * @brief  应用程序入口函数.
  * @retval int
  */
int main(void)
{
  /* 复位所有外设，初始化flash接口和systick */
  HAL_Init();
  BSP_USART_Config();

  /* I2C初始化 */
    I2cHandle.Instance             = I2C;                      /* I2C */
    I2cHandle.Init.ClockSpeed      = I2C_SPEEDCLOCK;           /* I2C通讯速度 */
    I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE;            /* I2C占空比 */
    I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;              /* I2C地址 */
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;  /* 禁止广播呼叫 */
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;    /* 允许时钟延长 */
    if (HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
      Error_Handler();
    }

  // //对u8g2和屏幕进行初始化
  // u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c_dma, u8x8_gpio_and_delay); // 初始化 u8g2 结构体
  // u8g2_InitDisplay(&u8g2);                                                                       // 根据所选的芯片进行初始化工作，初始化完成后，显示器处于关闭状态
  // u8g2_SetPowerSave(&u8g2, 0);                                                                   // 打开显示器
  // u8g2_SetFont(&u8g2,main_font_16);
  // u8g2_SetFontPosTop(&u8g2);
  

  // u8g2_ClearBuffer(&u8g2);
  // u8g2_DrawUTF8(&u8g2,0,2,"合宙Air001");
  // u8g2_DrawUTF8(&u8g2,0,2 + 16,"U8g2中文测试");
  // u8g2_DrawUTF8(&u8g2,0,2 + 32,"中文字库由生成器");
  // u8g2_DrawUTF8(&u8g2,0,2 + 48,"自动扫描并生成");
  // u8g2_SendBuffer(&u8g2);

  //扫描I2C设备地址并打印
  for (uint8_t i = 0; i < 128; i++)
  {
    if (HAL_I2C_IsDeviceReady(&I2cHandle, i << 1, 1, 10) == HAL_OK)
    {
      printf("I2C device found at address: 0x%02X\n", i);
    }else {
      printf("I2C device not found at address: 0x%02X\n", i);
    }
  }
		
  
  while(1)
  {
    printf("Are You Ok?\r\n");
    HAL_Delay(1000);
  }
}


/**
  * @brief  错误执行函数
  * @param  无
  * @retval 无
  */
void Error_Handler(void)
{
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  输出产生断言错误的源文件名及行号
  * @param  file：源文件名指针
  * @param  line：发生断言错误的行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* 用户可以根据需要添加自己的打印信息,
     例如: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* 无限循环 */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT AirM2M *****END OF FILE******************/
