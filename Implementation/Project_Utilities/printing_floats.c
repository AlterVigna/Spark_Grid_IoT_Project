#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/**
 * This function has been created just for debug reasons on Nordic Dongle Device that by default avoid to print floting point numbers.
 * By default it is not used in order to avoid wasting energy.
 * @param number The floating point number to be printed.
 * @param str The container for the string where the string number will be placed.
 * @param precision How many numbers to consider after the decimal separator.
 */
void floatToString(float number, char *str, int precision) {
    // Handling of the sign
    if (number < 0) {
        *str++ = '-'; 
        number = -number;
    }

    // Converting the integer part in a string 
    int integerPart = (int)number;
    int index = 0;
    int integerPartCopy = integerPart;

    // Counts the number of digits
    do {
        index++;
        integerPartCopy /= 10;
    } while (integerPartCopy);

    str[index] = '\0'; // End of string

    // Fill the string with integer part 
    for (int i = index - 1; i >= 0; i--) {
        str[i] = (integerPart % 10) + '0';
        integerPart /= 10;
    }

    str += index; // Position the pointer at the end of integer part

    // Add decimal separetor
    *str++ = '.';

    // Handle the decimal part
    float decimalPart = number - (int)number;
    for (int i = 0; i < precision; i++) {
        decimalPart *= 10;
        int digit = (int)decimalPart;
        *str++ = digit + '0';
        decimalPart -= digit;
    }

    *str = '\0'; 
}



/**
 * This function standardize how the output (LOGS) of the smart power meter sensor should be.
 * @param voltage is the numeric value of the voltage provided to the building
 * @param current_consumed is the numeric value of the current consumed by the building
 * @param current_produced is the numeric value of the current produced by the building
 * @param power_factor is the numeric value of the loads attacched to the building 
 * @param instant_power is the numberic value of the global power from/into the building (negative if power is produced, positive if consumed)
*/
void print_smart_power_meter_sensing_measurement(float voltage, float current_consumed, float current_produced, float power_factor,float instant_power,int loads_attacched){
	
	#ifdef COOJA
            LOG_DBG("Voltage: %.2fV -- Current Consumed: %.2fA -- Current Produced: %.2fA -- Power Factor: %.2f \n", voltage,current_consumed,current_produced,power_factor);
            LOG_DBG("No. Loads attacched: %d \n",loads_attacched);
            LOG_DBG("Instant Power: %.2f W\n", instant_power);
        #else
            
	    char string_voltage [10];
	    char string_current_produced[10];
	    char string_current_consumed[10];
            char string_power_factor[10];
	    char string_instant_power [10];

	    floatToString(voltage,string_voltage,2);
	    floatToString(current_consumed,string_current_consumed,2);
	    floatToString(current_produced,string_current_produced,2);
	    floatToString(power_factor,string_power_factor,2);
	    floatToString(instant_power,string_instant_power,2);

	    LOG_DBG("Voltage: %sV -- Current Consumed: %sA -- Current Produced: %sA -- Power Factor: %s \n", string_voltage,string_current_consumed,string_current_produced,string_power_factor);
	    LOG_DBG("No. Loads plugged: %d \n",loads_attacched);
	    LOG_DBG("Instant Power: %s W\n\n", string_instant_power);
        #endif
}



/**
 * This function standardize how the output (LOGS) of the smart transformer sensor should be.
 * @param Ia is the current measured on phase A
 * @param Ib is the current measured on phase B
 * @param Ic is the current measured on phase C
 * @param Va is the voltage measured on phase A
 * @param Vb is the voltage measured on phase B
 * @param Vc is the voltage measured on phase C
*/
void print_smart_transformer_sensing_measurement(float Ia,float Ib,float Ic, float Va, float Vb, float Vc){

	#ifdef COOJA
            LOG_DBG("Ia: %.2fMA -- Ib: %.2fMA -- Ic: %.2fMA\nVa: %.2fMV -- Vb: %.2fMV -- Vc: %.2fMV \n", Ia,Ib,Ic,Va,Vb,Vc);
        #else
 
	    char string_Ia[10];
	    char string_Ib[10];
	    char string_Ic[10];
	    char string_Va[10];
	    char string_Vb[10];
	    char string_Vc[10];
	
	    floatToString(Ia,string_Ia,2);
	    floatToString(Ib,string_Ib,2);
	    floatToString(Ic,string_Ic,2);
	    floatToString(Va,string_Va,2);
	    floatToString(Vb,string_Vb,2);
            floatToString(Vc,string_Vc,2);
	
	    LOG_DBG("Ia: %sMA -- Ib: %sMA -- Ic: %sMA -- Va: %sMV -- Vb: %sMV -- Vc: %sMV \n", string_Ia,string_Ib,string_Ic,string_Va,string_Vb,string_Vc);
        #endif
}




/**
 * Standard way to print the probabilities for ML task. It's know a-priori that the number of classes are 5.
 * @param outputs The vector of probability obtained after inferring
 * @param predicted_class The class having the highest probability
*/
void print_probabilities(float *outputs,int predicted_class){

     LOG_DBG("Fault-Type: %i -- Prob.: (%i, %i, %i, %i, %i)\n\n\n", predicted_class, (int) ((float) outputs[0]*100), (int) ((float) outputs[1]*100), (int) ((float)outputs[2]*100), (int)((float)outputs[3]*100), (int)((float)outputs[4]*100));
    

}


