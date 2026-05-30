################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/SYS/sys.c 

OBJS += \
./Drivers/BSP/SYS/sys.o 

C_DEPS += \
./Drivers/BSP/SYS/sys.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/SYS/%.o Drivers/BSP/SYS/%.su Drivers/BSP/SYS/%.cyclo: ../Drivers/BSP/SYS/%.c Drivers/BSP/SYS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DLL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL -DLL_ATON_RT_MODE=LL_ATON_RT_ASYNC -DLL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6 -DLL_ATON_SW_FALLBACK=1 -DUSE_HAL_DRIVER -DSTM32N647xx -c -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/AI" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/AI/Npu/ll_aton" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/AI/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/AI/Lib" -I../../../Appli/Core/Inc -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/AI/Npu/Devices/STM32N6xx" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/ADC" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/evision/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/Src" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/USB_Device/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/USB_Device/Src" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp_param_conf" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS/FatFs/source" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS/exfuns" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/KEY" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/LED" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/RGBLCD" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SD_CARD" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SD_NAND" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SYS" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/UART" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Application/User/FatFs" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Application/User/STM32_MW_ISP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/MALLOC" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/PICTURE" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/AP3216C" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/BEEP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/HyperRAM" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/IMX335" -I../../../Secure_nsclib -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-SYS

clean-Drivers-2f-BSP-2f-SYS:
	-$(RM) ./Drivers/BSP/SYS/sys.cyclo ./Drivers/BSP/SYS/sys.d ./Drivers/BSP/SYS/sys.o ./Drivers/BSP/SYS/sys.su

.PHONY: clean-Drivers-2f-BSP-2f-SYS

