################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CustomLib/NTP_Clock/NTP_Clock.c 

OBJS += \
./CustomLib/NTP_Clock/NTP_Clock.o 

C_DEPS += \
./CustomLib/NTP_Clock/NTP_Clock.d 


# Each subdirectory must supply rules for building sources it contributes
CustomLib/NTP_Clock/%.o CustomLib/NTP_Clock/%.su CustomLib/NTP_Clock/%.cyclo: ../CustomLib/NTP_Clock/%.c CustomLib/NTP_Clock/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../CustomLib/esp8266_mqtt -I../CustomLib/Serial -I../CustomLib/oled_SH1106 -I../CustomLib/NTP_Clock -I../CustomLib/hal_esp8266 -I../CustomLib/USB_Serial -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CustomLib-2f-NTP_Clock

clean-CustomLib-2f-NTP_Clock:
	-$(RM) ./CustomLib/NTP_Clock/NTP_Clock.cyclo ./CustomLib/NTP_Clock/NTP_Clock.d ./CustomLib/NTP_Clock/NTP_Clock.o ./CustomLib/NTP_Clock/NTP_Clock.su

.PHONY: clean-CustomLib-2f-NTP_Clock

