################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/STM32_MW_ISP/isp/Src/subdir.mk
-include Middlewares/PICTURE/subdir.mk
-include Middlewares/MALLOC/subdir.mk
-include Middlewares/FATFS/exfuns/subdir.mk
-include Middlewares/FATFS/FatFs/source/subdir.mk
-include Drivers/STM32N6xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/UART/subdir.mk
-include Drivers/BSP/SYS/subdir.mk
-include Drivers/BSP/SD_NAND/subdir.mk
-include Drivers/BSP/SD_CARD/subdir.mk
-include Drivers/BSP/RGBLCD/subdir.mk
-include Drivers/BSP/LED/subdir.mk
-include Drivers/BSP/KEY/subdir.mk
-include Drivers/BSP/IMX335/STM32_IMX335/subdir.mk
-include Drivers/BSP/IMX335/subdir.mk
-include Drivers/BSP/HyperRAM/subdir.mk
-include Drivers/BSP/BEEP/subdir.mk
-include Drivers/BSP/AP3216C/subdir.mk
-include Drivers/BSP/ADC/subdir.mk
-include Application/User/Startup/subdir.mk
-include Application/User/FatFs/subdir.mk
-include Application/User/Core/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := gr_Appli
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
gr_Appli.elf \

MAP_FILES += \
gr_Appli.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
gr_Appli.list \

OBJCOPY_HEX += \
gr_Appli.hex \

OBJCOPY_BIN += \
gr_Appli.bin \


# All Target
all: main-build

# Main-build Target
main-build: gr_Appli.elf secondary-outputs

# Tool invocations
gr_Appli.elf gr_Appli.map: $(OBJS) $(USER_OBJS) D:\modify_project\Projects\gr\STM32CubeIDE\Appli\STM32N647X0HXQ_ROMxspi2_RAMxspi1.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "gr_Appli.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m55 -T"D:\modify_project\Projects\gr\STM32CubeIDE\Appli\STM32N647X0HXQ_ROMxspi2_RAMxspi1.ld" --specs=nosys.specs -Wl,-Map="gr_Appli.map" -Wl,--gc-sections -static -Wl,--cmse-implib -Wl,--out-implib=./secure_nsclib.o -L"D:\modify_project\Projects\gr\STM32CubeIDE\Appli\Middlewares\STM32_MW_ISP\evision\Lib" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

gr_Appli.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "gr_Appli.list"
	@echo 'Finished building: $@'
	@echo ' '

gr_Appli.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "gr_Appli.hex"
	@echo 'Finished building: $@'
	@echo ' '

gr_Appli.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "gr_Appli.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout gr_Appli.bin gr_Appli.elf gr_Appli.hex gr_Appli.list gr_Appli.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
