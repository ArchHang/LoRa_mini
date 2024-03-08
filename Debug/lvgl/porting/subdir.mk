################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/porting/lv_port_disp.c \
../lvgl/porting/lv_port_fs.c \
../lvgl/porting/lv_port_indev.c 

OBJS += \
./lvgl/porting/lv_port_disp.o \
./lvgl/porting/lv_port_fs.o \
./lvgl/porting/lv_port_indev.o 

C_DEPS += \
./lvgl/porting/lv_port_disp.d \
./lvgl/porting/lv_port_fs.d \
./lvgl/porting/lv_port_indev.d 


# Each subdirectory must supply rules for building sources it contributes
lvgl/porting/%.o lvgl/porting/%.su lvgl/porting/%.cyclo: ../lvgl/porting/%.c lvgl/porting/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L431xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"E:/GitHub_Clone/CubeIDE_Project/LoRa_mini/lvgl" -I"E:/GitHub_Clone/CubeIDE_Project/LoRa_mini/lvgl/porting" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lvgl-2f-porting

clean-lvgl-2f-porting:
	-$(RM) ./lvgl/porting/lv_port_disp.cyclo ./lvgl/porting/lv_port_disp.d ./lvgl/porting/lv_port_disp.o ./lvgl/porting/lv_port_disp.su ./lvgl/porting/lv_port_fs.cyclo ./lvgl/porting/lv_port_fs.d ./lvgl/porting/lv_port_fs.o ./lvgl/porting/lv_port_fs.su ./lvgl/porting/lv_port_indev.cyclo ./lvgl/porting/lv_port_indev.d ./lvgl/porting/lv_port_indev.o ./lvgl/porting/lv_port_indev.su

.PHONY: clean-lvgl-2f-porting

