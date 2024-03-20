#include "spi.h"


typedef struct {

	SPI_HandleTypeDef 	*phspi;

	uint16_t			ID;

	uint32_t			Size;

	GPIO_TypeDef		*NSS_GPIO_Port;

	uint16_t 			NSS_GPIO_Pin;

} W25QXX_HandlerTypeDef;


extern W25QXX_HandlerTypeDef	flash1;


//#define	W25Q_NSS_PORT				GPIOB
//#define W25Q_NSS_PIN				GPIO_PIN_12



#define W25Q_ManufactDeviceID_CMD		((uint8_t)0x90)
#define W25Q_READ_DATA_CMD				((uint8_t)0x03)
#define W25Q_READ_SR1_CMD				((uint8_t)0x05)
#define W25Q_READ_SR2_CMD				((uint8_t)0x35)
#define W25Q_WRITE_ENABLE_CMD			((uint8_t)0x06)
#define W25Q_WRITE_DISABLE_CMD			((uint8_t)0x04)
#define W25Q_SECTOR_ERASE_CMD	    	((uint8_t)0x20)
#define W25Q_CHIP_ERASE_CMD	        	((uint8_t)0xc7)
#define W25Q_PAGE_PROGRAM_CMD        	((uint8_t)0x02)
#define W25Q_POWERDOWN_CMD				((uint8_t)0xb9)
#define W25Q_POWERDOWN_RELEASE_CMD		((uint8_t)0xab)




int W25QXX_Init(void);
int W25QXX_PowerDonw_Enable(W25QXX_HandlerTypeDef *pHandler);
int W25QXX_PowerDonw_Disable(W25QXX_HandlerTypeDef *pHandler);
int W25QXX_Read(W25QXX_HandlerTypeDef *pHandler, uint8_t *pData, uint32_t Addr, uint16_t Size);
int W25QXX_Erase_Sector(W25QXX_HandlerTypeDef *pHandler, uint32_t Sector_Addr);
int W25QXX_Erase_Chip(W25QXX_HandlerTypeDef *pHandler);
int W25QXX_WritePage(W25QXX_HandlerTypeDef *pHandler, uint8_t *pData, uint32_t Addr, uint16_t Size);
int W25QXX_ReadID(W25QXX_HandlerTypeDef *pHandler);
