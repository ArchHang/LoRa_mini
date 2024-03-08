#include "main.h"
#include "spi.h"




typedef struct
{
  uint32_t ColorSpace;               /*!< Image Color space : gray-scale, YCBCR, RGB or CMYK
                                           This parameter can be a value of @ref JPEG_ColorSpace */

  uint32_t ChromaSubsampling;        /*!< Chroma Subsampling in case of YCBCR or CMYK color space, 0-> 4:4:4 , 1-> 4:2:2, 2 -> 4:1:1, 3 -> 4:2:0
                                           This parameter can be a value of @ref JPEG_ChromaSubsampling */

  uint32_t ImageHeight;              /*!< Image height : number of lines */

  uint32_t ImageWidth;               /*!< Image width : number of pixels per line */


} ST7789_ConfTypeDef;

#define LCD_Width   240
#define LCD_Height  320
#define LCD_Pixel	LCD_Width*LCD_Height

//#define SOFTWARE_NSS

#define LCD_BL_PIN  GPIO_PIN_2
#define LCD_BL_PORT GPIOD
#define LCD_DC_PIN  GPIO_PIN_11
#define LCD_DC_PORT GPIOC
#define NSS_PIN		GPIO_PIN_5
#define NSS_PORT 	GPIOH

#define LCD_ARGB8888            0  /* ARGB8888 Color Format */
#define LCD_RGB888              1  /* RGB888 Color Format   */
#define LCD_RGB565              2  /* RGB565 Color Format   */

#define LCD_RGB_FORMAT			LCD_RGB565

#if (LCD_RGB_FORMAT == LCD_RGB565)
#define RED_MASK	((uint16_t)0xF800)
#define GREEN_MASK	((uint16_t)0x07E0)
#define BLUD_MASK	((uint16_t)0x001F)
#else
#define RED_MASK	((uint16_t)0x003F)
#define GREEN_MASK	((uint16_t)0x003F)
#define BLUD_MASK	((uint16_t)0x003F)
#endif

#define LCD_X_Offset 0
#define LCD_Y_Offset 0

#define LCD_BufferSize	16



/*----------------------- Funtion ---------------------*/
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Refresh(const uint16_t *frame_buf);
void LCD_AreaRefresh(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint16_t *frame_buf);
void LCD_Init(void);









