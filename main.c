#include "project.h"

int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */

    // initialize peripherals
    Rsense_adc_Start();
    Rsense_adc_StartConvert();  // let this run freely

    // Motor_emf_adc_Start(); // should this be started here or during a reading?

    // Rsense_adc example reading
    int32 Rsense_reading_counts = Rsense_adc_GetResult32();
    int16 Rsense_mv = Rsense_adc_CountsTo_mVolts(Rsense_reading_counts);

    // Motor_emf_adc example reading
    motor_enable_Write(0);
    CyDelay(1);             // delay for the fet to turn off
    Motor_emf_adc_Start();  // is the first reading ok to use?
    Motor_emf_adc_IsEndConversion(Motor_emf_adc_WAIT_FOR_RESULT);
    motor_enable_Write(1);
    int16 emf_reading_counts = Motor_emf_adc_GetResult16();
    int16 emf_mv = Motor_emf_adc_CountsTo_mVolts(emf_reading_counts);
    Motor_emf_adc_Stop();

    motor_enable_Write(1);
    CyDelay(500);
    motor_enable_Write(0);

    while(1) {
        /* Place your application code here. */
    }
}
