#include <float.h>
#include <math.h>
#include <stdio.h>

#include "project.h"

#define R_SENSE (0.62)
#define Ka      (0.3466)   // mv/min^-1
#define Kt      (0.00331)  // mN*m/mA
#define Vcc     (5.0)      // volts

void lcd_init(void) {
    LCD_Char_1_Start();

    LCD_Char_1_Position(0, 0);
    LCD_Char_1_PrintString("                    ");

    LCD_Char_1_Position(1, 0);
    LCD_Char_1_PrintString("                    ");

    LCD_Char_1_Position(2, 0);
    LCD_Char_1_PrintString("                    ");

    LCD_Char_1_Position(3, 0);
    LCD_Char_1_PrintString("                    ");
}

void lcd_print_line_str(uint8_t line, char8* str) {
    // blank line
    LCD_Char_1_Position(line, 0);
    LCD_Char_1_PrintString("                    ");

    // print line
    LCD_Char_1_Position(line, 0);
    LCD_Char_1_PrintString(str);
}

void lcd_print_line_str_float(uint8_t line, char8* str_in, float number) {
    int32_t num_whole = (int32_t)number;
    int32_t num_fractional = (int32_t)(number * 100) % 100;
    if(num_fractional < 0) {
        num_fractional = num_fractional * -1;
    }

    char8 str[21] = {0};
    sprintf(str, "%s %ld.%02ld", str_in, num_whole, num_fractional);

    // blank line
    LCD_Char_1_Position(line, 0);
    LCD_Char_1_PrintString("                    ");

    // print line
    LCD_Char_1_Position(line, 0);
    LCD_Char_1_PrintString(str);
}

int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */

    // initialize peripherals
    Motor_emf_adc_Start();
    Rsense_adc_Start();
    lcd_init();

    lcd_print_line_str(0, "Motor speed");

    motor_enable_Write(1);

    uint32_t loop_count = 0;
    while(1) {
        loop_count++;

        if(loop_count == 300) {
            // read current sense resistor and display torque
            Rsense_adc_StartConvert();
            Rsense_adc_IsEndConversion(Rsense_adc_WAIT_FOR_RESULT);
            Rsense_adc_StopConvert();
            int16 Rsense_adc_count = Rsense_adc_GetResult16();
            int16 Rsense_mV = Rsense_adc_CountsTo_mVolts(Rsense_adc_count);
            float current_mA = Rsense_mV / R_SENSE;
            float torque_mNm = current_mA * Kt;
            lcd_print_line_str_float(3, "Torque mNm:", torque_mNm);
        }

        if(loop_count == 2000) {
            // read motor back emf and display speed
            int16 max_emf_reading = 0;
            motor_enable_Write(0);
            CyDelayUs(300);

            // sample several times and save the max reading
            for(uint32_t i = 0; i < 700; ++i) {
                Motor_emf_adc_StartConvert();
                Motor_emf_adc_IsEndConversion(Motor_emf_adc_WAIT_FOR_RESULT);
                int16 emf_reading_counts = Motor_emf_adc_GetResult16();
                Motor_emf_adc_StopConvert();

                if(emf_reading_counts > max_emf_reading) {
                    max_emf_reading = emf_reading_counts;
                }
            }

            motor_enable_Write(1);

            // convert back emf reading to speed
            float emf_V = Vcc - Motor_emf_adc_CountsTo_Volts(max_emf_reading);
            float emf_mV = emf_V * 1000;
            float speed_rpm = emf_mV / Ka;
            lcd_print_line_str_float(1, "back emf V:", emf_V);
            lcd_print_line_str_float(2, "speed RPM:", speed_rpm);
            loop_count = 0;
        }

        CyDelay(1);
    }
}
