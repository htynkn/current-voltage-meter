TARGET = current_voltage_meter

PREFIX ?= arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

BUILD_DIR = build

# CPU & Architecture
CPU = -mcpu=cortex-m23 -mthumb -mfloat-abi=soft

# C Sources
C_SOURCES = \
  Drivers/CMSIS/GD/GD32E23x/Source/system_gd32e23x.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_rcu.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_gpio.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_misc.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_syscfg.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_exti.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_fmc.c \
  Drivers/GD32E23x_standard_peripheral/Source/gd32e23x_timer.c \
  Bsp/bsp_systick.c \
  Bsp/bsp_i2c.c \
  Bsp/bsp_oled.c \
  Bsp/bsp_oled_font.c \
  Bsp/bsp_ina226.c \
  Bsp/bsp_key.c \
  App/app_meter.c \
  App/app_ui.c \
  App/gd32e23x_it.c \
  App/main.c

# ASM Sources
ASM_SOURCES = \
  Drivers/CMSIS/GD/GD32E23x/Source/GCC/startup_gd32e23x.S

# Includes
C_INCLUDES = \
  -IDrivers/CMSIS/GD/GD32E23x/Include \
  -IDrivers/CMSIS/Core \
  -IDrivers/GD32E23x_standard_peripheral/Include \
  -IBsp \
  -IApp

# Defines
C_DEFS = \
  -DGD32E230 \
  -DUSE_STDPERIPH_DRIVER

# Compiler Flags
CFLAGS = $(CPU) $(C_DEFS) $(C_INCLUDES) -O2 -Wall -fdata-sections -ffunction-sections -g

# Linker
LDSCRIPT = GD32E230C8T6_FLASH.ld
LDFLAGS = $(CPU) -T$(LDSCRIPT) -Wl,--gc-sections -specs=nano.specs -specs=nosys.specs -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) | $(BUILD_DIR)
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(CP) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

flash: all
	openocd -f interface/cmsis-dap.cfg -f target/gd32e23x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

.PHONY: all clean flash
