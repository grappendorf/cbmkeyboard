################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../cbmkeyboard.cpp 

OBJS += \
./cbmkeyboard.o 

CPP_DEPS += \
./cbmkeyboard.d 


# Each subdirectory must supply rules for building sources it contributes
cbmkeyboard.o: ../cbmkeyboard.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"/home/grappendorf/workspace-arduino/arduino-framework" -w -Os -ffunction-sections -fdata-sections -fno-exceptions -ffunction-sections -fdata-sections -mmcu=atmega168 -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"cbmkeyboard.d" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


