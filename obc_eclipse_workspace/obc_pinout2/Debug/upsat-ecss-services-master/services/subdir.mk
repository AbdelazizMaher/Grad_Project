################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../upsat-ecss-services-master/services/event_reporting_service.c \
../upsat-ecss-services-master/services/function_management_service.c \
../upsat-ecss-services-master/services/housekeeping_service.c \
../upsat-ecss-services-master/services/large_data_service.c \
../upsat-ecss-services-master/services/mass_storage_service.c \
../upsat-ecss-services-master/services/scheduling_service.c \
../upsat-ecss-services-master/services/service_utilities.c \
../upsat-ecss-services-master/services/services.c \
../upsat-ecss-services-master/services/test_service.c \
../upsat-ecss-services-master/services/time_management_service.c \
../upsat-ecss-services-master/services/verification_service.c 

OBJS += \
./upsat-ecss-services-master/services/event_reporting_service.o \
./upsat-ecss-services-master/services/function_management_service.o \
./upsat-ecss-services-master/services/housekeeping_service.o \
./upsat-ecss-services-master/services/large_data_service.o \
./upsat-ecss-services-master/services/mass_storage_service.o \
./upsat-ecss-services-master/services/scheduling_service.o \
./upsat-ecss-services-master/services/service_utilities.o \
./upsat-ecss-services-master/services/services.o \
./upsat-ecss-services-master/services/test_service.o \
./upsat-ecss-services-master/services/time_management_service.o \
./upsat-ecss-services-master/services/verification_service.o 

C_DEPS += \
./upsat-ecss-services-master/services/event_reporting_service.d \
./upsat-ecss-services-master/services/function_management_service.d \
./upsat-ecss-services-master/services/housekeeping_service.d \
./upsat-ecss-services-master/services/large_data_service.d \
./upsat-ecss-services-master/services/mass_storage_service.d \
./upsat-ecss-services-master/services/scheduling_service.d \
./upsat-ecss-services-master/services/service_utilities.d \
./upsat-ecss-services-master/services/services.d \
./upsat-ecss-services-master/services/test_service.d \
./upsat-ecss-services-master/services/time_management_service.d \
./upsat-ecss-services-master/services/verification_service.d 


# Each subdirectory must supply rules for building sources it contributes
upsat-ecss-services-master/services/%.o: ../upsat-ecss-services-master/services/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32F405xx -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FatFs/src/drivers" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obc_pinout2/Drivers/CMSIS/Include"  -O2 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


