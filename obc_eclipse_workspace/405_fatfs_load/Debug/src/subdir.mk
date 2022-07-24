################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/diskio.c \
../src/fatfs.c \
../src/fatfs_sd_sdio.c \
../src/ff.c \
../src/ff_gen_drv.c \
../src/main.c \
../src/sd_diskio.c \
../src/syscall.c \
../src/syscalls.c \
../src/system_stm32f4xx.c \
../src/tm_stm32f4_fatfs.c \
../src/tm_stm32f4_gpio.c \
../src/unicode.c 

OBJS += \
./src/diskio.o \
./src/fatfs.o \
./src/fatfs_sd_sdio.o \
./src/ff.o \
./src/ff_gen_drv.o \
./src/main.o \
./src/sd_diskio.o \
./src/syscall.o \
./src/syscalls.o \
./src/system_stm32f4xx.o \
./src/tm_stm32f4_fatfs.o \
./src/tm_stm32f4_gpio.o \
./src/unicode.o 

C_DEPS += \
./src/diskio.d \
./src/fatfs.d \
./src/fatfs_sd_sdio.d \
./src/ff.d \
./src/ff_gen_drv.d \
./src/main.d \
./src/sd_diskio.d \
./src/syscall.d \
./src/syscalls.d \
./src/system_stm32f4xx.d \
./src/tm_stm32f4_fatfs.d \
./src/tm_stm32f4_gpio.d \
./src/unicode.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32F4 -DSTM32F405RGTx -DDEBUG -DSTM32F40XX -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/405_fatfs_load/StdPeriph_Driver/inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/405_fatfs_load/inc" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/405_fatfs_load/CMSIS/device" -I"C:/Users/Abdel/OneDrive/Documents/eclipseWorkspace/obc_upsat/405_fatfs_load/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


