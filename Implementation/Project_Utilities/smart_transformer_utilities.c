#include <stdio.h>
#include <stdlib.h>
#include "math_utilities.h"
#include "smart_transformer_utilities.h"

#include "sys/log.h"

#include "cJSON.h" 

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/**
* This function aims to simulate a random fix of the smart relay during the execution of the normal activity of the smart transformer.
* The goal is to gradually move the values in a safety range. This range was observed and obtained from the data used the analysis and development of ML task.
* @param type is the type of fault (class) recognized by the neural network
* @param Ia is the current measured on phase A
* @param Ib is the current measured on phase B
* @param Ic is the current measured on phase C
* @param Va is the voltage measured on phase A
* @param Vb is the voltage measured on phase B
* @param Vc is the voltage measured on phase C
*/
void repairing_fault(int type, float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc){
	
	float randomness; // variable used to mimic a generic randomness.

// Simulation of reparing current values (Ia,Ib,Ic).

	// Simulation repairing Ia
	if (*Ia<RANGE_MIN_IA || *Ia>RANGE_MAX_IA){
		*Ia=(*Ia<RANGE_MIN_IA)?*Ia+random_value_generation(MIN_VAR_IA,MAX_VAR_IA):*Ia;
		*Ia=(*Ia>RANGE_MAX_IA)?*Ia-random_value_generation(MIN_VAR_IA,MAX_VAR_IA):*Ia;
	}
	else {
		// In this case Ia is already in the safe range so it just simulate a random variation in the safe range.
	    	*Ia=random_value_generation_gradual_variation(RANGE_MIN_IA,RANGE_MAX_IA,VAR_IA,*Ia);
	}
	
	// Simulation repairing Ib
    	if (*Ib<RANGE_MIN_IB || *Ib>RANGE_MAX_IB){
    	
    		if (*Ib<RANGE_MIN_IB){
    			
    			// Depending on the type of fault there can be different action to implement.
    			switch (type){
    				case 1:
    					*Ib=*Ib+random_value_generation(MIN_VAR_IB_TYPE_1,MAX_VAR_IB_TYPE_1);
    				break;
    				
    				case 2: 
    					*Ib=*Ib+random_value_generation(MIN_VAR_IB_TYPE_2,MAX_VAR_IB_TYPE_2);
    				break;
    				
    				case 3:
    					randomness=random_value_generation(MIN_VAR_IB_TYPE_3,MAX_VAR_IB_TYPE_3);
    					*Ib=*Ib+randomness;
    					// If the Ib is near to the safe range, modify Ic in order to mimic a gradual descending also in the type of fault.
    					if (*Ib>-SAFETY_CHECK_IB){
    						randomness=random_value_generation(MIN_SUM_RANGE_VOLT_TYPE_0,MAX_SUM_RANGE_VOLT_TYPE_0);
    						*Ic=randomness-*Ia;
    					}
    					else{
    						// Modify Ic as same quantity as Ib.
    						*Ic=(*Ic>0)?*Ic-randomness:*Ic+randomness;
    					}	
    				break;
    			}

    		}
    		
    		// Specular behaviour but for the upper range.
    		if (*Ib>RANGE_MAX_IB){
    		
    			switch (type){
    				case 1:
    					*Ib=*Ib-random_value_generation(MIN_VAR_IB_TYPE_1,MAX_VAR_IB_TYPE_1);
    				break;
    				
    				case 2:
    					*Ib=*Ib-random_value_generation(MIN_VAR_IB_TYPE_2,MAX_VAR_IB_TYPE_2);
    				break;
    				
    				case 3:
    					randomness=random_value_generation(MIN_VAR_IB_TYPE_3,MAX_VAR_IB_TYPE_3);
    					*Ib=*Ib-randomness;
    				
	    				// If the Ib is near to the safe range, modify Ic in order to mimic a gradual descending also in the type of fault.
	    				if (*Ib>SAFETY_CHECK_IB){
	    					randomness=random_value_generation(MIN_SUM_RANGE_VOLT_TYPE_0,MAX_SUM_RANGE_VOLT_TYPE_0);
	    					*Ic=randomness-*Ia;
	    				}
	    				else{
	    					// Modify Ic as same quantity as Ib.
	    					*Ic=(*Ic>0)?*Ic-randomness:*Ic+randomness;
	    				}	
    				break;
    			}
    		}
    	}else {
    		// In this case Ib is already in the safe range so it just simulate a random variation in the safe range.
    		*Ib=random_value_generation_gradual_variation(RANGE_MIN_IB,RANGE_MAX_IB,VAR_IB,*Ib);
    	}
	    	
    	// If both Current Ia and Ib are in the safety ranges compute C as in Type-0.
    	if ((*Ia>=RANGE_MIN_IA && *Ia<=RANGE_MAX_IA) && (*Ib>=RANGE_MIN_IB && *Ib<=RANGE_MAX_IB)){
    		randomness=random_value_generation(MIN_SUM_RANGE_CURR_TYPE_0,MAX_SUM_RANGE_CURR_TYPE_0);
		*Ic=randomness-*Ia-*Ib;
    	}
    	else {
    		// Otherwise update with random value in a small range
    		*Ic=random_value_generation_gradual_variation(*Ic-1,*Ic+1,0.1,*Ic);
    	}
	    	
	    	 	
// Simulation reparing voltage values (Va,Vb,Vc).
	    
	    
    	if (*Va<RANGE_MIN_VA || *Va>RANGE_MAX_VA){
    		// Simulating adjust range for Va
	    	*Va=(*Va<RANGE_MIN_VA)?*Va+MAX_RAND_VAR_VOLT:*Va;
	    	*Va=(*Va>RANGE_MAX_VA)?*Va-MAX_RAND_VAR_VOLT:*Va;
    	
    	}else {
    		// In this case Va is already in the safe range so it just simulate a random variation in the safe range.
    		*Va=random_value_generation_gradual_variation(RANGE_MIN_VA,RANGE_MAX_VA,VAR_VA,*Va);
    	}
    	
    	if (*Vb<RANGE_MIN_VB || *Vb>RANGE_MAX_VB){
   		// Simulating adjust range for Vb
    		*Vb=(*Vb<RANGE_MIN_VB)?*Vb+MAX_RAND_VAR_VOLT:*Vb;
    		*Vb=(*Vb>RANGE_MAX_VB)?*Vb-MAX_RAND_VAR_VOLT:*Vb;
    		
    	}else {
    		// In this case Vb is already in the safe range so it just simulate a random variation in the safe range.
    		*Vb=random_value_generation_gradual_variation(RANGE_MIN_VB,RANGE_MAX_VB,VAR_VB,*Vb);
    	}
	 
	// Additional variation to voltage in case of type 3.   	
	if (type==3){
		*Va=random_value_generation_gradual_variation(RANGE_MIN_VA,RANGE_MAX_VA,MAX_RAND_VAR_VOLT,*Va);
		*Vb=random_value_generation_gradual_variation(RANGE_MIN_VB,RANGE_MAX_VB,MAX_RAND_VAR_VOLT,*Vb);
	} 
	
		
	// If Current Va and Vb are in ranges compute VC as follows.
    	if ((*Va>=RANGE_MIN_VA && *Va<=RANGE_MAX_VA) && (*Vb>=RANGE_MIN_VB && *Vb<=RANGE_MAX_VB)){
    		randomness=random_value_generation(MIN_SUM_RANGE_VOLT_TYPE_0,MAX_SUM_RANGE_VOLT_TYPE_0);
		*Vc=randomness-*Va-*Vb;
    	}
    	else {
    		// Otherwise update with random value
    		*Vc=random_value_generation_gradual_variation(*Vc-RANGE_VC,*Vc+RANGE_VC,MIN_VAR_RANGE_VC,*Vc);
    	}
}


/**
 * This function is made to simulate random generation of values type-4 that requires an human intervation to solve.
 * @param Ia is the current measured on phase A
 * @param Ib is the current measured on phase B
 * @param Ic is the current measured on phase C
 * @param Va is the voltage measured on phase A
 * @param Vb is the voltage measured on phase B
 * @param Vc is the voltage measured on phase C
*/
void handling_type_4(float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc){

	*Ia=random_value_generation_gradual_variation(*Ia-MIN_VAR_IA,*Ia+MIN_VAR_IA,VAR_IA,*Ia);
	*Ib=random_value_generation_gradual_variation(*Ib-MIN_VAR_IA,*Ib+MAX_VAR_IA,VAR_IA,*Ib);
	*Ic=random_value_generation_gradual_variation(*Ic-MIN_VAR_IA,*Ic+MAX_VAR_IA,VAR_IA,*Ic);
	
	*Va=random_value_generation_gradual_variation(*Va-MAX_RAND_VAR_VOLT,*Va+MAX_RAND_VAR_VOLT,MAX_RAND_VAR_VOLT,*Va);
	*Vb=random_value_generation_gradual_variation(*Vb-MAX_RAND_VAR_VOLT,*Vb+MAX_RAND_VAR_VOLT,MAX_RAND_VAR_VOLT,*Vb);
	*Vc=random_value_generation_gradual_variation(*Vc-MAX_RAND_VAR_VOLT,*Vc+MAX_RAND_VAR_VOLT,MAX_RAND_VAR_VOLT,*Vc);
}


/**
 * This function is made to initialize the transformer currents and voltages with specific fixed values.
 * @param Ia is the current measured on phase A
 * @param Ib is the current measured on phase B
 * @param Ic is the current measured on phase C
 * @param Va is the voltage measured on phase A
 * @param Vb is the voltage measured on phase B
 * @param Vc is the voltage measured on phase C
*/
void generate_initial_transformer_values(float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc){
	*Ia=IA_FAULT_TYPE_0;
	*Ib=IB_FAULT_TYPE_0;
	*Ic=IC_FAULT_TYPE_0;
	*Va=VA_FAULT_TYPE_0;
	*Vb=VB_FAULT_TYPE_0;
	*Vc=VC_FAULT_TYPE_0;
}


/**
 * This function is made to simulate random generation of values type-0 that mimic the correct behaviour of the transformer.
 * @param Ia is the current measured on phase A
 * @param Ib is the current measured on phase B
 * @param Ic is the current measured on phase C
 * @param Va is the voltage measured on phase A
 * @param Vb is the voltage measured on phase B
 * @param Vc is the voltage measured on phase C
*/
void generate_correct_transformers_values(float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc){
	
	float randomness;
	// Handle of current
	*Ia=random_value_generation_gradual_variation(RANGE_MIN_IA,RANGE_MAX_IA,VAR_IA,*Ia);
	*Ib=random_value_generation_gradual_variation(RANGE_MIN_IB,RANGE_MAX_IB,VAR_IB,*Ib);
	randomness=random_value_generation(MIN_SUM_RANGE_CURR_TYPE_0,MAX_SUM_RANGE_CURR_TYPE_0);
	*Ic=randomness-*Ia-*Ib;
		
	// Handle of voltage
		
	*Va=random_value_generation_gradual_variation(RANGE_MIN_VA,RANGE_MAX_VA,VAR_VA,*Va);
	*Vb=random_value_generation_gradual_variation(RANGE_MIN_VB,RANGE_MAX_VB,VAR_VB,*Vb);
	randomness=random_value_generation(MIN_SUM_RANGE_VOLT_TYPE_0,MAX_SUM_RANGE_VOLT_TYPE_0);
	*Vc=randomness-*Va-*Vb;

}
/**
 * This function is made to simulate the generation of a specific type of fault.
 * The specific values are taken from test-dataset.
 * @param Ia is the current measured on phase A
 * @param Ib is the current measured on phase B
 * @param Ic is the current measured on phase C
 * @param Va is the voltage measured on phase A
 * @param Vb is the voltage measured on phase B
 * @param Vc is the voltage measured on phase C
*/
void generate_transformer_fault(int type_of_fault, float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc){
	switch ( type_of_fault )
	{	    
	
		case FAULT_TYPE_0:
			//Do nothing
			break;
			
		case FAULT_TYPE_1:
			*Ia=IA_FAULT_TYPE_1;
			*Ib=IB_FAULT_TYPE_1;
			*Ic=IC_FAULT_TYPE_1;
					
			*Va=VA_FAULT_TYPE_1;
			*Vb=VB_FAULT_TYPE_1;
			*Vc=VC_FAULT_TYPE_1;
			break;
		
		case FAULT_TYPE_2:
			    				
		    	*Ia=IA_FAULT_TYPE_2;
			*Ib=IB_FAULT_TYPE_2;
			*Ic=IC_FAULT_TYPE_2;
					
			*Va=VA_FAULT_TYPE_2;
			*Vb=VB_FAULT_TYPE_2;
			*Vc=VC_FAULT_TYPE_2;
			break;
			
		case FAULT_TYPE_3:
		
			*Ia=IA_FAULT_TYPE_3;
			*Ib=IB_FAULT_TYPE_3;
			*Ic=IC_FAULT_TYPE_3;
					
			*Va=VA_FAULT_TYPE_3;
			*Vb=VB_FAULT_TYPE_3;
			*Vc=VC_FAULT_TYPE_3;
			break;
				
		case FAULT_TYPE_4:
		
			*Ia=IA_FAULT_TYPE_4;
			*Ib=IB_FAULT_TYPE_4;
			*Ic=IC_FAULT_TYPE_4;
					
			*Va=VA_FAULT_TYPE_4;
			*Vb=VB_FAULT_TYPE_4;
			*Vc=VC_FAULT_TYPE_4;
			break;
				
		default:
			// It should not pass here
			LOG_DBG("It should not pass here!");
	}

}



/**
 * This function aims to create a string representing the message for changing the state of an activation/disactivation of a building (remotely).
 * @param json_string_payload the string to be populated as json payload
 * @param state a boolean value indicating the new state to be changed
 */
void create_msg_house_change_state(char **json_string_payload,bool state){
	
	cJSON *root = cJSON_CreateObject();
	cJSON_AddBoolToObject(root, "status", state);

	char *json_payload=cJSON_PrintUnformatted(root);
	printf("%s\n",json_payload);
	// Make sure to allocate memory for the json_string_payload
	
	if (*json_string_payload == NULL) {
		*json_string_payload = (char *)malloc(strlen(json_payload) + 1);
		if (*json_string_payload == NULL) {
			printf("Memory allocation failed!\n");
			free(json_payload);
			cJSON_Delete(root); // Clean up the JSON object
			return;
		}
	}
	
	
	// Copy the formatted JSON string into the provided string buffer
	strcpy(*json_string_payload, json_payload);

	// Free the temporary JSON string
	free(json_payload);
	
	// Clean up the root object to avoid memory leaks
	cJSON_Delete(root);
}


/**
 * This function aims to create a string representing the registration message of a smart trasformer to the main external server.
 * @param json_string_payload the string to be populated as json payload
 */
void create_msg_registration_st(char **json_string_payload){
	
	char base_name[BASE_NAME_MAX_LEN];
	
	cJSON *root = cJSON_CreateObject();
	create_base_name_attribute(base_name);
	cJSON_AddStringToObject(root, "full_name", base_name);
	// For now this value is fixed
	cJSON_AddStringToObject(root, "alias", "smart_transformer_1");
	cJSON_AddNumberToObject(root, "type", 2);
	
	char *json_payload=cJSON_PrintUnformatted(root);
	printf("%s\n",json_payload);
	
	// Make sure to allocate memory for the json_string_payload
	if (*json_string_payload == NULL) {
		*json_string_payload = (char *)malloc(strlen(json_payload) + 1);
		if (*json_string_payload == NULL) {
			printf("Memory allocation failed!\n");
			free(json_payload);
			cJSON_Delete(root); // Clean up the JSON object
			return;
		}
	}
	
	// Copy the formatted JSON string into the provided string buffer
	strcpy(*json_string_payload, json_payload);

	// Free the temporary JSON string
	free(json_payload);
	
	// Clean up the root object to avoid memory leaks
	cJSON_Delete(root);
}




