################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CustomLib/SD_driver/SD_driver.c 

OBJS += \
./CustomLib/SD_driver/SD_driver.o 

C_DEPS += \
./CustomLib/SD_driver/SD_driver.d 


# Each subdirectory must supply rules for building sources it contributes
CustomLib/SD_driver/%.o CustomLib/SD_driver/%.su CustomLib/SD_driver/%.cyclo: ../CustomLib/SD_driver/%.c CustomLib/SD_driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../CustomLib/flashProgram -I../CustomLib/SD_driver -I../CustomLib/mySdFat -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CustomLib-2f-SD_driver

clean-CustomLib-2f-SD_driver:
	-$(RM) ./CustomLib/SD_driver/SD_driver.cyclo ./CustomLib/SD_driver/SD_driver.d ./CustomLib/SD_driver/SD_driver.o ./CustomLib/SD_driver/SD_driver.su

.PHONY: clean-CustomLib-2f-SD_driver

