################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/FatFs/src/drivers/sd_diskio.c 

OBJS += \
./Middlewares/Third_Party/FatFs/src/drivers/sd_diskio.o 

C_DEPS += \
./Middlewares/Third_Party/FatFs/src/drivers/sd_diskio.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/FatFs/src/drivers/%.o: ../Middlewares/Third_Party/FatFs/src/drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32F405xx -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Middlewares/Third_Party/FatFs/src/drivers" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/upsat-obc-software-master/cubeMX/obc/Drivers/CMSIS/Include"  -O2 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


