/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "ST7789_Driver.h"
#include "xl1278smt_driver.h"
#include "W25QXX_Driver.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "math.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//typedef enum {
//	KEY_STATE_DOWN = 0,
//	KEY_STATE_PRESS,
//	KEY_STATE_UP
//
//} Key_StateTypeDef;

typedef enum {
	KEY_NOEVENT = 0,
	KEY_DOWN,
	KEY_PRESS,
	KEY_UP,
	KEY_WFN,
	KEY_LOCK

} Key_EventTypeDef;


typedef struct {
	Key_EventTypeDef	KeyEvent;

	bool 		Shift;

	int			KeyCode;

	int			Modifiers;

	bool		Handled;

	void		(* KeyEventHandler)(void);

	void		(* KeyDownHandler)(void);

	void		(* KeyPressHandler)(void);

	void		(* KeyUpHandler)(void);

} Key_EventArgsTypeDef;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RAM2 __attribute__((section (".ram2")))

#define LORATEST_MODE_TX			0
#define LORATEST_MODE_RX			1
#define LoRaTest_STATE_IDLE			0
#define LoRaTest_STATE_BUSY			1
#define LoRaTest_STATE_WAITFORIDLE	2
#define LORATEST_PKTSIZE			32




#define KEY_0			0
#define KEY_1			1
#define KEY_2			2
#define KEY_3			3



#define LED_WHITE()		do { \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET); \
      	  	  	  	  	} while(0)

#define LED_RED()		do { \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET); \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET); \
      	  	  	  	  	} while(0)

#define LED_GREEN()		do { \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_2, GPIO_PIN_SET); \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); \
      	  	  	  	  	} while(0)

#define LED_BLUE()		do { \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET); \
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); \
      	  	  	  	  	} while(0)

#define LED_DeInit()	do { \
		  	  	  	  	  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2); \
	  	  				} while(0)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint32_t Get_SysTime(void);
void System_EnterSleep(void);
void System_EnterStop(void);
Key_EventTypeDef Key_GetEvent(void);
void Key_ClearEvent(void);
void Key_WaitForNext(void);
void Key_Lock(void);
void Key_UnLock(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* 系统参数 */
uint32_t __SYS_TIME__ = 0;
uint32_t Screen_State = 0;
uint32_t Screen_Off_Timeout = 0xfffffff;
uint32_t Screen_Off_Timeout_End;
uint32_t MCU_PowerOff_Timeout = 0xfffffff;
uint32_t MCU_PowerOff_Timeout_End;


/* 输入输出设备 */
int click_num = 0;
Key_EventTypeDef key_event = KEY_NOEVENT;
int press_cnt = 0;
Key_EventArgsTypeDef Key_EventArgs = {0};

/* LoRa测试模式参数 */
int LoRaTest_Mode = LORATEST_MODE_RX;
int LoRaTest_State = LoRaTest_STATE_IDLE;
RAM2 uint8_t LoRaTest_TxData[256] = {0};
RAM2 uint8_t LoRaTest_RxData[256] = {0};
int LoRaTest_SendPktNum = 0;
int LoRaTest_RecvPktNum = 0;
int LoRaTest_SER = 0;
/**
 * @retval	系统运行时间
 */
uint32_t Get_SysTime(void)
{
	return __SYS_TIME__;
}

void LCD_PowerOff(void)
{
//	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);
}

void LCD_PowerOn(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
}

/**
 *
 */
void LCD_Stop(void)
{
	HAL_SPI_DeInit(&hspi3);
	HAL_GPIO_DeInit(LCD_BL_PORT, LCD_BL_PIN);
	HAL_GPIO_DeInit(LCD_DC_PORT, LCD_DC_PIN);
	LCD_PowerOff();
}

/**
 *
 */
void LCD_WakeUp(void)
{
	HAL_SPI_Init(&hspi3);
	LCD_PowerOn();
	HAL_Delay(100);
	LCD_Init();
	lv_obj_t * scr_act = lv_scr_act();
	lv_scr_load(scr_act);
}


void LoRa_PowerOff(void)
{
//	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_5);
}

void LoRa_PowerOn(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
}

/**
 *
 */
void LoRa_Stop(void)
{
	HAL_SPI_DeInit(LoRa0.SPI_Inst);
	HAL_GPIO_DeInit(xl1278_DIO0_PORT, xl1278_DIO0_PIN);
	HAL_GPIO_DeInit(xl1278_NSS_PORT, xl1278_NSS_PIN);
	HAL_GPIO_DeInit(xl1278_RESET_PORT, xl1278_RESET_PIN);
	LoRa_PowerOff();

}

void LoRa_WakeUp(void)
{
	HAL_SPI_Init(&hspi1);
	LoRa_PowerOn();
}


void RS485_PowerOff(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
}

void RS485_PowerOn(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
}

/**
 *
 */
void RS485_Stop(void)
{
	HAL_UART_DeInit(&huart1);
	RS485_PowerOff();
}

void RS485_WakeUp(void)
{
	HAL_UART_Init(&huart1);
	RS485_PowerOn();
}


void System_GPIO_CLK_DISABLE(void)
{
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOD_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
}

void System_TIM_Init(void)
{
	MX_TIM2_Init();
	MX_TIM7_Init();
	MX_TIM15_Init();
}


void System_TIM_DeInit(void)
{
	HAL_TIM_Base_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim7);
	HAL_TIM_Base_DeInit(&htim15);
}

void System_SetWakeUpTime(uint32_t time_seconds)
{
	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, time_seconds, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}
/**
 * @brief	进入睡眠模式
 */
void System_EnterSleep(void)
{
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
}

/**
 * @brief	进入停止模式
 */
void System_EnterStop(void)
{
//	HAL_PWR_EnableWakeUpPin(GPIO_PIN_9);
	HAL_TIM_Base_Stop_IT(&htim7);
	HAL_TIM_Base_Stop_IT(&htim15);
	HAL_TIM_Base_Stop_IT(&htim2);
	LED_DeInit();
	LoRa_Stop();
	HAL_SPI_DeInit(&hspi2);
	LCD_Stop();
	RS485_Stop();
	/* 设置下次唤醒的时间 */
	System_SetWakeUpTime(15);
	HAL_SuspendTick();
//	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
	System_TIM_DeInit();
	System_GPIO_CLK_DISABLE();
    // 使能PWR时钟
//    __HAL_RCC_PWR_CLK_ENABLE();
    // 清除唤醒标记
//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
//    HAL_PWREx_EnableLowPowerRunMode();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	HAL_ResumeTick();
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

	/* 重新配置时钟 */
	SystemClock_Config();
	MX_GPIO_Init();
	System_TIM_Init();

	LoRa_WakeUp();
	HAL_SPI_Init(&hspi2);
	LCD_WakeUp();
	RS485_WakeUp();
	Key_WaitForNext();

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim7);
	HAL_TIM_Base_Start_IT(&htim15);
	Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
	MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
	LED_WHITE();

}

Key_EventTypeDef Key_GetEvent(void)
{

	if (Key_EventArgs.KeyEvent == KEY_DOWN) {
		Key_EventArgs.KeyEvent = KEY_PRESS;
		return KEY_DOWN;
	}

	if (Key_EventArgs.KeyEvent == KEY_UP) {
		Key_EventArgs.KeyEvent = KEY_NOEVENT;
		return KEY_UP;
	}

	return Key_EventArgs.KeyEvent;
}

void Key_ClearEvent(void)
{
	Key_EventArgs.KeyEvent = KEY_NOEVENT;
}

void Key_WaitForNext(void)
{
	Key_EventArgs.KeyEvent = KEY_WFN;
}

void Key_Lock(void)
{
	Key_EventArgs.KeyEvent = KEY_LOCK;
}

void Key_UnLock(void)
{
	Key_EventArgs.KeyEvent = KEY_NOEVENT;
}


void CheakSER(void)
{
	int SEN = 0;
	for (int i=0; i<LORATEST_PKTSIZE; i++) {
		if (LoRaTest_RxData[i] != LoRaTest_TxData[i])
			SEN++;
	}
	LoRaTest_SER = (float)SEN * 100 / LORATEST_PKTSIZE;
}




static lv_obj_t * scr_init;
static lv_obj_t * scr_main;
static lv_obj_t * scr1;
static lv_obj_t * scr2;
static lv_obj_t * scr3;
//static lv_obj_t * scr_setting;

static lv_style_t scr1_style;  //创建style
static lv_style_t scr2_style;  //创建style
static lv_style_t scr3_style;  //创建style

static lv_obj_t *scr2_lables[7] = {0};
//static lv_obj_t *scr_setting_lables[3] = {0};
int scr_sel_lab = 0;

lv_obj_t * label;
lv_obj_t * label2;
lv_obj_t * label3;
lv_obj_t * label4;
lv_obj_t * label5;
lv_obj_t * label6;
lv_obj_t * label7;
lv_obj_t * label8;
lv_obj_t * label9;
lv_obj_t * line0;
static lv_point_t line_points[] = { {120, 200}, {120, 200} };

lv_obj_t * slider1;
static lv_color_t bg_color;

void lv_scr_init(void)
{
	//创建屏幕
	scr_init = lv_obj_create(NULL);
	scr_main = lv_obj_create(NULL);
	scr1 = lv_obj_create(NULL);
	scr2 = lv_obj_create(NULL);
	scr3 = lv_obj_create(NULL);

	lv_style_init(&scr1_style);
	lv_style_set_bg_opa(&scr1_style, LV_OPA_COVER);
	bg_color = lv_color_make(255, 255, 255);
	lv_style_set_bg_color(&scr1_style, bg_color);
	lv_obj_add_style(scr1, &scr1_style,0);

	lv_style_init(&scr2_style);
	lv_style_set_bg_opa(&scr2_style, LV_OPA_COVER);
	bg_color = lv_color_make(0, 0, 0);
	lv_style_set_bg_color(&scr2_style, bg_color);
	lv_obj_add_style(scr2, &scr2_style, 0);

	lv_style_init(&scr3_style);
	lv_style_set_bg_opa(&scr3_style, LV_OPA_COVER);
	bg_color = lv_color_make(127, 127, 255);
	lv_style_set_bg_color(&scr3_style, bg_color);
	lv_obj_add_style(scr3, &scr3_style, 0);

}


void lv_ex_label(void)
{

	/* 传感器信息屏�??????????? */
	label = lv_label_create(scr1);
    lv_label_set_recolor(label, true);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label, 240);
    lv_obj_set_height(label, 20);
    lv_label_set_text_fmt(label, "#000000 Hello, World!#");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

	label2 = lv_label_create(scr1);
    lv_label_set_recolor(label2, true);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label2, 240);
    lv_obj_set_height(label2, 20);
    lv_label_set_text_fmt(label2, "#000000 click: %d#", click_num);
    lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 0, 20);

	label7 = lv_label_create(scr1);
    lv_label_set_recolor(label7, true);
    lv_label_set_long_mode(label7, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label7, 240);
    lv_obj_set_height(label7, 20);
    lv_label_set_text_fmt(label7, "#000000 time: %lu#", __SYS_TIME__);
    lv_obj_align(label7, LV_ALIGN_BOTTOM_LEFT, 0, 0);


    line0 = lv_line_create(scr1);
    lv_line_set_points(line0, line_points, sizeof(line_points) / sizeof(lv_point_t));
    lv_obj_set_style_line_width(line0, 5, LV_PART_MAIN);

}

void lv_scr2_init(void)
{
    scr2_lables[0] = lv_label_create(scr2);
    lv_label_set_recolor(scr2_lables[0], true);
    lv_label_set_long_mode(scr2_lables[0], LV_LABEL_LONG_CLIP); /*Circular scroll*/
//    lv_obj_set_width(scr2_lables[0], 240);
//    lv_obj_set_height(scr2_lables[0], 20);
    lv_label_set_text_fmt(scr2_lables[0], "#ffffff LoRa Test#");
    lv_obj_align(scr2_lables[0], LV_ALIGN_TOP_MID, 0, 0);

    scr2_lables[1] = lv_label_create(scr2);
    lv_label_set_recolor(scr2_lables[1], true);
    lv_label_set_long_mode(scr2_lables[1], LV_LABEL_LONG_CLIP); /*Circular scroll*/
    if (LoRaTest_Mode == LORATEST_MODE_TX) {
    	lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: TX#");
    }
    else {
    	lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: RX#");
    }

    lv_obj_align(scr2_lables[1], LV_ALIGN_TOP_MID, 0, 60);


    scr2_lables[2] = lv_label_create(scr2);
    lv_label_set_recolor(scr2_lables[2], true);
    lv_label_set_long_mode(scr2_lables[2], LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_label_set_text_fmt(scr2_lables[2], "#ffffff Start!#");
    lv_obj_align(scr2_lables[2], LV_ALIGN_TOP_MID, 0, 80);


    scr2_lables[3] = lv_label_create(scr2);
    lv_label_set_recolor(scr2_lables[3], true);
    lv_label_set_long_mode(scr2_lables[3], LV_LABEL_LONG_WRAP); /*Circular scroll*/
    lv_obj_set_width(scr2_lables[3], 240);
    lv_obj_set_height(scr2_lables[3], 20);
    lv_obj_align(scr2_lables[3], LV_ALIGN_CENTER, 0, 0);


    scr2_lables[4] = lv_label_create(scr2);
    lv_label_set_recolor(scr2_lables[4], true);
    lv_label_set_long_mode(scr2_lables[4], LV_LABEL_LONG_WRAP); /*Circular scroll*/
    lv_obj_set_width(scr2_lables[4], 240);
    lv_obj_set_height(scr2_lables[4], 100);
    lv_obj_align(scr2_lables[4], LV_ALIGN_TOP_LEFT, 0, 200);

}



void scr_sel_lable(int sel_num)
{
	static lv_style_t sel_style;

	lv_style_init(&sel_style);
	lv_style_set_radius(&sel_style, 5);
	lv_style_set_bg_opa(&sel_style, LV_OPA_COVER);
	bg_color = lv_color_make(100, 100, 200);
	lv_style_set_bg_color(&sel_style, bg_color);

	for (int i=0; i<3; i++) {
		if (i != sel_num) {
			lv_obj_remove_style(scr2_lables[i], &sel_style, 0);
		}
		else {
			lv_obj_add_style(scr2_lables[i], &sel_style, 0);
		}
	}
}


/**
 * @brief	RTC唤醒回调函数
 */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_5) {
		if (LoRa0.State == DEVICE_STATE_BUSY_TX) {
			LoRa0.TxCpltCallback();
		}
		else if (LoRa0.State == DEVICE_STATE_BUSY_RX) {
			LoRa0.RxCpltCallback();
		}
	}
}

void xl1278_TxCpltCallback(void)
{
	xl1278_ClearIrq(LoRa0.SPI_Inst);

	if (LoRaTest_State == LoRaTest_STATE_BUSY) {
		LoRaTest_SendPktNum++;

		xl1278_TxPacket(LoRa0.SPI_Inst, LoRaTest_TxData, LORATEST_PKTSIZE);
		lv_label_set_text_fmt(scr2_lables[4], "#ffffff Send Pkt Num: %d#", LoRaTest_SendPktNum);
	}
	else if (LoRaTest_State == LoRaTest_STATE_WAITFORIDLE) {
		xl1278_SetSleep(LoRa0.SPI_Inst);
		LoRa0.State = DEVICE_STATE_IDLE;
		lv_label_set_text_fmt(scr2_lables[2], "#ffffff Start!#");
		lv_label_set_text_fmt(scr2_lables[3], " ");
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
		LoRaTest_State = LoRaTest_STATE_IDLE;
	}
	else {
		xl1278_SetSleep(LoRa0.SPI_Inst);
		LoRa0.State = DEVICE_STATE_IDLE;
	}
}

void xl1278_RxCpltCallback(void)
{
	uint16_t recv_pktsize;
	int snr;
	int rssi;

	xl1278_ClearIrq(LoRa0.SPI_Inst);
	xl1278_RxPacket(LoRa0.SPI_Inst, LoRaTest_RxData, &recv_pktsize);
	if (LoRaTest_State == LoRaTest_STATE_BUSY) {
		if (recv_pktsize == LORATEST_PKTSIZE) {
			LoRaTest_RecvPktNum++;
			CheakSER();

			xl1278_GetSNR(LoRa0.SPI_Inst, &snr);
			xl1278_GetRSSI(LoRa0.SPI_Inst, &rssi);
			lv_label_set_text_fmt(scr2_lables[4], "#ffffff Rcvd Pkt Num: %d# \n#ffffff SNR: %d dB# \n#ffffff RSSI: %d dBm# \n#ffffff SER: %d %%#",
					LoRaTest_RecvPktNum, snr, rssi, LoRaTest_SER);
		}
		else {
			lv_label_set_text_fmt(scr2_lables[3], "#ff0000 Multiple Transmitters!#");
		}
	}
	else {
		xl1278_SetSleep(LoRa0.SPI_Inst);
		LoRa0.State = DEVICE_STATE_IDLE;
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
	{

	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim7)
	{
		lv_tick_inc(5);
	}
	else if (htim == &htim15)
	{
		if ((GPIOB->IDR & (GPIO_PIN_8 | GPIO_PIN_9)) != (GPIO_PIN_8 | GPIO_PIN_9)) {
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET) {
				if (Key_EventArgs.KeyEvent == KEY_NOEVENT) {
					Key_EventArgs.KeyEvent = KEY_DOWN;
				}
				else if (Key_EventArgs.KeyEvent == KEY_PRESS) {
					press_cnt++;
				}

				Key_EventArgs.KeyCode = KEY_0;
				LCD_DisplayOn();
				Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
				MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
			}
			else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == GPIO_PIN_RESET) {
				if (Key_EventArgs.KeyEvent == KEY_NOEVENT) {
					Key_EventArgs.KeyEvent = KEY_DOWN;
				}
				else if (Key_EventArgs.KeyEvent == KEY_PRESS) {
					press_cnt++;
				}

				Key_EventArgs.KeyCode = KEY_1;
				LCD_DisplayOn();
				Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
				MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
			}
		}
		else {
			if (Key_EventArgs.KeyEvent != KEY_NOEVENT)
			{
				if (Key_EventArgs.KeyEvent == KEY_LOCK)
					Key_EventArgs.KeyEvent = KEY_LOCK;
				else if (Key_EventArgs.KeyEvent == KEY_WFN)
					Key_EventArgs.KeyEvent = KEY_NOEVENT;
				else
					Key_EventArgs.KeyEvent = KEY_UP;

				press_cnt = 0;
			}

		}

	}
	else if (htim == &htim2)
	{
		__SYS_TIME__++;

		if (LoRaTest_State != LoRaTest_STATE_IDLE) {
			Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
			MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
		}
		if (Get_SysTime() > Screen_Off_Timeout_End) {
			LCD_DisplayOff();
		}
	}
}

uint8_t reg_read;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_TIM15_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
//  LoRa_Init();
//  while (0) {
//	  if (LoRa0.State == DEVICE_STATE_IDLE) {
//		  xl1278_TxPacket(LoRa0.SPI_Inst, LoRaTest_TxData, LORATEST_PKTSIZE);
////		  xl1278_SetRxContConfig(LoRa0.SPI_Inst);
//		  LoRa0.State = DEVICE_STATE_BUSY_TX;
//	  }
//
//	  xl1278_RegRead(LoRa0.SPI_Inst, xl1278_RegOpMode, &reg_read);
//  }

  LCD_PowerOn();
  LoRa_PowerOn();
  RS485_PowerOn();

  LED_WHITE();
  HAL_Delay(100);

  for (int i=0; i<LORATEST_PKTSIZE; i++) {
	  LoRaTest_TxData[i] = i;
  }


  W25QXX_Init();
  W25QXX_PowerDonw_Enable(&flash1);
//  W25QXX_Read(&flash1, (uint8_t *)&click_num, 0, 4);

  HAL_TIM_Base_Start_IT(&htim7);
  HAL_TIM_Base_Start_IT(&htim15);


  lv_init();
  lv_port_disp_init();
  lv_scr_init();
  lv_ex_label();
  lv_scr2_init();
  lv_scr_load(scr_init);

  lv_obj_t *init_label = lv_label_create(scr_init);
  lv_label_set_recolor(init_label, true);
  lv_label_set_long_mode(init_label, LV_LABEL_LONG_WRAP); /*Circular scroll*/
  lv_obj_set_width(init_label, 240);
//  lv_obj_set_height(init_label, 20);
  lv_label_set_text_fmt(init_label, "#000000 System Initializing...#");
  lv_obj_align(init_label, LV_ALIGN_CENTER, 0, 0);
  lv_task_handler();
  HAL_Delay(100);

  LoRa_Init();

  if (LoRa0.State == DEVICE_NOTFOUND) {
	  lv_label_set_text_fmt(init_label, "#ff0000 LoRa Device Not Found, # #ff0000 Please Cheak Your Device!# \r\n#000000 Press Any Key To Continue...#");
  }
  else {
	  lv_label_set_text_fmt(init_label, "#000000 Initialized Successfully.# \r\n#000000 Press Any Key To Continue...#");
  }
  lv_task_handler();
  HAL_Delay(100);
  Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
  MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
  HAL_TIM_Base_Start_IT(&htim2);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  HAL_Delay(4);
	  switch (Key_GetEvent()) {
  	  	  case KEY_NOEVENT: {

  	  		  break;
  	  	  }

	  	  case KEY_DOWN: {
			  lv_obj_t * scr_act = lv_scr_act();

			  if (scr_act == scr_init) {
				  Key_WaitForNext();
				  lv_scr_load(scr2);
				  scr_sel_lable(scr_sel_lab);
				  lv_obj_del(scr_init);
			  }
			  else if (scr_act == scr_main) {

			  }
			  else if (scr_act == scr1) {
				  click_num++;
				  lv_label_set_text_fmt(label2, "#000000 click: %d#", click_num);
//				  W25QXX_Erase_Sector(&flash1, 0);
//				  W25QXX_WritePage(&flash1, (uint8_t *)&click_num, 0, 4);
			  }

	  		  break;
	  	  }

	  	  case KEY_PRESS: {
			  lv_obj_t * scr_act = lv_scr_act();

			  if (scr_act == scr1) {
				  if (press_cnt > 60) {
					  Key_WaitForNext();
					  lv_scr_load(scr2);
					  scr_sel_lable(scr_sel_lab);
				  }
			  }
			  else if (scr_act == scr2) {
				  if (press_cnt > 60) {
					  Key_WaitForNext();
					  lv_scr_load(scr1);
				  }
			  }
	  		  break;
	  	  }

	  	  case KEY_UP: {
			  lv_obj_t * scr_act = lv_scr_act();

			  if (scr_act == scr2) {
				  if (LoRaTest_State == LoRaTest_STATE_IDLE) {
					  if (Key_EventArgs.KeyCode == KEY_0) {
						  LoRaTest_Mode = LORATEST_MODE_TX;
					  }
					  else if (Key_EventArgs.KeyCode == KEY_1) {
						  LoRaTest_Mode = LORATEST_MODE_RX;
					  }

					  LoRaTest_State = LoRaTest_STATE_BUSY;
					  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
					  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
					  lv_label_set_text_fmt(scr2_lables[2], "#ffff00 Waitting...#");
					  lv_task_handler();
					  LoRa_Init();
					  if (LoRa0.State == DEVICE_NOTFOUND) {
						  lv_label_set_text_fmt(scr2_lables[2], "#ffffff Start!#");
						  lv_label_set_text_fmt(scr2_lables[3], "#ff0000 LoRa Device Not Found, # #ff0000 Please Cheak Your Device!# \r\n#ffffff Press Any Key To Continue...#");
						  LoRaTest_State = LoRaTest_STATE_IDLE;
						  break;
					  }

					  lv_label_set_text_fmt(scr2_lables[2], "#ff0f0f Stop#");

					  if (LoRaTest_Mode == LORATEST_MODE_TX) {
						  LoRaTest_SendPktNum = 0;

						  lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: TX#");
						  lv_label_set_text_fmt(scr2_lables[3], "#ff0000 Transmitting Test Data...#");
						  lv_label_set_text_fmt(scr2_lables[4], "#ffffff Send Pkt Num: %d#", LoRaTest_SendPktNum);
						  xl1278_TxPacket(LoRa0.SPI_Inst, LoRaTest_TxData, LORATEST_PKTSIZE);
						  LoRa0.State = DEVICE_STATE_BUSY_TX;
					  }
					  else {
						  LoRaTest_RecvPktNum = 0;

						  lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: RX#");
						  lv_label_set_text_fmt(scr2_lables[3], "#ff0000 Receiving Test Data...#");
						  lv_label_set_text_fmt(scr2_lables[4], "#ffffff Rcvd Pkt Num: %d#", LoRaTest_RecvPktNum);
						  xl1278_SetRxContConfig(LoRa0.SPI_Inst);
						  LoRa0.State = DEVICE_STATE_BUSY_RX;
					  }
				  }
				  else {
					  if (LoRaTest_Mode == LORATEST_MODE_TX) {
						  lv_label_set_text_fmt(scr2_lables[2], "#ffff00 Waitting...#");
						  LoRaTest_State = LoRaTest_STATE_WAITFORIDLE;
					  }
					  else {
						  xl1278_SetSleep(LoRa0.SPI_Inst);
						  LoRa0.State = DEVICE_STATE_IDLE;
						  lv_label_set_text_fmt(scr2_lables[2], "#ffffff Start!#");
						  lv_label_set_text_fmt(scr2_lables[3], " ");
						  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
						  LoRaTest_State = LoRaTest_STATE_IDLE;
					  }
				  }
			  }

	  		  break;
	  	  } // KEY_UP事件结束

	  	  default: {
	  		  break;
	  	  }
	  }


	  lv_label_set_text_fmt(label7, "#000000 time: %lu#", __SYS_TIME__);
	  if (Get_SysTime() > MCU_PowerOff_Timeout_End) {
		  System_EnterStop();

	  }
	  lv_task_handler();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
