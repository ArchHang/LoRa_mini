#include "ST7789_Driver.h"
#include <stdio.h>

SPI_HandleTypeDef *phspi = &hspi3;



void LCD_DisplayOn(void)
{
	HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
}


void LCD_DisplayOff(void)
{
	HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);
}

void LCD_DC_Cmd(void)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
}

void LCD_DC_Data(void)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
}

void SPI_Set_DataSize(uint32_t datasize)
{
	uint32_t frxth;

	phspi->Init.DataSize = datasize;

	if (phspi->Init.DataSize > SPI_DATASIZE_8BIT)
	{
		frxth = SPI_RXFIFO_THRESHOLD_HF;
	}
	else
	{
	    frxth = SPI_RXFIFO_THRESHOLD_QF;
	}

    WRITE_REG(phspi->Instance->CR2, (((phspi->Init.NSS >> 16U) & SPI_CR2_SSOE) |
                                    (phspi->Init.TIMode & SPI_CR2_FRF) |
                                    (phspi->Init.NSSPMode & SPI_CR2_NSSP) |
                                    (phspi->Init.DataSize & SPI_CR2_DS_Msk) |
                                    (frxth & SPI_CR2_FRXTH)));
}


void LCD_Write_Cmd(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);

#ifdef SOFTWARE_NSS
	HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_RESET);
#endif
	HAL_SPI_Transmit(phspi, &cmd, 1, 100);
#ifdef SOFTWARE_NSS
	HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_SET);
#endif
}

void LCD_Write_Data(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

#ifdef SOFTWARE_NSS
	HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_RESET);
#endif
	HAL_SPI_Transmit(phspi, &data, 1, 100);
#ifdef SOFTWARE_NSS
	HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_SET);
#endif
}


void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* 指定X方向操作区域 */
    LCD_Write_Cmd(0x2a);
    x1+=LCD_X_Offset;
    x2+=LCD_X_Offset;
    LCD_Write_Data(x1 >> 8);
    LCD_Write_Data(x1);
    LCD_Write_Data(x2 >> 8);
    LCD_Write_Data(x2);

    /* 指定Y方向操作区域 */
    y1+=LCD_Y_Offset;
    y2+=LCD_Y_Offset;
    LCD_Write_Cmd(0x2b);
    LCD_Write_Data(y1 >> 8);
    LCD_Write_Data(y1);
    LCD_Write_Data(y2 >> 8);
    LCD_Write_Data(y2);

    /* 发送该命令，LCD开始等待接收显存数据 */
    LCD_Write_Cmd(0x2C);
}


void LCD_Refresh(const uint16_t *frame_buf)
{

    /* 指定显存操作地址为全屏幕 */
    LCD_Address_Set(0, 0, LCD_Width - 1, LCD_Height - 1);
    /* 指定接下来的数据为数据 */
//    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
    LCD_DC_Data();

    /* 修改spi发送数据长度 */
//    phspi->Init.DataSize = SPI_DATASIZE_16BIT;
//    HAL_SPI_Init(phspi);
    SPI_Set_DataSize(SPI_DATASIZE_16BIT);

    /* 将显存缓冲区的数据全部写入缓冲区 */
#ifdef SOFTWARE_NSS
    HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_RESET);
#endif
    HAL_SPI_Transmit(phspi, (uint8_t *)frame_buf, LCD_Width*LCD_Height/2, 1000);
    HAL_SPI_Transmit(phspi, (uint8_t *)&frame_buf[LCD_Width*LCD_Height/2], LCD_Width*LCD_Height/2, 1000);
#ifdef SOFTWARE_NSS
    HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_SET);
#endif

//    phspi->Init.DataSize = SPI_DATASIZE_8BIT;
//    HAL_SPI_Init(phspi);
    SPI_Set_DataSize(SPI_DATASIZE_8BIT);
}


void LCD_AreaRefresh(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t *frame_buf)
{
	uint32_t pixelsize = (x2-x1+1)*(y2-y1+1);

    /* 指定显存操作地址为全屏幕 */
    LCD_Address_Set(x1, y1, x2, y2);
    /* 指定接下来的数据为数据 */
//    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
    LCD_DC_Data();

    /* 修改spi发送数据长度 */
//    phspi->Init.DataSize = SPI_DATASIZE_16BIT;
//    HAL_SPI_Init(phspi);
    SPI_Set_DataSize(SPI_DATASIZE_16BIT);

    /* 将显存缓冲区的数据全部写入缓冲区 */
#ifdef SOFTWARE_NSS
    HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_RESET);
#endif
    if (pixelsize>65535)
    {
    	HAL_SPI_Transmit(phspi, (uint8_t *)frame_buf, 30000, 1000);
    	HAL_SPI_Transmit(phspi, (uint8_t *)&frame_buf[30000], pixelsize-30000, 1000);
    }
    else
    	HAL_SPI_Transmit(phspi, (uint8_t *)frame_buf, pixelsize, 1000);
#ifdef SOFTWARE_NSS
    HAL_GPIO_WritePin(NSS_PORT, NSS_PIN, GPIO_PIN_SET);
#endif

//    phspi->Init.DataSize = SPI_DATASIZE_8BIT;
//    HAL_SPI_Init(phspi);
    SPI_Set_DataSize(SPI_DATASIZE_8BIT);
}

void LCD_AreaColor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint32_t pixelsize = (x2-x1+1)*(y2-y1+1);

    /* 指定显存操作地址为全屏幕 */
    LCD_Address_Set(x1, y1, x2, y2);
    LCD_DC_Data();

    SPI_Set_DataSize(SPI_DATASIZE_16BIT);

    while (pixelsize)
    {

    }
}

void LCD_Init(void)
{



	    /* 开始设置显存扫描模式，数据格式等 */
	    LCD_Write_Cmd(0x36);
	    LCD_Write_Data(0x00);
	    /* RGB 5-6-5-bit格式  */
	    LCD_Write_Cmd(0x3A);
	    LCD_Write_Data(0x05);
	    /* porch 设置 */
	    LCD_Write_Cmd(0xB2);
	    LCD_Write_Data(0x0C);
	    LCD_Write_Data(0x0C);
	    LCD_Write_Data(0x00);
	    LCD_Write_Data(0x33);
	    LCD_Write_Data(0x33);
	    /* VGH设置 */
	    LCD_Write_Cmd(0xB7);
	    LCD_Write_Data(0x35);
	    /* VCOM 设置 */
	    LCD_Write_Cmd(0xBB);
	    LCD_Write_Data(0x19);
	    /* LCM 设置 */
	    LCD_Write_Cmd(0xC0);
	    LCD_Write_Data(0x2C);
	    /* VDV and VRH 设置 */
	    LCD_Write_Cmd(0xC2);
	    LCD_Write_Data(0x01);
	    /* VRH 设置 */
	    LCD_Write_Cmd(0xC3);
	    LCD_Write_Data(0x12);
	    /* VDV 设置 */
	    LCD_Write_Cmd(0xC4);
	    LCD_Write_Data(0x20);
	    /* 普通模式下显存速率设置 60Mhz */
	    LCD_Write_Cmd(0xC6);
	    LCD_Write_Data(0x0F);
	    /* 电源控制 */
	    LCD_Write_Cmd(0xD0);
	    LCD_Write_Data(0xA4);
	    LCD_Write_Data(0xA1);
	    /* 电压设置 */
	    LCD_Write_Cmd(0xE0);
	    LCD_Write_Data(0xD0);
	    LCD_Write_Data(0x04);
	    LCD_Write_Data(0x0D);
	    LCD_Write_Data(0x11);
	    LCD_Write_Data(0x13);
	    LCD_Write_Data(0x2B);
	    LCD_Write_Data(0x3F);
	    LCD_Write_Data(0x54);
	    LCD_Write_Data(0x4C);
	    LCD_Write_Data(0x18);
	    LCD_Write_Data(0x0D);
	    LCD_Write_Data(0x0B);
	    LCD_Write_Data(0x1F);
	    LCD_Write_Data(0x23);
	    /* 电压设置 */
	    LCD_Write_Cmd(0xE1);
	    LCD_Write_Data(0xD0);
	    LCD_Write_Data(0x04);
	    LCD_Write_Data(0x0C);
	    LCD_Write_Data(0x11);
	    LCD_Write_Data(0x13);
	    LCD_Write_Data(0x2C);
	    LCD_Write_Data(0x3F);
	    LCD_Write_Data(0x44);
	    LCD_Write_Data(0x51);
	    LCD_Write_Data(0x2F);
	    LCD_Write_Data(0x1F);
	    LCD_Write_Data(0x1F);
	    LCD_Write_Data(0x20);
	    LCD_Write_Data(0x23);
	    /* 显示开 */
	    LCD_Write_Cmd(0x21);

	    /* 关闭睡眠模式 */
	    LCD_Write_Cmd(0x11);
	    HAL_Delay(120);

	    LCD_Write_Cmd(0x29);




	    /*打开显示*/
	    LCD_DisplayOn();


}
