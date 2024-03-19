#include "spi.h"
#include "xl1278smt_driver.h"

XL1278_Device_PCB LoRa0;









//delay@168MHz
//1000cycles->72us
//
/**
  * @brief  延时函数
  * @note   1000cycles->72us
  * @param  delay_cycle 延时所循环的次数
  */
void Delay_cycles(uint32_t delay_cycle)
{
	for (volatile uint32_t i=0; i<delay_cycle; i++);
}

/**
  * @brief  建立spi的片选通道
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  */
void xl1278_Channel_Enable(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(xl1278_NSS_PORT, xl1278_NSS_PIN, GPIO_PIN_RESET);
}



/**
  * @brief  关闭spi的片选通道
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  */
void xl1278_Channel_Disable(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(xl1278_NSS_PORT, xl1278_NSS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  向sx1278模块进行写寄存器操作
  * @note   一次只能写一个字节
  * @param  hspi 指向所使用spi的结构体指针
  * @param  addr 所写寄存器的地址
  * @param  data 要发送的数据
  */
int xl1278_RegWrite(SPI_HandleTypeDef *hspi, uint8_t addr, uint8_t data)
{
	int status = 0;
	uint8_t senddata[2];

	senddata[0] = addr | xl1278_WriteMask;
	senddata[1] = data;
	xl1278_Channel_Enable(hspi);
	status = HAL_SPI_Transmit(hspi, senddata, 2, 50);
	xl1278_Channel_Disable(hspi);
	Delay_cycles(1000);
	return status;
}



/**
  * @brief  向sx1278模块进行读寄存器操作
  * @note   一次只能读一个字节
  * @param  hspi 指向所使用spi的结构体指针
  * @param  addr 所读寄存器的地址
  * @param  data 要接收的数据的地址
  */
int xl1278_RegRead(SPI_HandleTypeDef *hspi, uint8_t addr, uint8_t *pdata)
{
	int status = 0;

	addr = addr & xl1278_ReadMask;
	xl1278_Channel_Enable(hspi);
	status |= HAL_SPI_Transmit(hspi, &addr, 1, 50);
	status |= HAL_SPI_Receive(hspi, pdata, 1, 50);
	xl1278_Channel_Disable(hspi);
	Delay_cycles(1000);
	return status;
}



/**
  * @brief  向sx1278模块的FIFO进行突发写操作
  * @note   最多写255字节
  * @param  hspi 指向所使用spi的结构体指针
  * @param  pdata 指向要发送数据的指针
  * @param  size 要写入的数据字节数
  */
int xl1278_FifoWrite(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t size)
{	
	int status = 0;
	uint8_t addr;

	addr = xl1278_RegFifo | xl1278_WriteMask;
	xl1278_Channel_Enable(hspi);
	status |= HAL_SPI_Transmit(hspi, &addr, 1, 50);
	status |= HAL_SPI_Transmit(hspi, pdata, size, 50);
	xl1278_Channel_Disable(hspi);
	Delay_cycles(1000);
	return status;
}



/**
  * @brief  向sx1278模块的FIFO进行突发读操作
  * @note   最多读255字节
  * @param  hspi 指向所使用spi的结构体指针
  * @param  pdata 指向接收数据的指针
  * @param  size 要读取的数据字节数
  */
int xl1278_FifoRead(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t size)
{
	int status = 0;
	uint8_t senddata = xl1278_RegFifo & xl1278_ReadMask;

	xl1278_Channel_Enable(hspi);
	status = HAL_SPI_Transmit(hspi, &senddata, 1, 50);
	if(status != HAL_OK)
		return status;
	status = HAL_SPI_Receive(hspi, pdata, size, 50);
	xl1278_Channel_Disable(hspi);
	Delay_cycles(1000);
	return status;
}



/**
  * @brief  将sx1278模块设置成睡眠模式
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  */
int xl1278_SetSleep(SPI_HandleTypeDef *hspi)
{
	int status = 0;
	uint8_t OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Sleep;

	status = xl1278_RegWrite(hspi, xl1278_RegOpMode, OpMode);
	return status;
}



/**
  * @brief  将sx1278模块设置成待机模式
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  */
int xl1278_SetStandby(SPI_HandleTypeDef *hspi)
{
	int status = 0;
	uint8_t OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Standby;

	status = xl1278_RegWrite(hspi, xl1278_RegOpMode, OpMode);
	return status;
}



/**
  * @brief  将sx1278模块设置成所需要的模式
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  mode 器件模式
  */
int xl1278_SetOpMode(SPI_HandleTypeDef *hspi, uint8_t mode)
{
	int status = 0;

	status =  xl1278_RegWrite(hspi, xl1278_RegOpMode, mode);
	return status;
}


/**
 *
 */
int xl1278_GetOpMode(SPI_HandleTypeDef *hspi, uint8_t *opmode)
{
	int status = 0;

	status =  xl1278_RegRead(hspi, xl1278_RegOpMode, opmode);
	*opmode &= 0x07;
	return status;
}

/**
  * @brief  设置sx1278的频率
  * @note   只有在器件处于睡眠或者待机模式时才能设置
  * @param  hspi 指向所使用spi的结构体指针
  * @param  freq_set 设置的频率
  */
int xl1278_SetFreq(SPI_HandleTypeDef *hspi, uint32_t freq_set)
{
	int status = 0;
	uint8_t senddata[3];
	uint32_t f_RF = (float)freq_set / FREQ_STEP;

	senddata[2]	= (f_RF & 0x00ff0000) >> 16;
	senddata[1]	= (f_RF & 0x0000ff00) >> 8;
	senddata[0]	= (f_RF & 0x000000ff) >> 0;
	status |= xl1278_RegWrite(hspi, xl1278_RegFrMsb, senddata[2]);
	status |= xl1278_RegWrite(hspi, xl1278_RegFrMid, senddata[1]);
	status |= xl1278_RegWrite(hspi, xl1278_RegFrLsb, senddata[0]);
	return status;
}

/**
  * @brief  读取sx1278的频率
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  freq_get 读到的频率
  */
int xl1278_GetFreq(SPI_HandleTypeDef *hspi, uint32_t *freq_get)
{
	int status = 0;
	uint8_t getdata[3];

	status |= xl1278_RegRead(hspi, xl1278_RegFrMsb, &getdata[2]);
	status |= xl1278_RegRead(hspi, xl1278_RegFrMid, &getdata[1]);
	status |= xl1278_RegRead(hspi, xl1278_RegFrLsb, &getdata[0]);
	*freq_get = (getdata[0] + ((float)getdata[1] * 256) + ((float)getdata[2] * 65536)) * FREQ_STEP;
	return status;
}

/**
  * @brief  设置sx1278的FIFO指针
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  fifoptr 指针地址
  */
int xl1278_SetFifoAddrPtr(SPI_HandleTypeDef *hspi, uint8_t fifoptr)
{
	int status = 0;

	status =  xl1278_RegWrite(hspi, xl1278_RegFifoAddrPtr, fifoptr);
	return status;
}



/**
  * @brief  设置sx1278的发送FIFO基地址
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  BaseAddr 发送FIFO基地址
  */
int xl1278_SetFifoTxBaseAddr(SPI_HandleTypeDef *hspi, uint8_t BaseAddr)
{
	int status = 0;

	status =  xl1278_RegWrite(hspi, xl1278_RegFifoTxBaseAddr, BaseAddr);
	return status;
}



/**
  * @brief  设置sx1278的接收FIFO基地址
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  BaseAddr 接收FIFO基地址
  */
int xl1278_SetFifoRxBaseAddr(SPI_HandleTypeDef *hspi, uint8_t BaseAddr)
{
	int status = 0;

	status =  xl1278_RegWrite(hspi, xl1278_RegFifoRxBaseAddr, BaseAddr);
	return status;
}



/**
  * @brief  设置sx1278的前导码长度
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  PreambleLength 前导码长度
  */
int xl1278_SetPreamble(SPI_HandleTypeDef *hspi, uint16_t PreambleLength)
{
	int status = 0;

	status |= xl1278_RegWrite(hspi, xl1278_RegPreambleMsb, (PreambleLength >> 8));
	status |= xl1278_RegWrite(hspi, xl1278_RegPreambleLsb, PreambleLength);
	return status;
}



/**
  * @brief  清除sx1278的中断标志
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  */
int xl1278_ClearIrq(SPI_HandleTypeDef *hspi)
{
	int status = 0;

	status =  xl1278_RegWrite(hspi, xl1278_RegIrqFlags, xl1278_IrqFlag_ALLClear);
	return status;
}



/**
  * @brief  读取sx1278本次接收数据包的字节数
  * @note   只有在器件处于睡眠或者待机模式时才能设置
  * @param  hspi 指向所使用spi的结构体指针
  * @param  RxNbBytes 接收数据包字节数所存放的变量地址
  */
int xl1278_GetRxNbBytes(SPI_HandleTypeDef *hspi, uint8_t *RxNbBytes)
{
	int status = 0;

	status =  xl1278_RegRead(hspi, xl1278_RegRxNbBytes, RxNbBytes);
	return status;
}

/**
 * 读取最后一个接收数据包的SNR
 * 读取的值以二进制补码格式乘以4
 */
int xl1278_GetSNR(SPI_HandleTypeDef *hspi, int *Snr)
{
	int status = 0;
	uint8_t getsnr;

	status =  xl1278_RegRead(hspi, xl1278_RegPktSnrValue, &getsnr);
	if (getsnr & 0x80)
	{
		getsnr = ~(getsnr - 0x01);
		*Snr = -(int)getsnr / 4;
	}
	else
	{
		*Snr = getsnr / 4;
	}
	return status;
}

/**
 * 读取最后一个接收数据包的RSSI
 *
 */
int xl1278_GetRSSI(SPI_HandleTypeDef *hspi, int *Rssi)
{
	int status = 0;
	uint8_t getrssi;

	status =  xl1278_RegRead(hspi, xl1278_RegPktRssiValue, &getrssi);
	*Rssi = -137 + (int)getrssi;
	return status;
}


/**
  * @brief  设置sx1278为单次接收模式
  * @note   只有在器件处于睡眠或者待机模式时才能设置
  * @param  hspi 指向所使用spi的结构体指针
  * @param  freq_set 设置的频率
  */
int xl1278_SetRxSglConfig(SPI_HandleTypeDef *hspi)
{
	int status = 0;
	uint8_t OpMode;

	status |= xl1278_RegWrite(hspi, xl1278_RegPADAC, xl1278_PADAC_Default);
	status |= xl1278_RegWrite(hspi, xl1278_RegHopPeriod, xl1278_FreqHopping_Disable);
	status |= xl1278_RegWrite(hspi, xl1278_RegIrqFlagsMask, ~xl1278_IrqFlag_RxDoneMask);//Enable RxDone interrupt
	status |= xl1278_ClearIrq(hspi);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoTxBaseAddr, xl1278_TxFifoBaseAddr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoRxBaseAddr, xl1278_RxFifoBaseAddr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoAddrPtr, xl1278_RxFifoBaseAddr);
	OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_RXSingle;
	status |= xl1278_RegWrite(hspi, xl1278_RegOpMode, OpMode);
	return status;
}



/**
  * @brief  设置sx1278的频率
  * @note   只有在器件处于睡眠或者待机模式时才能设置
  * @param  hspi 指向所使用spi的结构体指针
  * @param  freq_set 设置的频率
  */
int xl1278_SetRxContConfig(SPI_HandleTypeDef *hspi)
{
	int status = 0;
	uint8_t OpMode;

	status |= xl1278_RegWrite(hspi, xl1278_RegPADAC, xl1278_PADAC_Default);
	status |= xl1278_RegWrite(hspi, xl1278_RegHopPeriod, xl1278_FreqHopping_Disable);
	status |= xl1278_RegWrite(hspi, xl1278_RegIrqFlagsMask, ~xl1278_IrqFlag_RxDoneMask);//Enable RxDone interrupt
	status |= xl1278_ClearIrq(hspi);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoTxBaseAddr, xl1278_TxFifoBaseAddr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoRxBaseAddr, xl1278_RxFifoBaseAddr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoAddrPtr, xl1278_RxFifoBaseAddr);
	OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_RXContinuous;
	status |= xl1278_RegWrite(hspi, xl1278_RegOpMode, OpMode);
	return status;
}



/**
  * @brief  设置sx1278的频率
  * @note   只有在器件处于睡眠或者待机模式时才能设置
  * @param  hspi 指向所使用spi的结构体指针
  * @param  freq_set 设置的频率
  */
int xl1278_SetTxConfig(SPI_HandleTypeDef *hspi)
{
	int status = 0;
	uint8_t DioMapping1 = xl1278_DIOMAPPING_DIO0_TxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;

	status |= xl1278_RegWrite(hspi, xl1278_RegPADAC, xl1278_PADAC_20dBm);
	status |= xl1278_RegWrite(hspi, xl1278_RegHopPeriod, xl1278_FreqHopping_Disable);
	status |= xl1278_RegWrite(hspi, xl1278_RegIrqFlagsMask, ~xl1278_IrqFlag_TxDoneMask);//Enable TxDone interrupt
	status |= xl1278_RegWrite(hspi, xl1278_RegDIOMAPPING1, DioMapping1);
	status |= xl1278_ClearIrq(hspi);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoTxBaseAddr, xl1278_TxFifoBaseAddr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoRxBaseAddr, xl1278_RxFifoBaseAddr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoAddrPtr, xl1278_TxFifoBaseAddr);
	return status;
}



/**
  * @brief  令sx1278接收数据包
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  pdata 存放接收数据的地址
  * @param  length 接收数据包的字节数
  */
int xl1278_RxPacket(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t *length)
{
	int status = 0;
	uint8_t rxptr;
	uint8_t rxsize;

	status |= xl1278_RegRead(hspi, xl1278_RegFifoRxCurrentAddr, &rxptr);
	status |= xl1278_RegWrite(hspi, xl1278_RegFifoAddrPtr, rxptr);
	status |= xl1278_RegRead(hspi, xl1278_RegRxNbBytes, &rxsize);
	*length = rxsize;
	status |= xl1278_FifoRead(hspi, pdata, rxsize);
	status |= xl1278_ClearIrq(hspi);
	return status;
}



/**
  * @brief  令sx1278发送数据包
  * @note
  * @param  hspi 指向所使用spi的结构体指针
  * @param  pdata 发送数据的地址
  * @param  length 发送数据包的字节数
  */
int xl1278_TxPacket(SPI_HandleTypeDef *hspi, uint8_t *pdata, uint16_t length)
{
	int status = 0;
	uint8_t OpMode;

	status |= xl1278_SetTxConfig(hspi);
	status |= xl1278_SetStandby(hspi);
	status |= xl1278_RegWrite(hspi, xl1278_RegPayloadLength, length);
	status |= xl1278_FifoWrite(hspi, pdata, length);
	OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_TX;
	status |= xl1278_RegWrite(hspi, xl1278_RegOpMode, OpMode);
	return status;
}



/**
  * @brief  检测模块是否存在
  * @note
  * @param  cpcb 指向sx1278簇的结构体指针
  * @param  DeviceID 设备的ID
  */
void xl1278_DeviceCheak(XL1278_Device_PCB *cpcb)
{
	uint8_t OpMode;

	xl1278_RegRead(cpcb->SPI_Inst, xl1278_RegOpMode, &OpMode);
	if((OpMode&0x78) == 0x08)
		cpcb->State = DEVICE_STATE_IDLE;
	else
		cpcb->State = DEVICE_NOTFOUND;
}



void xl1278_Reset(void)
{
	HAL_GPIO_WritePin(xl1278_RESET_PORT, xl1278_RESET_PIN, GPIO_PIN_RESET);
	Delay_cycles(2000000);
	HAL_GPIO_WritePin(xl1278_RESET_PORT, xl1278_RESET_PIN, GPIO_PIN_SET);
	Delay_cycles(2000000);
}



/**
 *
 */
int xl1278_Init(SPI_HandleTypeDef *hspi, XL1278_InitTypeDef *Config)
{
	int status = 0;
	
	xl1278_SetSleep(hspi);
	Delay_cycles(2000000);
	status |=  xl1278_RegWrite(hspi, xl1278_RegOpMode, Config->OpMode);
	
	status |= xl1278_SetFreq(hspi, Config->Freq);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegPaConfig, Config->PaConfig);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegOcp, Config->OcpConfig);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegLna, Config->Lna);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegModemConfig1, Config->ModemConfig1);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegModemConfig2, Config->ModemConfig2);
	
	if ((Config->SpreadFactor >= xl1278_SpreadFactor_2048) && (Config->Bandwidth <= xl1278_Bandwidth_125k)) {
		status |= xl1278_RegWrite(hspi, xl1278_RegModemConfig3, Config->ModemConfig3);
	}

	status |= xl1278_RegWrite(hspi, xl1278_RegSymbTimeoutLsb, Config->SymbTimeoutLsb);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegPreambleMsb, (Config->Preamble >> 8));
	
	status |= xl1278_RegWrite(hspi, xl1278_RegPreambleLsb, Config->Preamble);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegDIOMAPPING1, Config->DioMapping1);
	
	status |= xl1278_RegWrite(hspi, xl1278_RegDIOMAPPING2, Config->DioMapping2);
	
	xl1278_SetStandby(hspi);
	Delay_cycles(2000000);
	return status;
}



void LoRa_Init(void)
{
	XL1278_InitTypeDef InitConfig = {0};
	LoRa0.SPI_Inst = &hspi1;
	LoRa0.TxCpltCallback = xl1278_TxCpltCallback;
	LoRa0.RxCpltCallback = xl1278_RxCpltCallback;

	InitConfig.OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Sleep;
	InitConfig.Freq = 434000000;
	InitConfig.PaConfig = xl1278_PaOutputPower_20dbm;
	InitConfig.OcpConfig = xl1278_Ocp_Disable | xl1278_OcpTrim_Default;
	InitConfig.Lna = xl1278_LnaGain_G1 | xl1278_LnaBoosHf_Enable;
	InitConfig.Bandwidth = xl1278_Bandwidth_125k;
	InitConfig.CodingRate = xl1278_CodingRate_4_5;
	InitConfig.HeaderMode = xl1278_ImplicitHeaderMode_Disable;
	InitConfig.ModemConfig1 = InitConfig.Bandwidth | InitConfig.CodingRate | InitConfig.HeaderMode;
	InitConfig.SpreadFactor = xl1278_SpreadFactor_4096;
	InitConfig.Crc = xl1278_RxPayloadCrcOn_Enable;
	InitConfig.ModemConfig2 = InitConfig.SpreadFactor | InitConfig.Crc | xl1278_SymbTimeoutMsb_Max;
	InitConfig.SymbTimeoutLsb = 0xFF;
	InitConfig.Preamble = 12;
	InitConfig.ModemConfig3 = xl1278_LowDataRate_Enable | xl1278_AgcAuto_Disable;
	InitConfig.DioMapping1 = xl1278_DIOMAPPING_DIO0_RxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;
	InitConfig.DioMapping2 = xl1278_DIOMAPPING_DIO4_Default | xl1278_DIOMAPPING_DIO5_Default;

	xl1278_Reset();

	LoRa0.Init = InitConfig;
	LoRa0.Init.Freq = 434000000;
	xl1278_DeviceCheak(&LoRa0);
	if (LoRa0.State == DEVICE_STATE_IDLE)
	{
		xl1278_Init(LoRa0.SPI_Inst, &LoRa0.Init);
	}
}
















//
//
///**
//  * @brief  初始化RxCluster0
//  * @note
//  */
//void xl1278_RxCluster0_Init(void)
//{
//	XL1278_InitTypeDef InitConfig = {0};
//	RxCluster0.SPI_Inst = &hspi1;
//
//	InitConfig.OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Sleep;
//	InitConfig.Freq = 434000000;
//	InitConfig.PaConfig = xl1278_PaOutputPower_20dbm;
//	InitConfig.OcpConfig = xl1278_Ocp_Disable | xl1278_OcpTrim_Default;
//	InitConfig.Lna = xl1278_LnaGain_G1 | xl1278_LnaBoosHf_Enable;
//	InitConfig.Bandwidth = xl1278_Bandwidth_500k;
//	InitConfig.CodingRate = xl1278_CodingRate_4_5;
//	InitConfig.HeaderMode = xl1278_ImplicitHeaderMode_Disable;
//	InitConfig.ModemConfig1 = InitConfig.Bandwidth | InitConfig.CodingRate | InitConfig.HeaderMode;
//	InitConfig.SpreadFactor = xl1278_SpreadFactor_4096;
//	InitConfig.Crc = xl1278_RxPayloadCrcOn_Enable;
//	InitConfig.ModemConfig2 = InitConfig.SpreadFactor | InitConfig.Crc | xl1278_SymbTimeoutMsb_Max;
//	InitConfig.SymbTimeoutLsb = 0xFF;
//	InitConfig.Preamble = 12;
//	InitConfig.DioMapping1 = xl1278_DIOMAPPING_DIO0_RxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;
//	InitConfig.DioMapping2 = xl1278_DIOMAPPING_DIO4_Default | xl1278_DIOMAPPING_DIO5_Default;
//
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_0].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_0].Init.Freq = 434000000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_0);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_0].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_0].Init) != HAL_OK)
//		{
//			printf("RX0 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX0 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_1].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_1].Init.Freq = 433000000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_1);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_1].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_1].Init) != HAL_OK)
//		{
//			printf("RX1 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX1 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_2].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_2].Init.Freq = 432000000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_2);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_2].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_2].Init) != HAL_OK)
//		{
//			printf("RX2 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX2 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_3].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_3].Init.Freq = 431000000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_3);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_3].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_3].Init) != HAL_OK)
//		{
//			printf("RX3 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX3 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_4].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_4].Init.Freq = 429000000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_4);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_4].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_4].Init) != HAL_OK)
//		{
//			printf("RX4 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX4 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_5].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_5].Init.Freq = 429000000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_5);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_5].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_5].Init) != HAL_OK)
//		{
//			printf("RX5 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX5 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_6].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_6].Init.Freq = 488800000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_6);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_6].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_6].Init) != HAL_OK)
//		{
//			printf("RX6 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX6 Not Found!\r\n");
//	}
//
//	RxCluster0.Device_PCB[xl1278_DeviceID_7].Init = InitConfig;
//	RxCluster0.Device_PCB[xl1278_DeviceID_7].Init.Freq = 498800000;
//	xl1278_DeviceCheak(&RxCluster0, xl1278_DeviceID_7);
//	if (RxCluster0.Device_PCB[xl1278_DeviceID_7].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster0.SPI_Inst, &RxCluster0.Device_PCB[xl1278_DeviceID_7].Init) != HAL_OK)
//		{
//			printf("RX7 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX7 Not Found!\r\n");
//	}
//
//}
//
//
//
///**
//  * @brief  初始化RxCluster1
//  * @note
//  */
//void xl1278_RxCluster1_Init(void)
//{
//	XL1278_InitTypeDef InitConfig = {0};
//	RxCluster1.SPI_Inst = &hspi2;
//
//	InitConfig.OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Sleep;
//	InitConfig.Freq = 434000000;
//	InitConfig.PaConfig = xl1278_PaOutputPower_20dbm;
//	InitConfig.OcpConfig = xl1278_Ocp_Disable | xl1278_OcpTrim_Default;
//	InitConfig.Lna = xl1278_LnaGain_G1 | xl1278_LnaBoosHf_Enable;
//	InitConfig.Bandwidth = xl1278_Bandwidth_500k;
//	InitConfig.CodingRate = xl1278_CodingRate_4_5;
//	InitConfig.HeaderMode = xl1278_ImplicitHeaderMode_Disable;
//	InitConfig.ModemConfig1 = InitConfig.Bandwidth | InitConfig.CodingRate | InitConfig.HeaderMode;
//	InitConfig.SpreadFactor = xl1278_SpreadFactor_4096;
//	InitConfig.Crc = xl1278_RxPayloadCrcOn_Enable;
//	InitConfig.ModemConfig2 = InitConfig.SpreadFactor | InitConfig.Crc | xl1278_SymbTimeoutMsb_Max;
//	InitConfig.SymbTimeoutLsb = 0xFF;
//	InitConfig.Preamble = 12;
//	InitConfig.DioMapping1 = xl1278_DIOMAPPING_DIO0_RxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;
//	InitConfig.DioMapping2 = xl1278_DIOMAPPING_DIO4_Default | xl1278_DIOMAPPING_DIO5_Default;
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_0].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_0].Init.Freq = 434000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_0);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_0].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_0].Init) != HAL_OK)
//		{
//			printf("RX8 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("RX8 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_1].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_1].Init.Freq = 436000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_1);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_1].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_1].Init) != HAL_OK)
//			printf("RX9 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX9 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_2].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_2].Init.Freq = 437000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_2);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_2].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_2].Init) != HAL_OK)
//			printf("RX10 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX10 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_3].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_3].Init.Freq = 438000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_3);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_3].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_3].Init) != HAL_OK)
//			printf("RX11 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX11 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_4].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_4].Init.Freq = 439000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_4);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_4].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_4].Init) != HAL_OK)
//			printf("RX12 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX12 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_5].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_5].Init.Freq = 440000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_5);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_5].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_5].Init) != HAL_OK)
//			printf("RX13 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX13 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_6].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_6].Init.Freq = 441000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_6);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_6].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_6].Init) != HAL_OK)
//			printf("RX14 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX14 Not Found!\r\n");
//	}
//
//	RxCluster1.Device_PCB[xl1278_DeviceID_7].Init = InitConfig;
//	RxCluster1.Device_PCB[xl1278_DeviceID_7].Init.Freq = 442000000;
//	xl1278_DeviceCheak(&RxCluster1, xl1278_DeviceID_7);
//	if (RxCluster1.Device_PCB[xl1278_DeviceID_7].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(RxCluster1.SPI_Inst, &RxCluster1.Device_PCB[xl1278_DeviceID_7].Init) != HAL_OK)
//			printf("RX15 Init Failed!\r\n");
//	}
//	else
//	{
//		printf("RX15 Not Found!\r\n");
//	}
//
//
//}
//
//
//
//
///**
//  * @brief  初始化TxCluster
//  * @note
//  */
//void xl1278_TxCluster_Init(void)
//{
//	XL1278_InitTypeDef InitConfig = {0};
//	TxCluster.SPI_Inst = &hspi3;
//
//	InitConfig.OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Sleep;
//	InitConfig.Freq = 410000000;
//	InitConfig.PaConfig = xl1278_PaOutputPower_20dbm;
//	InitConfig.OcpConfig = xl1278_Ocp_Disable | xl1278_OcpTrim_Default;
//	InitConfig.Lna = xl1278_LnaGain_G1 | xl1278_LnaBoosHf_Enable;
//	InitConfig.Bandwidth = xl1278_Bandwidth_500k;
//	InitConfig.CodingRate = xl1278_CodingRate_4_5;
//	InitConfig.HeaderMode = xl1278_ImplicitHeaderMode_Disable;
//	InitConfig.ModemConfig1 = InitConfig.Bandwidth | InitConfig.CodingRate | InitConfig.HeaderMode;
//	InitConfig.SpreadFactor = xl1278_SpreadFactor_4096;
//	InitConfig.Crc = xl1278_RxPayloadCrcOn_Enable;
//	InitConfig.ModemConfig2 = InitConfig.SpreadFactor | InitConfig.Crc | xl1278_SymbTimeoutMsb_Max;
//	InitConfig.SymbTimeoutLsb = 0xFF;
//	InitConfig.Preamble = 12;
//	InitConfig.DioMapping1 = xl1278_DIOMAPPING_DIO0_TxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;
//	InitConfig.DioMapping2 = xl1278_DIOMAPPING_DIO4_Default | xl1278_DIOMAPPING_DIO5_Default;
//
//	TxCluster.Device_PCB[xl1278_DeviceID_0].Init = InitConfig;
//	TxCluster.Device_PCB[xl1278_DeviceID_0].Init.Freq = 410000000;
//	xl1278_DeviceCheak(&TxCluster, xl1278_DeviceID_0);
//	if (TxCluster.Device_PCB[xl1278_DeviceID_0].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(TxCluster.SPI_Inst, &TxCluster.Device_PCB[xl1278_DeviceID_0].Init) != HAL_OK)
//		{
//			printf("TxCluster DeviceID_0 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("TxCluster DeviceID_0 Not Found!\r\n");
//	}
//
//	TxCluster.Device_PCB[xl1278_DeviceID_1].Init = InitConfig;
//	TxCluster.Device_PCB[xl1278_DeviceID_1].Init.Freq = 410000000;
//	xl1278_DeviceCheak(&TxCluster, xl1278_DeviceID_1);
//	if (TxCluster.Device_PCB[xl1278_DeviceID_1].State == DEVICE_STATE_IDLE)
//	{
//		if (xl1278_Init(TxCluster.SPI_Inst, &TxCluster.Device_PCB[xl1278_DeviceID_1].Init) != HAL_OK)
//		{
//			printf("TxCluster DeviceID_1 Init Failed!\r\n");
//		}
//	}
//	else
//	{
//		printf("TxCluster DeviceID_1 Not Found!\r\n");
//	}
//
//}
//
//
///**
//  * @brief  复位RxCluster0
//  * @note
//  */
//void xl1278_RxCluster0_Reset(void)
//{
//	HAL_GPIO_WritePin(xl1278_RxCluster0_GPIO_PORT, xl1278_RxCluster0_RESETPIN, GPIO_PIN_RESET);
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(xl1278_RxCluster0_GPIO_PORT, xl1278_RxCluster0_RESETPIN, GPIO_PIN_SET);
//	HAL_Delay(400);
//}
//
//
///**
//  * @brief  复位RxCluster1
//  * @note
//  */
//void xl1278_RxCluster1_Reset(void)
//{
//	HAL_GPIO_WritePin(xl1278_RxCluster1_GPIO_PORT, xl1278_RxCluster1_RESETPIN, GPIO_PIN_RESET);
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(xl1278_RxCluster1_GPIO_PORT, xl1278_RxCluster1_RESETPIN, GPIO_PIN_SET);
//	HAL_Delay(400);
//}
//
//
///**
//  * @brief  复位TxCluster
//  * @note
//  */
//void xl1278_TxCluster_Reset(void)
//{
//	HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_RESETPIN, GPIO_PIN_RESET);
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_RESETPIN, GPIO_PIN_SET);
//	HAL_Delay(400);
//}
//
//
//
///**
//  * @brief  使能NSS引脚
//  * @note
//  * @param 	DeviceID
//  */
//void xl1278_RxCluster0_NSS(uint32_t DeviceID)
//{
//	HAL_GPIO_WritePin(xl1278_RxCluster0_GPIO_PORT, xl1278_RxCluster0_SEL_ENABLE, GPIO_PIN_RESET);
//
//	uint32_t GPIO_Pin_SET = DeviceID & xl1278_RxCluster0_SEL_MASK;
//	uint32_t GPIO_Pin_RESET = ((~DeviceID) & xl1278_RxCluster0_SEL_MASK) << 16U;
//	xl1278_RxCluster0_GPIO_PORT->BSRR = GPIO_Pin_SET;
//	xl1278_RxCluster0_GPIO_PORT->BSRR = GPIO_Pin_RESET;
//
//}
//
//void xl1278_RxCluster1_NSS(uint32_t DeviceID)
//{
//	HAL_GPIO_WritePin(xl1278_RxCluster0_GPIO_PORT, xl1278_RxCluster1_SEL_ENABLE, GPIO_PIN_RESET);
//
//	uint32_t GPIO_Pin_SET = (DeviceID << 8) & xl1278_RxCluster1_SEL_MASK;
//	uint32_t GPIO_Pin_RESET = ((~(DeviceID << 8)) & xl1278_RxCluster1_SEL_MASK) << 16U;
//	xl1278_RxCluster0_GPIO_PORT->BSRR = GPIO_Pin_SET;
//	xl1278_RxCluster0_GPIO_PORT->BSRR = GPIO_Pin_RESET;
//}
//
//void xl1278_TxCluster_NSS(uint32_t DeviceID)
//{
//	if(DeviceID == xl1278_DeviceID_0){
//		HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_Device_0_NSSPIN, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_SEL, GPIO_PIN_RESET);
//	}
//	else if(DeviceID == xl1278_DeviceID_1){
//		HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_Device_1_NSSPIN, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_SEL, GPIO_PIN_SET);
//	}
//}
//
//void xl1278_RxCluster0_Channel_Disable(void)
//{
//	HAL_GPIO_WritePin(xl1278_RxCluster0_GPIO_PORT, xl1278_RxCluster0_SEL_ENABLE, GPIO_PIN_SET);
//}
//
//void xl1278_RxCluster1_Channel_Disable(void)
//{
//	HAL_GPIO_WritePin(xl1278_RxCluster1_GPIO_PORT, xl1278_RxCluster1_SEL_ENABLE, GPIO_PIN_SET);
//}
//
//void xl1278_TxCluster_Channel_Disable(void)
//{
//	HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_Device_0_NSSPIN, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(xl1278_TxCluster_GPIO_PORT, xl1278_TxCluster_Device_1_NSSPIN, GPIO_PIN_SET);
//}
//
//
//




///**
//  * @brief  设置sx1278簇的某个器件为睡眠模式
//  * @note
//  * @param  cpcb 指向所使用sx1278簇的结构体指针
//  * @param  DeviceID 设备ID
//  */
//int xl1278_ClusterSetSleep(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_SetSleep(cpcb->SPI_Inst);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//
//
///**
//  * @brief  设置sx1278簇的某个器件为待机模式
//  * @note
//  * @param  cpcb 指向所使用sx1278簇的结构体指针
//  * @param  DeviceID 设备ID
//  */
//int xl1278_ClusterSetStandby(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_SetStandby(cpcb->SPI_Inst);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//
//
//
//
//int xl1278_ClusterSetFreq(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint32_t freq_set)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//	/*if(cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY_TX)
//		return xl1278_BUSY;*/
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_SetStandby(cpcb->SPI_Inst);
//	xl1278_SetFreq(cpcb->SPI_Inst, freq_set);
//	if (cpcb == &TxCluster)
//		;
//	else
//		xl1278_SetRxContConfig(cpcb->SPI_Inst);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//
//
//
//int xl1278_ClusterGetFreq(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint32_t *freq_get)
//{
//	Device_StateTypeDef devicestate;
//
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	devicestate = cpcb->Device_PCB[DeviceID].State;
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_GetFreq(cpcb->SPI_Inst, freq_get);
//	cpcb->Device_PCB[DeviceID].State = devicestate;
//	return xl1278_OK;
//}
//
//
///**
// *
// */
//int xl1278_ClusterGetSNR(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, int *snr)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_GetSNR(cpcb->SPI_Inst, snr);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//int xl1278_ClusterClearIrq(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_ClearIrq(cpcb->SPI_Inst);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//
//int xl1278_ClusterTxPacket(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t *pdata, uint16_t length)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY_TX)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_TxPacket(cpcb->SPI_Inst, pdata, length);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY_TX;
//	return xl1278_OK;
//}
//
//
//int xl1278_ClusterTxDone(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//
//	cpcb->Active_Device = DeviceID;
//	xl1278_ClearIrq(cpcb->SPI_Inst);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//
//	return xl1278_OK;
//}
//
//
//int xl1278_ClusterSetRxContConfig(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_SetRxContConfig(cpcb->SPI_Inst);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//
//int xl1278_ClusterRxPacket(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t *pdata, uint16_t *length)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_RxPacket(cpcb->SPI_Inst, pdata, length);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//int xl1278_ClusterRegWrite(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t addr, uint8_t data)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_RegWrite(cpcb->SPI_Inst, addr, data);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//
//int xl1278_ClusterRegRead(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID, uint8_t addr, uint8_t *pdata)
//{
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_NOTFOUND)
//		return xl1278_ERROR;
//	if (cpcb->Device_PCB[DeviceID].State == DEVICE_STATE_BUSY)
//		return xl1278_BUSY;
//
//	cpcb->Active_Device = DeviceID;
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_BUSY;
//	xl1278_RegRead(cpcb->SPI_Inst, addr, pdata);
//	cpcb->Device_PCB[DeviceID].State = DEVICE_STATE_IDLE;
//	return xl1278_OK;
//}
//



/**
 *
 */
__weak void xl1278_TxCpltCallback(void)
{

}



/**
 *
 */
__weak void xl1278_RxCpltCallback(void)
{

}


//
//
//XL1278_Event xl1278_TxDonePolling()
//{
//	XL1278_Event event;
//	event.pTxCluster_PCB = &TxCluster;
//	event.Event = DEVICE_NOEVENT;
//	uint16_t tx_dio0_pin = TxDone_DIO_PORT->IDR;
//	if (tx_dio0_pin & xl1278_TxCluster_Device_0_DIO0)
//	{
//		xl1278_ClusterTxDone(&TxCluster, xl1278_DeviceID_0);
//		event.Event = DEVICE_TXDONE;
//		event.Tx_DeviceID = xl1278_DeviceID_0;
//	}
//	else if (tx_dio0_pin & xl1278_TxCluster_Device_1_DIO0)
//	{
//		xl1278_ClusterTxDone(&TxCluster, xl1278_DeviceID_1);
//		event.Event = DEVICE_TXDONE;
//		event.Tx_DeviceID = xl1278_DeviceID_1;
//	}
//
//	if (event.Event == DEVICE_TXDONE)
//	{
//		XL1278Cluster_TxCpltCallback(event.pTxCluster_PCB, event.Tx_DeviceID);
//	}
//	return event;
//}
//
//
//XL1278_Event xl1278_RxDonePolling()
//{
//	XL1278_Event event;
//	event.Event = DEVICE_NOEVENT;
//	uint16_t rx_dio0_pin = RxDone_DIO_PORT->IDR;
//
//	if (rx_dio0_pin)
//	{
//		for (uint8_t i=0; i<16; i++)
//		{
//			if ((rx_dio0_pin & 0x1) == 0x1)
//			{
//				if (i < 8) {
//					event.pRxCluster_PCB = &RxCluster0;
//					event.Rx_DeviceID = i;
//				}
//				else {
//					event.pRxCluster_PCB = &RxCluster1;
//					event.Rx_DeviceID = i - 8;
//				}
//				event.Event = DEVICE_RXDONE;
//				xl1278_ClusterClearIrq(event.pRxCluster_PCB, event.Rx_DeviceID);
//				break;
//			}
//			rx_dio0_pin = rx_dio0_pin >> 1;
//		}
//	}
//
//	if (event.Event == DEVICE_RXDONE)
//	{
//		XL1278Cluster_RxCpltCallback(event.pRxCluster_PCB, event.Rx_DeviceID);
//
//	}
//	return event;
//}
//
//
//void xl1278_ClusterInit(XL1278_Cluster_PCB *cpcb, uint32_t DeviceID)
//{
//
//	xl1278_DeviceCheak(cpcb, DeviceID);
////	cpcb->Device_PCB[DeviceID].Init.OpMode = xl1278_LoRaMode | xl1278_LowFreqMode_Low | xl1278_Mode_Sleep;
////	cpcb->Device_PCB[DeviceID].Init.PaConfig = xl1278_PaOutputPower_20dbm;
////	cpcb->Device_PCB[DeviceID].Init.OcpConfig = xl1278_Ocp_Disable | xl1278_OcpTrim_Default;
////	cpcb->Device_PCB[DeviceID].Init.Lna = xl1278_LnaGain_G1 | xl1278_LnaBoosHf_Enable;
////	cpcb->Device_PCB[DeviceID].Init.Bandwidth = xl1278_Bandwidth_500k;
////	cpcb->Device_PCB[DeviceID].Init.CodingRate = xl1278_CodingRate_4_5;
////	cpcb->Device_PCB[DeviceID].Init.HeaderMode = xl1278_ImplicitHeaderMode_Disable;
////	cpcb->Device_PCB[DeviceID].Init.ModemConfig1 = cpcb->Device_PCB[DeviceID].Init.Bandwidth |
////												cpcb->Device_PCB[DeviceID].Init.CodingRate |
////												cpcb->Device_PCB[DeviceID].Init.HeaderMode;
////	cpcb->Device_PCB[DeviceID].Init.SpreadFactor = xl1278_SpreadFactor_4096;
////	cpcb->Device_PCB[DeviceID].Init.Crc = xl1278_RxPayloadCrcOn_Enable;
////	cpcb->Device_PCB[DeviceID].Init.ModemConfig2 = cpcb->Device_PCB[DeviceID].Init.SpreadFactor |
////												cpcb->Device_PCB[DeviceID].Init.Crc |
////												xl1278_SymbTimeoutMsb_Max;
////	cpcb->Device_PCB[DeviceID].Init.SymbTimeoutLsb = 0xFF;
////	cpcb->Device_PCB[DeviceID].Init.Preamble = 12;
////	if(cpcb == &TxCluster)
////		cpcb->Device_PCB[DeviceID].Init.DioMapping1 = xl1278_DIOMAPPING_DIO0_TxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;
////	else
////		cpcb->Device_PCB[DeviceID].Init.DioMapping1 = xl1278_DIOMAPPING_DIO0_RxDone | xl1278_DIOMAPPING_DIO1_Default | xl1278_DIOMAPPING_DIO2_Default | xl1278_DIOMAPPING_DIO3_Default;
////	cpcb->Device_PCB[DeviceID].Init.DioMapping2 = xl1278_DIOMAPPING_DIO4_Default | xl1278_DIOMAPPING_DIO5_Default;
//
//	cpcb->Active_Device = DeviceID;
//	xl1278_Init(cpcb->SPI_Inst, &cpcb->Device_PCB[DeviceID].Init);
//
//}




																					
																					
																					
