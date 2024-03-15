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
#include "adc.h"
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
	KEY_WFN,	//失效按键，直到下�???次按�???
	KEY_LOCK

} Key_EventTypeDef;

typedef enum {
	STICK_STATE_CENTER = 0,
	STICK_STATE_0,
	STICK_STATE_90,
	STICK_STATE_180,
	STICK_STATE_270,
} Stick_StateTypeDef;

typedef enum {
	STICK_NOEVENT = 0,
	STICK_CLICK,
	STICK_UP,
	STICK_DOWN,
	STICK_RIGHT,
	STICK_LEFT,
	STICK_LOCK
} Stick_EventTypeDef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RAM2 __attribute__((section (".ram2")))

#define LORATEST_MODE_TX			0
#define LORATEST_MODE_RX			1
#define LoRaTest_STATE_IDLE			0
#define LoRaTest_STATE_BUSY			1
#define LoRaTest_STATE_WAITFORIDLE	2
#define LORATEST_PKTSIZE			255

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
uint32_t Screen_Off_Timeout = 30;
uint32_t Screen_Off_Timeout_End;
uint32_t MCU_PowerOff_Timeout = 45;
uint32_t MCU_PowerOff_Timeout_End;


/* 输入输出设备 */
int click_num = 0;
Key_EventTypeDef key_event = KEY_NOEVENT;
Stick_StateTypeDef stick_state = STICK_STATE_CENTER;
Stick_EventTypeDef stick_event = STICK_NOEVENT;
static int press_cnt = 0;

int x = 0;
int y = 0;
int x_max = 4038;
int x_min = 0;
int x_mid = 2000;
int y_max = 4038;
int y_min = 0;
int y_mid = 2011;
float disc_x = 0;
float disc_y = 0;
int move_flag = 0;

/* LoRa测试模式参数 */
int LoRaTest_Mode = LORATEST_MODE_TX;
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
	HAL_TIM_Base_Stop_IT(&htim7);
	HAL_TIM_Base_Stop_IT(&htim15);
	HAL_TIM_Base_Stop_IT(&htim2);
//	HAL_SuspendTick();
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
    // 使能PWR时钟
    __HAL_RCC_PWR_CLK_ENABLE();
    // 清除唤醒标记
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWREx_EnableLowPowerRunMode();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
	HAL_Init();
	SystemClock_Config();
//	hadc1.State = HAL_ADC_STATE_RESET;
//	MX_ADC1_Init();
	HAL_ADC_MspInit(&hadc1);
	Key_WaitForNext();
	LCD_DisplayOn();
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim7);
	HAL_TIM_Base_Start_IT(&htim15);
	Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
	MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);

}

Key_EventTypeDef Key_GetEvent(void)
{

	if (key_event == KEY_DOWN) {
		key_event = KEY_PRESS;
		return KEY_DOWN;
	}

	if (key_event == KEY_UP) {
		key_event = KEY_NOEVENT;
		return KEY_UP;
	}

	return key_event;
}

void Key_ClearEvent(void)
{
	key_event = KEY_NOEVENT;
}

void Key_WaitForNext(void)
{
	key_event = KEY_WFN;
}

void Key_Lock(void)
{
	key_event = KEY_LOCK;
}

void Key_UnLock(void)
{
	key_event = KEY_NOEVENT;
}

Stick_StateTypeDef Stick_GetState(void)
{
	return stick_state;
}

Stick_EventTypeDef Stick_GetEvent(void)
{
	Stick_EventTypeDef ret_event = stick_event;

	stick_event = STICK_NOEVENT;
	return ret_event;
}

void Stick_ClearEvent(void)
{
	stick_event = STICK_NOEVENT;
}

void Get_Coordinate(void)
{

	HAL_ADC_Start(&hadc1);     //启动ADC转换
	HAL_ADC_PollForConversion(&hadc1, 20);   //等待转换完成
 	x = HAL_ADC_GetValue(&hadc1); //获取

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 20);
 	y = HAL_ADC_GetValue(&hadc1);

	HAL_ADC_Stop(&hadc1);
}

void Square2Disc(int Square_x, int Square_y, float *Disc_x, float *Disc_y)
{
	float x1, y1, x2, y2;

	x1 = ((float)Square_x / x_mid) - 1.0f;
	y1 = ((float)Square_y / y_mid) - 1.0f;

	x2 = x1 * sqrt(1 - (pow(y1, 2) / 2));
	y2 = y1 * sqrt(1 - (pow(x1, 2) / 2));

	*Disc_x = x2;
	*Disc_y = y2;
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

	/* 传感器信息屏�??? */
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

	label3 = lv_label_create(scr1);
    lv_label_set_recolor(label3, true);
    lv_label_set_long_mode(label3, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label3, 240);
    lv_obj_set_height(label3, 20);
    lv_label_set_text_fmt(label3, "#000000 x: %d#", x);
    lv_obj_align(label3, LV_ALIGN_TOP_LEFT, 0, 40);

	label4 = lv_label_create(scr1);
    lv_label_set_recolor(label4, true);
    lv_label_set_long_mode(label4, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label4, 240);
    lv_obj_set_height(label4, 20);
    lv_label_set_text_fmt(label4, "#000000 y: %d#", y);
    lv_obj_align(label4, LV_ALIGN_TOP_LEFT, 0, 60);

	label5 = lv_label_create(scr1);
    lv_label_set_recolor(label5, true);
    lv_label_set_long_mode(label5, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label5, 240);
    lv_obj_set_height(label5, 20);
    lv_label_set_text_fmt(label5, "#000000 disc_x: %f#", disc_x);
    lv_obj_align(label5, LV_ALIGN_TOP_LEFT, 0, 80);

	label6 = lv_label_create(scr1);
    lv_label_set_recolor(label6, true);
    lv_label_set_long_mode(label6, LV_LABEL_LONG_CLIP); /*Circular scroll*/
    lv_obj_set_width(label6, 240);
    lv_obj_set_height(label6, 20);
    lv_label_set_text_fmt(label6, "#000000 disc_y: %f#", disc_y);
    lv_obj_align(label6, LV_ALIGN_TOP_LEFT, 0, 100);

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
    lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: TX#");
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

//void lv_scr_setting_init(void)
//{
//	for (int i=0; i<3; i++) {
//		scr_setting_lables[i] = lv_label_create(scr_setting);
//	    lv_label_set_recolor(scr_setting_lables[i], true);
//	    lv_label_set_long_mode(scr_setting_lables[i], LV_LABEL_LONG_CLIP); /*Circular scroll*/
//	    lv_obj_set_width(scr_setting_lables[i], 240);
//	    lv_obj_set_height(scr_setting_lables[i], 20);
//	    lv_obj_align(scr_setting_lables[i], LV_ALIGN_TOP_MID, 0, i*20);
//	}
//
//    lv_label_set_text_fmt(scr_setting_lables[0], "#ffffff Sleep#");
//    lv_label_set_text_fmt(scr_setting_lables[1], "#ffffff Mode: TX#");
//    lv_label_set_text_fmt(scr_setting_lables[2], "#ffffff Start!#");
//}

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
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET)
		{
			if (key_event == KEY_NOEVENT) {
				key_event = KEY_DOWN;
			}
			else if (key_event == KEY_PRESS) {
				press_cnt++;
			}

			LCD_DisplayOn();
			Screen_Off_Timeout_End = Get_SysTime() + Screen_Off_Timeout;
			MCU_PowerOff_Timeout_End = Get_SysTime() + MCU_PowerOff_Timeout;
		}
		else
		{
			if (key_event != KEY_NOEVENT)
			{
				if (key_event == KEY_LOCK)
					key_event = KEY_LOCK;
				else if (key_event == KEY_WFN)
					key_event = KEY_NOEVENT;
				else
					key_event = KEY_UP;

				press_cnt = 0;
			}

		}

		Get_Coordinate();
		if ((fabs(x-x_mid) < 200) && (fabs(y-y_mid) < 200)) {
			stick_state = STICK_STATE_CENTER;
		}
		else if ((fabs(x-x_mid) < 200) && (fabs(y-y_max) < 200)) {
			if (stick_state == STICK_STATE_CENTER)
				stick_event = STICK_UP;

			stick_state = STICK_STATE_90;
		}
		else if ((fabs(x-x_mid) < 200) && (fabs(y-y_min) < 200)) {
			if (stick_state == STICK_STATE_CENTER)
				stick_event = STICK_DOWN;

			stick_state = STICK_STATE_270;
		}
		else if ((fabs(x-x_max) < 200) && (fabs(y-y_mid) < 200)) {
			if (stick_state == STICK_STATE_CENTER)
				stick_event = STICK_RIGHT;

			stick_state = STICK_STATE_0;
		}
		else if ((fabs(x-x_min) < 200) && (fabs(y-y_mid) < 200)) {
			if (stick_state == STICK_STATE_CENTER)
				stick_event = STICK_LEFT;

			stick_state = STICK_STATE_180;
		}
		Square2Disc(x, y, &disc_x, &disc_y);
		move_flag = 1;
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
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_TIM15_Init();
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

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);

  for (int i=0; i<LORATEST_PKTSIZE; i++) {
	  LoRaTest_TxData[i] = i;
  }

  W25QXX_Init();
  W25QXX_Read(&flash1, (uint8_t *)&click_num, 0, 4);

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
				  W25QXX_Erase_Sector(&flash1, 0);
				  W25QXX_WritePage(&flash1, (uint8_t *)&click_num, 0, 4);
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

						  lv_label_set_text_fmt(scr2_lables[3], "#ff0000 Transmitting Test Data...#");
						  lv_label_set_text_fmt(scr2_lables[4], "#ffffff Send Pkt Num: %d#", LoRaTest_SendPktNum);
						  xl1278_TxPacket(LoRa0.SPI_Inst, LoRaTest_TxData, LORATEST_PKTSIZE);
						  LoRa0.State = DEVICE_STATE_BUSY_TX;
					  }
					  else {
						  LoRaTest_RecvPktNum = 0;

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

	  /* 摇杆事件 */
	  switch (Stick_GetEvent()) {
  	  	  case STICK_NOEVENT: {
  	  		  break;
  	  	  }

  	  	  case STICK_UP: {
  	  		  if (scr_sel_lab == 0) {
  	  			  scr_sel_lab = 2;
  	  		  }
  	  		  else {
  	  			  scr_sel_lab--;
  	  		  }
  	  		  scr_sel_lable(scr_sel_lab);
  	  		  break;
  	  	  }

  	  	  case STICK_DOWN: {
			  lv_obj_t * scr_act = lv_scr_act();

			  if (scr_act == scr2) {
				  scr_sel_lab = (scr_sel_lab+1) % 3;
				  scr_sel_lable(scr_sel_lab);
			  }
  	  		  break;
  	  	  }

  	  	  case STICK_LEFT: {
  	  		  break;
  	  	  }

  	  	  case STICK_RIGHT: {
			  lv_obj_t * scr_act = lv_scr_act();

			  if (scr_act == scr2) {
				  if (LoRaTest_State == LoRaTest_STATE_IDLE) {
					  if (LoRaTest_Mode == LORATEST_MODE_TX) {
						  lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: RX#");
						  LoRaTest_Mode = LORATEST_MODE_RX;
					  }
					  else {
						  lv_label_set_text_fmt(scr2_lables[1], "#ffffff Mode: TX#");
						  LoRaTest_Mode = LORATEST_MODE_TX;
					  }
				  }

			  }
  	  		  break;
  	  	  }

	  	  default: {
	  		  break;
	  	  }
	  }

	  if (move_flag) {
		  move_flag = 0;
		  lv_obj_t * scr_act = lv_scr_act();
		  if (scr_act == scr1) {
			  lv_label_set_text_fmt(label3, "#000000 x: %d#", x);
			  lv_label_set_text_fmt(label4, "#000000 y: %d#", y);
			  lv_label_set_text_fmt(label5, "#000000 disc_x: %d#", (int)(disc_x * 100));
			  lv_label_set_text_fmt(label6, "#000000 disc_y: %d#", (int)(disc_y * 100));

			  line_points[1].x = disc_x * 100 + 120;
			  line_points[1].y = disc_y * -100 + 200;
			  lv_line_set_points(line0, line_points, sizeof(line_points) / sizeof(lv_point_t));
		  }

	  }
	  else {
		  //System_EnterSleep();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
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
