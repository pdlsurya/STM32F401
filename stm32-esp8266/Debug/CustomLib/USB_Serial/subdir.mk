################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CustomLib/USB_Serial/USB_Serial.c 

OBJS += \
./CustomLib/USB_Serial/USB_Serial.o 

C_DEPS += \
./CustomLib/USB_Serial/USB_Serial.d 


# Each subdirectory must supply rules for building sources it contributes
CustomLib/USB_Serial/%.o CustomLib/USB_Serial/%.su CustomLib/USB_Serial/%.cyclo: ../CustomLib/USB_Serial/%.c CustomLib/USB_Serial/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../CustomLib/esp8266_mqtt -I../CustomLib/Serial -I../CustomLib/oled_SH1106 -I../CustomLib/NTP_Clock -I../CustomLib/hal_esp8266 -I../CustomLib/USB_Serial -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CustomLib-2f-USB_Serial

clean-CustomLib-2f-USB_Serial:
	-$(RM) ./CustomLib/USB_Serial/USB_Serial.cyclo ./CustomLib/USB_Serial/USB_Serial.d ./CustomLib/USB_Serial/USB_Serial.o ./CustomLib/USB_Serial/USB_Serial.su

.PHONY: clean-CustomLib-2f-USB_Serial

