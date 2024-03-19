
#include "main.h"
#include <stdio.h>
#include "usart.h"
//#include "gpio.h"

typedef void (*XL1278Cluster_Callback)(uint32_t c_id);

typedef struct
{
	uint8_t 	OpMode;                /*!< Specifies  */

	uint32_t 	Freq;           			/*!< Specifies  */

	uint8_t 	SpreadFactor;            /*!< Specifies  */

	uint8_t 	Bandwidth;         /*!< Specifies */

	uint8_t 	CodingRate;            /*!< Specifies */

	uint8_t 	DeviceMode;                 /*!< Specifies */

	uint8_t 	HeaderMode;   			/*!< Specifies */

	uint8_t 	RxPayloadCrcOn;            /*!< Specifies */

	uint16_t 	Preamble;              /*!< Specifies */

	uint8_t 	Crc;      /*!< Specifies */

	uint8_t 	PaConfig;       /*!< Specifies */
	
	uint8_t   	OcpConfig;
	
	uint8_t		Lna;
	
	uint8_t 	ModemConfig1;
	
	uint8_t 	ModemConfig2;
	
	uint8_t 	ModemConfig3;

	uint8_t		SymbTimeoutLsb;
	
	uint8_t		DioMapping1;
	
	uint8_t		DioMapping2;
	


} XL1278_InitTypeDef;


typedef enum
{
	DEVICE_STATE_IDLE    	= 0x00U,
	DEVICE_STATE_BUSY,
	DEVICE_STATE_BUSY_TX,
	DEVICE_STATE_BUSY_RX,
	DEVICE_STATE_WAIT,
	DEVICE_NOTFOUND
} Device_StateTypeDef;

typedef struct
{
	volatile Device_StateTypeDef 	State;
	
	XL1278_InitTypeDef				Init;

	SPI_HandleTypeDef* 				SPI_Inst;	/* HAL库SPI实例指针 */

	void (* TxCpltCallback)(void);
	void (* RxCpltCallback)(void);
	
} XL1278_Device_PCB;


//typedef struct
//{
//	SPI_HandleTypeDef* 	SPI_Inst;	/* HAL库SPI实例指针 */
//
//	uint32_t			Active_Device;	/* 使能的设备*/
//
//	XL1278_Device_PCB 	Device_PCB[8]; /* 设备进程块*/
//
//	XL1278Cluster_Callback	Callback; /* 回调函数指针 */
//
//} XL1278_Cluster_PCB;


typedef enum
{
	DEVICE_NOEVENT	    	= 0x00U,
	DEVICE_RXDONE,
	DEVICE_TXDONE,
	DEVICE_ERROR,
	DEVICE_WAKEUP,
	DEVICE_SETFREQ
} EventTypeDef;

//typedef struct
//{
//	XL1278_Cluster_PCB	 	*pCluster_PCB;
//
//	uint32_t				DeviceID;
//
//	EventTypeDef			Event;
//
//} XL1278_RxEvent;


//typedef struct
//{
//	uint32_t 				Tx_DeviceID;
//
//	uint32_t 				Rx_DeviceID;
//
//	XL1278_Cluster_PCB	 	*pTxCluster_PCB;
//
//	XL1278_Cluster_PCB	 	*pRxCluster_PCB;
//
//	EventTypeDef			Event;
//
//	int						Flag;
//
//	uint32_t 				Freq;
//
//	uint32_t				wait_time;
//
//	void 					*Info;
//
//} XL1278_Event;
//
//
//
//
//
//
//
//extern XL1278_Cluster_PCB	RxCluster0;
//extern XL1278_Cluster_PCB	RxCluster1;
//extern XL1278_Cluster_PCB	TxCluster;

extern XL1278_Device_PCB LoRa0;

#define FREQ_32M

#ifdef FREQ_32M
#define FREQ_STEP 61.035f
#else
#define FREQ_STEP 49.591f
#endif

/* state */
#define DEVICE_OFF  	0x00
#define DEVICE_ON   	0x00


#define xl1278_OK		0x00
#define xl1278_ERROR	0x01
#define xl1278_BUSY		0x02

/** @defgroup XL1278-smt RegAddr
  * @{
  */
#define xl1278_RegFifo						0x00
#define xl1278_RegOpMode					0x01
#define xl1278_RegFrMsb						0x06
#define xl1278_RegFrMid						0x07
#define xl1278_RegFrLsb						0x08
#define xl1278_RegPaConfig					0x09
#define xl1278_RegPaRamp					0x0A
#define xl1278_RegOcp						0x0B
#define xl1278_RegLna						0x0C
#define xl1278_RegFifoAddrPtr				0x0D
#define xl1278_RegFifoTxBaseAddr			0x0E
#define xl1278_RegFifoRxBaseAddr			0x0F
#define xl1278_RegFifoRxCurrentAddr			0x10
#define xl1278_RegIrqFlagsMask				0x11
#define xl1278_RegIrqFlags					0x12
#define xl1278_RegRxNbBytes					0x13
#define xl1278_RegRxHeaderCntValueMsb		0x14
#define xl1278_RegRxHeaderCntValueLsb		0x15
#define xl1278_RegRxPacketCntValueMsb		0x16
#define xl1278_RegRxPacketCntValueLsb		0x17
#define xl1278_RegModemStat					0x18
#define xl1278_RegPktSnrValue				0x19
#define xl1278_RegPktRssiValue				0x1A
#define xl1278_RegRssiValue					0x1B
#define xl1278_RegHopChannel				0x1C
#define xl1278_RegModemConfig1				0x1D
#define xl1278_RegModemConfig2				0x1E
#define xl1278_RegSymbTimeoutLsb			0x1F
#define xl1278_RegPreambleMsb				0x20
#define xl1278_RegPreambleLsb				0x21
#define xl1278_RegPayloadLength				0x22
#define xl1278_RegMaxPayloadLength			0x23
#define xl1278_RegHopPeriod					0x24
#define xl1278_RegFifoRxByteAddr			0x25
#define xl1278_RegModemConfig3				0x26
// I/O settings
#define xl1278_RegDIOMAPPING1               0x40
#define xl1278_RegDIOMAPPING2               0x41
// Version
#define xl1278_RegVERSION                   0x42
// Additional settings
#define xl1278_RegPLLHOP                    0x44
#define xl1278_RegTCXO                      0x4B
#define xl1278_RegPADAC                     0x4D
#define xl1278_RegFORMERTEMP                0x5B

#define xl1278_RegAGCREF                    0x61
#define xl1278_RegAGCTHRESH1                0x62
#define xl1278_RegAGCTHRESH2                0x63
#define xl1278_RegAGCTHRESH3                0x64


#define	xl1278_FifoBaseAddr				    0x00
#define	xl1278_TxFifoBaseAddr			    xl1278_FifoBaseAddr
#define	xl1278_RxFifoBaseAddr				xl1278_FifoBaseAddr



/**
  * @}
  */
	
/* Peripheral Definitions for Operation */
#define	xl1278_WriteMask	0x80
#define xl1278_ReadMask  	0x7f


/*0x01 RegOpMode Operation */
#define xl1278_LoRaMode						0x80
#define xl1278_FSKMode						0x00

#define xl1278_AccessSharedReg_LoRa		    0x00
#define xl1278_AccessSharedReg_FSK		    0x40

#define xl1278_LowFreqMode_High				0x00
#define xl1278_LowFreqMode_Low				0x08

#define xl1278_Mode_Sleep					0x00
#define xl1278_Mode_Standby					0x01
#define xl1278_Mode_FSTx					0x02
#define xl1278_Mode_TX						0x03
#define xl1278_Mode_FSRx					0x04
#define xl1278_Mode_RXContinuous			0x05
#define xl1278_Mode_RXSingle				0x06
#define xl1278_Mode_CAD						0x07


/*0x09 RegPaConfig Operation */
#define xl1278_PaSelect_RFO					0x00
#define xl1278_PaSelect_BOOST				0x80

#define xl1278_PaOutputPower_20dbm 		    0xFF
#define xl1278_PaOutputPower_17dbm 		    0xFC
#define xl1278_PaOutputPower_14dbm 		    0xF9
#define xl1278_PaOutputPower_11dbm 		    0xF6


/*0x0A RegPaRamp Operation */
#define xl1278_PaRamp_3ms			        0x00
#define xl1278_PaRamp_2ms			        0x01
#define xl1278_PaRamp_1ms			        0x02
#define xl1278_PaRamp_500us		            0x03
#define xl1278_PaRamp_250us		            0x04
#define xl1278_PaRamp_125us		            0x05
#define xl1278_PaRamp_100us		            0x06
#define xl1278_PaRamp_62us		            0x07
#define xl1278_PaRamp_50us		            0x08
#define xl1278_PaRamp_40us		            0x09
#define xl1278_PaRamp_31us		            0x0A
#define xl1278_PaRamp_25us		            0x0B
#define xl1278_PaRamp_20us		            0x0C
#define xl1278_PaRamp_15us		            0x0D
#define xl1278_PaRamp_12us		            0x0E
#define xl1278_PaRamp_10us		            0x0F


/*0x0B RegOcp Operation */
#define xl1278_Ocp_Disable		            0x00
#define xl1278_Ocp_Enable			        0x10

#define xl1278_OcpTrim_Default		        0x0B


/*0x0C RegLna Operation */
#define xl1278_LnaGain_Disable		        0x00
#define xl1278_LnaGain_G1					0x20
#define xl1278_LnaGain_G2					0x40
#define xl1278_LnaGain_G3					0x60
#define xl1278_LnaGain_G4					0x80
#define xl1278_LnaGain_G5					0xA0
#define xl1278_LnaGain_G6					0xC0

#define xl1278_LnaBoosHf_Disable	        0x00
#define xl1278_LnaBoosHf_Enable		        0x03



/*0x11 RegIrqFlagMask Operation */
#define xl1278_IrqFlag_ALLDisable			    0xFF
#define xl1278_IrqFlag_RxTimeoutMask		    0x80
#define xl1278_IrqFlag_RxDoneMask				0x40
#define xl1278_IrqFlag_PayloadCrcErrorMask	    0x20
#define xl1278_IrqFlag_ValidHeaderMask		    0x10
#define xl1278_IrqFlag_TxDoneMask				0x08
#define xl1278_IrqFlag_CadDoneMask				0x04
#define xl1278_IrqFlag_DhssChangeChannelMask	0x02
#define xl1278_IrqFlag_CadDetectedMask			0x01


/*0x12 RegIrqFlag Operation */
#define xl1278_IrqFlag_ALLClear					0xFF
#define xl1278_IrqFlag_RxTimeout				0x80
#define xl1278_IrqFlag_RxDone					0x40
#define xl1278_IrqFlag_PayloadCrcError		    0x20
#define xl1278_IrqFlag_ValidHeader				0x10
#define xl1278_IrqFlag_TxDone					0x08
#define xl1278_IrqFlag_CadDone					0x04
#define xl1278_IrqFlag_DhssChangeChannel	    0x02
#define xl1278_IrqFlag_CadDetected				0x01


/*0x1D RegModemConfig1 Operation */
#define xl1278_Bandwidth_8k								0x00
#define xl1278_Bandwidth_10k							0x10
#define xl1278_Bandwidth_16k							0x20
#define xl1278_Bandwidth_21k							0x30
#define xl1278_Bandwidth_31k							0x40
#define xl1278_Bandwidth_42k							0x50
#define xl1278_Bandwidth_63k							0x60
#define xl1278_Bandwidth_125k							0x70
#define xl1278_Bandwidth_250k							0x80
#define xl1278_Bandwidth_500k							0x90

#define xl1278_CodingRate_4_5							0x02
#define xl1278_CodingRate_4_6							0x04
#define xl1278_CodingRate_4_7							0x06
#define xl1278_CodingRate_4_8							0x08

#define xl1278_ImplicitHeaderMode_Disable		 	    0x00
#define xl1278_ImplicitHeaderMode_Enable			    0x01

/*0x1E RegModemConfig2 Operation */
#define xl1278_SpreadFactor_64							0x60
#define xl1278_SpreadFactor_128							0x70
#define xl1278_SpreadFactor_256							0x80
#define xl1278_SpreadFactor_512							0x90
#define xl1278_SpreadFactor_1024						0xA0
#define xl1278_SpreadFactor_2048						0xB0
#define xl1278_SpreadFactor_4096						0xC0
#define xl1278_TxContinuousMode_Normal				    0x00
#define xl1278_TxContinuousMode_Continuous		        0x08
#define xl1278_RxPayloadCrcOn_Disable					0x00
#define xl1278_RxPayloadCrcOn_Enable					0x04
#define xl1278_SymbTimeoutMsb_Max						0x03


/*0x24 RegHopPeriod Operation */
#define xl1278_FreqHopping_Disable						0x00



/*0x26 RegModemConfig3 Operation */
#define xl1278_LowDataRate_Disable						0x00
#define xl1278_LowDataRate_Enable						0x08
#define xl1278_AgcAuto_Disable							0x00
#define xl1278_AgcAuto_Enable							0x04


/*0x40 RegDIOMAPPING1 Operation */
#define xl1278_DIOMAPPING_DIO0_RxDone					0x00
#define xl1278_DIOMAPPING_DIO0_TxDone					0x40
#define xl1278_DIOMAPPING_DIO0_CadDone					0x80
#define xl1278_DIOMAPPING_DIO0_Default					0x00

#define xl1278_DIOMAPPING_DIO1_RxTimeout				0x00
#define xl1278_DIOMAPPING_DIO1_FhssChangeChannel		0x10
#define xl1278_DIOMAPPING_DIO1_CadDetect				0x20
#define xl1278_DIOMAPPING_DIO1_Default					0x00

#define xl1278_DIOMAPPING_DIO2_FhssChangeChannel		0x00
#define xl1278_DIOMAPPING_DIO2_Default					0x00

#define xl1278_DIOMAPPING_DIO3_CadDone					0x00
#define xl1278_DIOMAPPING_DIO3_ValidHeader				0x01
#define xl1278_DIOMAPPING_DIO3_PayloadCrcError			0x02
#define xl1278_DIOMAPPING_DIO3_Default					0x00

/*0x41 RegDIOMAPPING2 Operation */
#define xl1278_DIOMAPPING_DIO4_CadDetect				0x00
#define xl1278_DIOMAPPING_DIO4_PLLLOCK					0x40
#define xl1278_DIOMAPPING_DIO4_Default					0x00

#define xl1278_DIOMAPPING_DIO5_ModeReady				0x00
#define xl1278_DIOMAPPING_DIO5_ClkOut					0x10
#define xl1278_DIOMAPPING_DIO5_Default					0x00

/*0x4D RegPADAC Operation */
#define xl1278_PADAC_Default							0x84
#define xl1278_PADAC_20dBm                          	0x87


#define xl1278_RESET_PORT						GPIOA
#define	xl1278_RESET_PIN		        		GPIO_PIN_3
#define	xl1278_NSS_PORT							GPIOA
#define	xl1278_NSS_PIN							GPIO_PIN_4
#define	xl1278_DIO0_PORT						GPIOA
#define	xl1278_DIO0_PIN							GPIO_PIN_5


///* ID */
//#define xl1278_DeviceID_0						((uint32_t)0x0000)
//#define xl1278_DeviceID_1						((uint32_t)0x0001)
//#define xl1278_DeviceID_2						((uint32_t)0x0002)
//#define xl1278_DeviceID_3						((uint32_t)0x0003)
//#define xl1278_DeviceID_4						((uint32_t)0x0004)
//#define xl1278_DeviceID_5						((uint32_t)0x0005)
//#define xl1278_DeviceID_6						((uint32_t)0x0006)
//#define xl1278_DeviceID_7						((uint32_t)0x0007)
//
///* GPIO */
//#define xl1278_RxCluster0_GPIO_PORT				GPIOD
//#define	xl1278_RxCluster0_RESETPIN		        GPIO_PIN_4
//#define	xl1278_RxCluster0_SEL_A0		        GPIO_PIN_0
//#define	xl1278_RxCluster0_SEL_A1		        GPIO_PIN_1
//#define	xl1278_RxCluster0_SEL_A2		        GPIO_PIN_2
//#define xl1278_RxCluster0_SEL_ENABLE			GPIO_PIN_3
//#define xl1278_RxCluster0_SEL_MASK				((uint32_t)0x0007)
//
//
//
//
//
//#define xl1278_RxCluster1_GPIO_PORT				GPIOD
//#define	xl1278_RxCluster1_RESETPIN		        GPIO_PIN_12
//#define	xl1278_RxCluster1_SEL_A0		        GPIO_PIN_8
//#define	xl1278_RxCluster1_SEL_A1		        GPIO_PIN_9
//#define	xl1278_RxCluster1_SEL_A2		        GPIO_PIN_10
//#define xl1278_RxCluster1_SEL_ENABLE			GPIO_PIN_11
//#define xl1278_RxCluster1_SEL_MASK				((uint32_t)0x0700)
//
//
//
//#define xl1278_TxCluster_GPIO_PORT				GPIOE
//#define	xl1278_TxCluster_RESETPIN		        GPIO_PIN_10
//#define	xl1278_TxCluster_Device_0_NSSPIN		GPIO_PIN_11
//#define	xl1278_TxCluster_Device_1_NSSPIN		GPIO_PIN_12
//#define	xl1278_TxCluster_Device_0_DIO0			GPIO_PIN_13
//#define	xl1278_TxCluster_Device_1_DIO0			GPIO_PIN_14
//#define	xl1278_TxCluster_SEL		        	GPIO_PIN_15
//
//
//
//#define RxDone_DIO_PORT							GPIOG
//#define xl1278_RxCluster0_Device_0_DIO0			GPIO_PIN_0
//#define xl1278_RxCluster0_Device_1_DIO0 		GPIO_PIN_1
//#define xl1278_RxCluster0_Device_2_DIO0			GPIO_PIN_2
//#define xl1278_RxCluster0_Device_3_DIO0			GPIO_PIN_3
//#define xl1278_RxCluster0_Device_4_DIO0			GPIO_PIN_4
//#define xl1278_RxCluster0_Device_5_DIO0			GPIO_PIN_5
//#define xl1278_RxCluster0_Device_6_DIO0			GPIO_PIN_6
//#define xl1278_RxCluster0_Device_7_DIO0			GPIO_PIN_7
//#define xl1278_RxCluster1_Device_0_DIO0			GPIO_PIN_8
//#define xl1278_RxCluster1_Device_1_DIO0			GPIO_PIN_9
//#define xl1278_RxCluster1_Device_2_DIO0			GPIO_PIN_10
//#define xl1278_RxCluster1_Device_3_DIO0			GPIO_PIN_11
//#define xl1278_RxCluster1_Device_4_DIO0			GPIO_PIN_12
//#define xl1278_RxCluster1_Device_5_DIO0			GPIO_PIN_13
//#define xl1278_RxCluster1_Device_6_DIO0			GPIO_PIN_14
//#define xl1278_RxCluster1_Device_7_DIO0			GPIO_PIN_15
//
//#define TxDone_DIO_PORT							GPIOE










/* I/O operation functions  ***************************************************/

/* 这是直接对spi接口进行操作的 */
int xl1278_RegWrite(SPI_HandleTypeDef *hspi, uint8_t addr, uint8_t data);
int xl1278_RegRead(SPI_HandleTypeDef *hspi, uint8_t addr, uint8_t *pdata);
int xl1278_FifoWrite(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t size);
int xl1278_FifoRead(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t size);
int xl1278_SetSleep(SPI_HandleTypeDef *hspi);
int xl1278_SetStandby(SPI_HandleTypeDef *hspi);
int xl1278_SetOpMode(SPI_HandleTypeDef *hspi, uint8_t mode);
int xl1278_SetFreq(SPI_HandleTypeDef *hspi, uint32_t freq_set);
int xl1278_SetFifoAddrPtr(SPI_HandleTypeDef *hspi, uint8_t fifoptr);
int xl1278_SetFifoTxBaseAddr(SPI_HandleTypeDef *hspi, uint8_t BaseAddr);
int xl1278_SetFifoRxBaseAddr(SPI_HandleTypeDef *hspi, uint8_t BaseAddr);
int xl1278_SetPreamble(SPI_HandleTypeDef *hspi, uint16_t PreambleLength);
int xl1278_ClearIrq(SPI_HandleTypeDef *hspi);
int xl1278_GetRxNbBytes(SPI_HandleTypeDef *hspi, uint8_t *RxNbBytes);
int xl1278_GetSNR(SPI_HandleTypeDef *hspi, int *Snr);
int xl1278_GetRSSI(SPI_HandleTypeDef *hspi, int *Rssi);
int xl1278_SetRxSglConfig(SPI_HandleTypeDef *hspi);
int xl1278_SetRxContConfig(SPI_HandleTypeDef *hspi);
int xl1278_SetTxConfig(SPI_HandleTypeDef *hspi);
int xl1278_RxPacket(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t *length);
int xl1278_TxPacket(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t length);
int xl1278_Init(SPI_HandleTypeDef *hspi, XL1278_InitTypeDef *Config);
void LoRa_Init(void);
void xl1278_DeviceCheak(XL1278_Device_PCB *cpcb);
//
///* 这是将几个模块整合成一个簇 */
//void xl1278_RxCluster0_Init(void);
//void xl1278_RxCluster1_Init(void);
//void xl1278_TxCluster_Init(void);
//void xl1278_RxCluster0_Reset(void);
//void xl1278_RxCluster1_Reset(void);
//void xl1278_TxCluster_Reset(void);
void xl1278_Channel_Enable(SPI_HandleTypeDef *hspi);
void xl1278_Channel_Disable(SPI_HandleTypeDef *hspi);
//void xl1278_RxCluster0_Channel_Disable(void);
//void xl1278_RxCluster1_Channel_Disable(void);
//void xl1278_TxCluster_Channel_Disable(void);
//int xl1278_ClusterRegWrite(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t addr, uint8_t data);
//int xl1278_ClusterRegRead(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t addr, uint8_t *pdata);
//int xl1278_ClusterSetSleep(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID);
//int xl1278_ClusterSetStandby(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID);
//int xl1278_ClusterSetFreq(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint32_t freq_set);
//int xl1278_ClusterGetFreq(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint32_t *freq_get);
//int xl1278_ClusterGetSNR(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, int *snr);
//int xl1278_ClusterClearIrq(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID);
//int xl1278_ClusterTxPacket(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t *pdata, uint16_t length);
//int xl1278_ClusterTxDone(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID);
//int xl1278_ClusterSetRxContConfig(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID);
//int xl1278_ClusterRxPacket(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t *pdata, uint16_t *length);
//XL1278_Event xl1278_TxDonePolling();
//XL1278_Event xl1278_RxDonePolling();
//void xl1278_ClusterInit(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID);
__weak void xl1278_TxCpltCallback(void);
__weak void xl1278_RxCpltCallback(void);



