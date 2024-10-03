#include "math.h"
#include "random.h"

/**
 * Given an interval it generates a random number between min and max value.
 * @param min The minimun range in the interval.
 * @param max The maximum range in the interval.
 * @return A random number between min-max.
*/
float random_value_generation (float min, float max){	
	return min+(max-min)* ((float) random_rand() / RANDOM_RAND_MAX);
}

/**
 * Given an interval, a maximum possibile variation and a number it generates a random number in the given interval that has a maxium variation respect to the original number. 
 * @param min The minimun range in the interval.
 * @param max The maximum range in the interval.
 * @param max_var The maxium variation tollerated for the old_value passed.
 * @param original_number The maximum range in the interval.
 * @return A random number obtained by the original number plus/minus a max random variation respecting the range (min-max)
*/
float random_value_generation_gradual_variation(float min, float max,float max_var,float original_number){
	
	int choice=random_rand() % 2;
	float variation=max_var*((float) random_rand() / RANDOM_RAND_MAX);
	float newValue=0;
	
	// Safty check
	/*if (original_number>max){
		original_number=max;
	}
	
	if (original_number<min){
		original_number=min;
	}*/
	
	choice=(choice==0)?1:-1;
	newValue=original_number+(choice)*variation;
	newValue=(newValue>max)?original_number-variation:newValue;
	newValue=(newValue<min)?original_number+variation:newValue;
	
	return newValue;
}


/**
 * Given an array of number returns the index corresponding to the maximum value.
 * @param arr The array of numbers to find an index
 * @param n The lenght of the array
 * @return The integer corresponding to the value of the maximum value
*/
int find_max_index(float *arr, int n){
	int max_index=0, i;

	for (i = 1; i < n; i++) {
		if (arr[i] > arr[max_index]) {
		    max_index = i; 
		}
    	}
	return max_index; // Restituisci l'indice del valore massimo
}

