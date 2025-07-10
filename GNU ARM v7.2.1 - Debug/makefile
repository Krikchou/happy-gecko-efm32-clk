################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include service/subdir.mk
-include external_copied_files/subdir.mk
-include emlib/subdir.mk
-include Graphics/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32HG/subdir.mk
-include BSP/subdir.mk
-include .metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: happy-gecko-efm32-clk.axf

# Tool invocations
happy-gecko-efm32-clk.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "happy-gecko-efm32-clk.ld" -Xlinker --gc-sections -Xlinker -Map="happy-gecko-efm32-clk.map" --specs=nano.specs -o happy-gecko-efm32-clk.axf "./.metadata/.plugins/org.eclipse.cdt.make.core/specs.o" "./BSP/bsp_bcc.o" "./BSP/bsp_stk.o" "./CMSIS/EFM32HG/startup_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./Drivers/display.o" "./Drivers/displayls013b7dh03.o" "./Drivers/displaypalemlib.o" "./Drivers/i2cspm.o" "./Drivers/si7013.o" "./Drivers/udelay.o" "./Graphics/dmd_display.o" "./Graphics/glib.o" "./Graphics/glib_bitmap.o" "./Graphics/glib_circle.o" "./Graphics/glib_font_narrow_6x8.o" "./Graphics/glib_font_normal_8x8.o" "./Graphics/glib_line.o" "./Graphics/glib_rectangle.o" "./Graphics/glib_string.o" "./emlib/em_adc.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_i2c.o" "./emlib/em_leuart.o" "./emlib/em_rtc.o" "./emlib/em_usart.o" "./external_copied_files/capsense.o" "./external_copied_files/em_acmp.o" "./service/sl_sleeptimer.o" "./service/sl_sleeptimer_hal_rtc.o" "./src/7segment_font.o" "./src/clock_control.o" "./src/font_custom.o" "./src/graphics.o" "./src/humitemp.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: happy-gecko-efm32-clk.hex'
	arm-none-eabi-objcopy -O ihex "happy-gecko-efm32-clk.axf" "happy-gecko-efm32-clk.hex"
	@echo ' '

	@echo 'Building bin file: happy-gecko-efm32-clk.bin'
	arm-none-eabi-objcopy -O binary "happy-gecko-efm32-clk.axf" "happy-gecko-efm32-clk.bin"
	@echo ' '

	@echo 'Building s37 file: happy-gecko-efm32-clk.s37'
	arm-none-eabi-objcopy -O srec "happy-gecko-efm32-clk.axf" "happy-gecko-efm32-clk.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "happy-gecko-efm32-clk.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) happy-gecko-efm32-clk.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
