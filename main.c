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

void lcd_display_emf(float emf_V) {
    lcd_print_line_str_float(1, "back emf V:", emf_V);
}

void lcd_display_speed(float speed_rpm) {
    lcd_print_line_str_float(2, "Speed RPM: ", speed_rpm);
}

void lcd_display_torque(float torque_mNm) {
    lcd_print_line_str_float(3, "Torque mNm:", torque_mNm);
}

void motor_enable(void) {
    motor_enable_Write(1);
}

void motor_disable(void) {
    motor_enable_Write(0);
}

void motor_adc_init(void) {
    Motor_emf_adc_Start();
}

int16 motor_adc_read_mv(void) {
    Motor_emf_adc_StartConvert();
    Motor_emf_adc_IsEndConversion(Motor_emf_adc_WAIT_FOR_RESULT);
    int16 emf_reading_counts = Motor_emf_adc_GetResult16();
    Motor_emf_adc_StopConvert();
    int16 emf_mV = Motor_emf_adc_CountsTo_mVolts(emf_reading_counts);
    return emf_mV;
}

void current_adc_init(void) {
    Rsense_adc_Start();
}

int16 current_adc_read_mv(void) {
    Rsense_adc_StartConvert();
    Rsense_adc_IsEndConversion(Rsense_adc_WAIT_FOR_RESULT);
    Rsense_adc_StopConvert();
    int16 adc_reading = Rsense_adc_GetResult16();
    int16 Rsense_mV = Rsense_adc_CountsTo_mVolts(adc_reading);
    return Rsense_mV;
}

float calculate_torque(float Rsense_mV) {
    float current_mA = Rsense_mV / R_SENSE;
    float torque_mNm = current_mA * Kt;
    return torque_mNm;
}

float calculate_speed(float emf_mV) {
    float speed_rpm = emf_mV / Ka;
    return speed_rpm;
}

int main(void) {
    // initialize peripherals
    motor_adc_init();
    current_adc_init();
    lcd_init();

    lcd_print_line_str(0, "Motor speed");

    motor_enable();

    uint32_t loop_count = 0;
    while(1) {
        loop_count++;

        if(loop_count == 300) {
            // read current sense resistor and display torque
            float Rsense_mV = current_adc_read_mv();
            float torque_mNm = calculate_torque(Rsense_mV);
            lcd_display_torque(torque_mNm);
        }

        if(loop_count == 2000) {
            // read motor back emf and display speed
            int16 max_motor_reading_mv = 0;
            motor_disable();
            CyDelayUs(300);

            // sample several times and save the max reading
            for(uint32_t i = 0; i < 700; ++i) {
                int16 motor_reading_mv = motor_adc_read_mv();

                if(motor_reading_mv > max_motor_reading_mv) {
                    max_motor_reading_mv = motor_reading_mv;
                }
            }

            motor_enable();

            // convert back emf reading to speed
            float emf_mV = Vcc * 1000 - max_motor_reading_mv;
            float emf_V = emf_mV / 1000;
            float speed_rpm = calculate_speed(emf_mV);

            lcd_display_emf(emf_V);
            lcd_display_speed(speed_rpm);
            loop_count = 0;
        }

        CyDelay(1);
    }
}
