################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32.s 

OBJS += \
./startup/startup_stm32.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obcTESTrecieve/StdPeriph_Driver/inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obcTESTrecieve/inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obcTESTrecieve/CMSIS/device" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/obcTESTrecieve/CMSIS/core" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


