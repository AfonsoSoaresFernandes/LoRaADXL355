################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Adxl355/Src/adxl355.c \
../User/Adxl355/Src/adxl355_comm.c \
../User/Adxl355/Src/adxl355_data.c \
../User/Adxl355/Src/adxl355_util.c 

OBJS += \
./User/Adxl355/Src/adxl355.o \
./User/Adxl355/Src/adxl355_comm.o \
./User/Adxl355/Src/adxl355_data.o \
./User/Adxl355/Src/adxl355_util.o 

C_DEPS += \
./User/Adxl355/Src/adxl355.d \
./User/Adxl355/Src/adxl355_comm.d \
./User/Adxl355/Src/adxl355_data.d \
./User/Adxl355/Src/adxl355_util.d 


# Each subdirectory must supply rules for building sources it contributes
User/Adxl355/Src/%.o User/Adxl355/Src/%.su User/Adxl355/Src/%.cyclo: ../User/Adxl355/Src/%.c User/Adxl355/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WL55xx -c -I../Core/Inc -I../LoRaWAN/App -I../LoRaWAN/Target -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Utilities/trace/adv_trace -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/timer -I../Utilities/lpm/tiny_lpm -I../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Middlewares/Third_Party/LoRaWAN/Crypto -I../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../Middlewares/Third_Party/LoRaWAN/Mac -I../Middlewares/Third_Party/LoRaWAN/LmHandler -I../Middlewares/Third_Party/LoRaWAN/Utilities -I../Middlewares/Third_Party/SubGHz_Phy -I../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../Drivers/CMSIS/Include -I"C:/Users/afonsofernandes/STM32CubeIDE/Workspace_LORA/LoRa_Adxl2/Drivers/BSP" -I"C:/Users/afonsofernandes/STM32CubeIDE/Workspace_LORA/LoRa_Adxl2/User/Adxl355/Inc" -I"C:/Users/afonsofernandes/STM32CubeIDE/Workspace_LORA/LoRa_Adxl2/User/Flash_Interface" -I"C:/Users/afonsofernandes/STM32CubeIDE/Workspace_LORA/LoRa_Adxl2/User/Frame/Inc" -I"C:/Users/afonsofernandes/STM32CubeIDE/Workspace_LORA/LoRa_Adxl2/User/Utils/Inc" -I"C:/Users/afonsofernandes/STM32CubeIDE/Workspace_LORA/LoRa_Adxl2/User/LoRaManager" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-User-2f-Adxl355-2f-Src

clean-User-2f-Adxl355-2f-Src:
	-$(RM) ./User/Adxl355/Src/adxl355.cyclo ./User/Adxl355/Src/adxl355.d ./User/Adxl355/Src/adxl355.o ./User/Adxl355/Src/adxl355.su ./User/Adxl355/Src/adxl355_comm.cyclo ./User/Adxl355/Src/adxl355_comm.d ./User/Adxl355/Src/adxl355_comm.o ./User/Adxl355/Src/adxl355_comm.su ./User/Adxl355/Src/adxl355_data.cyclo ./User/Adxl355/Src/adxl355_data.d ./User/Adxl355/Src/adxl355_data.o ./User/Adxl355/Src/adxl355_data.su ./User/Adxl355/Src/adxl355_util.cyclo ./User/Adxl355/Src/adxl355_util.d ./User/Adxl355/Src/adxl355_util.o ./User/Adxl355/Src/adxl355_util.su

.PHONY: clean-User-2f-Adxl355-2f-Src

