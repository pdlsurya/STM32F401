################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CustomLib/mySdFat/mySdFat.c 

OBJS += \
./CustomLib/mySdFat/mySdFat.o 

C_DEPS += \
./CustomLib/mySdFat/mySdFat.d 


# Each subdirectory must supply rules for building sources it contributes
CustomLib/mySdFat/%.o CustomLib/mySdFat/%.su CustomLib/mySdFat/%.cyclo: ../CustomLib/mySdFat/%.c CustomLib/mySdFat/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../CustomLib/flashProgram -I../CustomLib/SD_driver -I../CustomLib/mySdFat -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CustomLib-2f-mySdFat

clean-CustomLib-2f-mySdFat:
	-$(RM) ./CustomLib/mySdFat/mySdFat.cyclo ./CustomLib/mySdFat/mySdFat.d ./CustomLib/mySdFat/mySdFat.o ./CustomLib/mySdFat/mySdFat.su

.PHONY: clean-CustomLib-2f-mySdFat

