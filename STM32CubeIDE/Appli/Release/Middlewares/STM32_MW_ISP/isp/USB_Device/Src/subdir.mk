################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.c \
../Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.c \
../Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.c 

OBJS += \
./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.o \
./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.o \
./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.o 

C_DEPS += \
./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.d \
./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.d \
./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/STM32_MW_ISP/isp/USB_Device/Src/%.o Middlewares/STM32_MW_ISP/isp/USB_Device/Src/%.su Middlewares/STM32_MW_ISP/isp/USB_Device/Src/%.cyclo: ../Middlewares/STM32_MW_ISP/isp/USB_Device/Src/%.c Middlewares/STM32_MW_ISP/isp/USB_Device/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m55 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32N647xx -c -I../../../Appli/Core/Inc -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/ADC" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/evision/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/Src" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/USB_Device/Inc" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp/USB_Device/Src" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP/isp_param_conf" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS/FatFs/source" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS/exfuns" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/KEY" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/LED" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/RGBLCD" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SD_CARD" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SD_NAND" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/SYS" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/UART" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Application/User/FatFs" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Application/User/STM32_MW_ISP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/FATFS" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/MALLOC" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/PICTURE" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Middlewares/STM32_MW_ISP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/AP3216C" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/BEEP" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/HyperRAM" -I"D:/modify_project/Projects/gr/STM32CubeIDE/Appli/Drivers/BSP/IMX335" -I../../../Secure_nsclib -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Device/ST/STM32N6xx/Include -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/STM32N6xx_HAL_Driver/Inc/Legacy -ID:/modify_project/STM32Cube_FW_N6_V1.0.0/Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -mcmse -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-STM32_MW_ISP-2f-isp-2f-USB_Device-2f-Src

clean-Middlewares-2f-STM32_MW_ISP-2f-isp-2f-USB_Device-2f-Src:
	-$(RM) ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.cyclo ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.d ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.o ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usb_device.su ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.cyclo ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.d ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.o ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_cdc_if.su ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.cyclo ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.d ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.o ./Middlewares/STM32_MW_ISP/isp/USB_Device/Src/usbd_desc.su

.PHONY: clean-Middlewares-2f-STM32_MW_ISP-2f-isp-2f-USB_Device-2f-Src

