################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/FATFS/FatFs/source/ff.c \
../Middlewares/FATFS/FatFs/source/ffsystem.c \
../Middlewares/FATFS/FatFs/source/ffunicode.c 

OBJS += \
./Middlewares/FATFS/FatFs/source/ff.o \
./Middlewares/FATFS/FatFs/source/ffsystem.o \
./Middlewares/FATFS/FatFs/source/ffunicode.o 

C_DEPS += \
./Middlewares/FATFS/FatFs/source/ff.d \
./Middlewares/FATFS/FatFs/source/ffsystem.d \
./Middlewares/FATFS/FatFs/source/ffunicode.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/FATFS/FatFs/source/%.o Middlewares/FATFS/FatFs/source/%.su Middlewares/FATFS/FatFs/source/%.cyclo: ../Middlewares/FATFS/FatFs/source/%.c Middlewares/FATFS/FatFs/source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -c -I../../../Appli/Core/Inc -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/ADC" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/evision/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/Src" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/USB_Device/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/USB_Device/Src" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp_param_conf" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS/FatFs/source" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS/exfuns" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/KEY" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/LED" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/RGBLCD" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SD_CARD" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SD_NAND" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SYS" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/UART" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Application/User/FatFs" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Application/User/STM32_MW_ISP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/MALLOC" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/PICTURE" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/AP3216C" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/BEEP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/HyperRAM" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/IMX335" -I../../../Secure_nsclib -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-FATFS-2f-FatFs-2f-source

clean-Middlewares-2f-FATFS-2f-FatFs-2f-source:
	-$(RM) ./Middlewares/FATFS/FatFs/source/ff.cyclo ./Middlewares/FATFS/FatFs/source/ff.d ./Middlewares/FATFS/FatFs/source/ff.o ./Middlewares/FATFS/FatFs/source/ff.su ./Middlewares/FATFS/FatFs/source/ffsystem.cyclo ./Middlewares/FATFS/FatFs/source/ffsystem.d ./Middlewares/FATFS/FatFs/source/ffsystem.o ./Middlewares/FATFS/FatFs/source/ffsystem.su ./Middlewares/FATFS/FatFs/source/ffunicode.cyclo ./Middlewares/FATFS/FatFs/source/ffunicode.d ./Middlewares/FATFS/FatFs/source/ffunicode.o ./Middlewares/FATFS/FatFs/source/ffunicode.su

.PHONY: clean-Middlewares-2f-FATFS-2f-FatFs-2f-source

