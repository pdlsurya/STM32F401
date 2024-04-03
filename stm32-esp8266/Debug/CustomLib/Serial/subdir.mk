################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CustomLib/Serial/Serial.c 

OBJS += \
./CustomLib/Serial/Serial.o 

C_DEPS += \
./CustomLib/Serial/Serial.d 


# Each subdirectory must supply rules for building sources it contributes
CustomLib/Serial/%.o CustomLib/Serial/%.su CustomLib/Serial/%.cyclo: ../CustomLib/Serial/%.c CustomLib/Serial/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../CustomLib/esp8266_mqtt -I../CustomLib/Serial -I../CustomLib/oled_SH1106 -I../CustomLib/NTP_Clock -I../CustomLib/hal_esp8266 -I../CustomLib/USB_Serial -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CustomLib-2f-Serial

clean-CustomLib-2f-Serial:
	-$(RM) ./CustomLib/Serial/Serial.cyclo ./CustomLib/Serial/Serial.d ./CustomLib/Serial/Serial.o ./CustomLib/Serial/Serial.su

.PHONY: clean-CustomLib-2f-Serial

