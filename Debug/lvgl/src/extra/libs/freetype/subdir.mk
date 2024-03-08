################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/extra/libs/freetype/lv_freetype.c 

OBJS += \
./lvgl/src/extra/libs/freetype/lv_freetype.o 

C_DEPS += \
./lvgl/src/extra/libs/freetype/lv_freetype.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/extra/libs/freetype/%.o lvgl/src/extra/libs/freetype/%.su lvgl/src/extra/libs/freetype/%.cyclo: ../lvgl/src/extra/libs/freetype/%.c lvgl/src/extra/libs/freetype/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L431xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"E:/GitHub_Clone/CubeIDE_Project/LoRa_mini/lvgl" -I"E:/GitHub_Clone/CubeIDE_Project/LoRa_mini/lvgl/porting" -I"E:/GitHub_Clone/CubeIDE_Project/LoRa_mini/lvgl/demos" -I"E:/GitHub_Clone/CubeIDE_Project/LoRa_mini/lvgl/demos/benchmark" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-src-2f-extra-2f-libs-2f-freetype

clean-lvgl-2f-src-2f-extra-2f-libs-2f-freetype:
	-$(RM) ./lvgl/src/extra/libs/freetype/lv_freetype.cyclo ./lvgl/src/extra/libs/freetype/lv_freetype.d ./lvgl/src/extra/libs/freetype/lv_freetype.o ./lvgl/src/extra/libs/freetype/lv_freetype.su

.PHONY: clean-lvgl-2f-src-2f-extra-2f-libs-2f-freetype

