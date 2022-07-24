################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../upsat-ecss-services-master/core/ecss_stats.c \
../upsat-ecss-services-master/core/flash.c \
../upsat-ecss-services-master/core/gps.c \
../upsat-ecss-services-master/core/hldlc.c \
../upsat-ecss-services-master/core/pkt_pool.c \
../upsat-ecss-services-master/core/queue.c \
../upsat-ecss-services-master/core/su_mnlp.c \
../upsat-ecss-services-master/core/sysview.c \
../upsat-ecss-services-master/core/upsat.c 

OBJS += \
./upsat-ecss-services-master/core/ecss_stats.o \
./upsat-ecss-services-master/core/flash.o \
./upsat-ecss-services-master/core/gps.o \
./upsat-ecss-services-master/core/hldlc.o \
./upsat-ecss-services-master/core/pkt_pool.o \
./upsat-ecss-services-master/core/queue.o \
./upsat-ecss-services-master/core/su_mnlp.o \
./upsat-ecss-services-master/core/sysview.o \
./upsat-ecss-services-master/core/upsat.o 

C_DEPS += \
./upsat-ecss-services-master/core/ecss_stats.d \
./upsat-ecss-services-master/core/flash.d \
./upsat-ecss-services-master/core/gps.d \
./upsat-ecss-services-master/core/hldlc.d \
./upsat-ecss-services-master/core/pkt_pool.d \
./upsat-ecss-services-master/core/queue.d \
./upsat-ecss-services-master/core/su_mnlp.d \
./upsat-ecss-services-master/core/sysview.d \
./upsat-ecss-services-master/core/upsat.d 


# Each subdirectory must supply rules for building sources it contributes
upsat-ecss-services-master/core/%.o: ../upsat-ecss-services-master/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32F405xx -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FatFs/src/drivers" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/CMSIS/Include"  -O2 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


