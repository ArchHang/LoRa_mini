#include "W25QXX_Driver.h"

W25QXX_HandlerTypeDef	flash1;

void W25QXX_NSS_Enable(W25QXX_HandlerTypeDef *pHandler)
{
	HAL_GPIO_WritePin(pHandler->NSS_GPIO_Port, pHandler->NSS_GPIO_Pin, GPIO_PIN_RESET);
}

void W25QXX_NSS_Disable(W25QXX_HandlerTypeDef *pHandler)
{
	HAL_GPIO_WritePin(pHandler->NSS_GPIO_Port, pHandler->NSS_GPIO_Pin, GPIO_PIN_SET);
}

int W25QXX_Write_Enable(W25QXX_HandlerTypeDef *pHandler)
{
	int status;
	uint8_t tx_buf = W25Q_WRITE_ENABLE_CMD;

	W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, &tx_buf, 1, 50);
	W25QXX_NSS_Disable(pHandler);

	return status;
}

int W25QXX_Write_Disable(W25QXX_HandlerTypeDef *pHandler)
{
	int status;
	uint8_t tx_buf = W25Q_WRITE_DISABLE_CMD;

	W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, &tx_buf, 1, 50);
	W25QXX_NSS_Disable(pHandler);

	return status;
}

uint8_t W25QXX_ReadReg(W25QXX_HandlerTypeDef *pHandler, uint8_t reg)
{
	int status;
	uint8_t recv_buf;
	uint8_t tx_buf[4] = { reg, 0, 0, 0};

	W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, tx_buf, 4, 50);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return 0;
	}

	status = HAL_SPI_Receive(pHandler->phspi, &recv_buf, 1, 50);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return 0;
	}
	W25QXX_NSS_Disable(pHandler);

	return recv_buf;
}

/**
 * @brief	等待Flash空闲
 * @param   pHandler
 * @param	Timeout		单位ms
 */
int W25QXX_Wait_Busy(W25QXX_HandlerTypeDef *pHandler, uint32_t Timeout)
{
	uint32_t tickstart = HAL_GetTick();

	while (W25QXX_ReadReg(pHandler, W25Q_READ_SR1_CMD) & 0x01) {
		if ((HAL_GetTick() - tickstart) > Timeout)
			return HAL_TIMEOUT;
	}

	return HAL_OK;
}


/**
 *
 */
int W25QXX_ReadID(W25QXX_HandlerTypeDef *pHandler)
{
	int status;
	uint8_t recv_buf[2];
	uint8_t tx_buf[4] = { W25Q_ManufactDeviceID_CMD, 0, 0, 0};

	W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, tx_buf, 4, 50);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return status;
	}

	status = HAL_SPI_Receive(pHandler->phspi, recv_buf, 2, 50);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return status;
	}
	W25QXX_NSS_Disable(pHandler);

	pHandler->ID = (recv_buf[0] << 8) | recv_buf[1];
	return status;
}

/**
 *
 */
int W25QXX_Read(W25QXX_HandlerTypeDef *pHandler, uint8_t *pData, uint32_t Addr, uint16_t Size)
{
	int status;
	uint8_t tx_buf[4] = { W25Q_READ_DATA_CMD, 0, 0, 0};

	W25QXX_Wait_Busy(pHandler, 100);

	tx_buf[1] = Addr >> 16;
	tx_buf[2] = Addr >> 8;
	tx_buf[3] = Addr;

	W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, tx_buf, 4, 50);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return status;
	}

	status = HAL_SPI_Receive(pHandler->phspi, pData, Size, 100);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return status;
	}
	W25QXX_NSS_Disable(pHandler);

	return status;
}


/**
 * @brief   W25QXX擦除一个扇区
 * @param
 * @param   sector_addr    —— 扇区地址 根据实际容量设置
 * @retval
 * @note    阻塞操作
 */
int W25QXX_Erase_Sector(W25QXX_HandlerTypeDef *pHandler, uint32_t Sector_Addr)
{
	int status;
	uint8_t tx_buf[4] = { W25Q_SECTOR_ERASE_CMD, 0, 0, 0};

	W25QXX_Write_Enable(pHandler);  //擦除操作即写入0xFF，需要开启写使能
    status = W25QXX_Wait_Busy(pHandler, 100);
	if (status != HAL_OK) {
		return status;
	}

	Sector_Addr = Sector_Addr << 12;
	tx_buf[1] = Sector_Addr >> 16;
	tx_buf[2] = Sector_Addr >> 8;
	tx_buf[3] = Sector_Addr;

    W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, tx_buf, 4, 50);
    W25QXX_NSS_Disable(pHandler);

    return status;
}

int W25QXX_Erase_Chip(W25QXX_HandlerTypeDef *pHandler)
{
	int status;
	uint8_t tx_buf = W25Q_CHIP_ERASE_CMD;

	W25QXX_Write_Enable(pHandler);  //擦除操作即写入0xFF，需要开启写使能
    status = W25QXX_Wait_Busy(pHandler, 100);
	if (status != HAL_OK) {
		return status;
	}

    W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, &tx_buf, 1, 50);
    W25QXX_NSS_Disable(pHandler);

    status = W25QXX_Wait_Busy(pHandler, 5000);
    return status;
}


int W25QXX_WritePage(W25QXX_HandlerTypeDef *pHandler, uint8_t *pData, uint32_t Addr, uint16_t Size)
{
	int status;
	uint16_t max_write_size = 0;
	uint8_t tx_buf[4] = { W25Q_PAGE_PROGRAM_CMD, 0, 0, 0};

    status = W25QXX_Wait_Busy(pHandler, 100);
	if (status != HAL_OK) {
		return status;
	}
	W25QXX_Write_Enable(pHandler);

	tx_buf[1] = Addr >> 16;
	tx_buf[2] = Addr >> 8;
	tx_buf[3] = Addr;
	max_write_size = 256 - ((Addr % 256));
	if (Size > max_write_size)
		Size = max_write_size;

	W25QXX_NSS_Enable(pHandler);
	status = HAL_SPI_Transmit(pHandler->phspi, tx_buf, 4, 50);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return status;
	}

	status = HAL_SPI_Transmit(pHandler->phspi, pData, Size, 100);
	if (status != HAL_OK) {
		W25QXX_NSS_Disable(pHandler);
		return status;
	}
	W25QXX_NSS_Disable(pHandler);

	return status;
}


int W25QXX_Init(void)
{
	int status;

	flash1.phspi = &hspi2;
	flash1.NSS_GPIO_Port = GPIOB;
	flash1.NSS_GPIO_Pin = GPIO_PIN_12;

	status = W25QXX_ReadID(&flash1);
	if (status != HAL_OK) {
		return status;
	}



	return 0;
}

