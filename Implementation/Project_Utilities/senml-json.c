#include "senml-json.h"
#include "os/net/linkaddr.h"
#include "cJSON.h" 
#include "string.h"

/**
 * This method is used to create a string in a proper format containing the MAC address of the device who's sending a msg
 * @param base_name the attribute that will be updated with the standard format containing the MAC address.
 */
void create_base_name_attribute(char *base_name){

	snprintf(base_name, BASE_NAME_MAX_LEN, "urn:dev:mac:%02X%02X%02X%02X%02X%02X%02X%02X:", 
	     linkaddr_node_addr.u8[0], 
	     linkaddr_node_addr.u8[1], 
	     linkaddr_node_addr.u8[2],
	     linkaddr_node_addr.u8[3],
	     linkaddr_node_addr.u8[4], 
	     linkaddr_node_addr.u8[5], 
	     linkaddr_node_addr.u8[6],
	     linkaddr_node_addr.u8[7]);
	     
	//printf("base_name = %s\n",base_name);
}


/**
 * This function is used to create a JSON payload following the standard of SenML.
 * @param payload is the populated datastructure used to create a JSON (string) representation of the payload
 * @param json_string_payload is the actual string used to represent the entire payload of the message to be send.
 */
void create_senml_payload(senml_payload *payload,char **json_string_payload){

	// This obj. is the main container of all the information.
	cJSON *root = cJSON_CreateObject();

	create_base_name_attribute(payload->base_name);

	cJSON_AddStringToObject(root, "bn", payload->base_name);

	if (payload->base_time!=0){
		cJSON_AddNumberToObject(root, "bt", payload->base_time);
	}

	if (payload->base_unit!=NULL){
		cJSON_AddStringToObject(root, "bu", payload->base_unit);
	}
	cJSON_AddNumberToObject(root, "ver", 1);
	
	// This obj. represents the list of all the measurments executed by a sensor.
	cJSON *measurments = cJSON_CreateArray();
	
	for (int i=0; i<payload->nr_measurments;i++){

		cJSON *measurment = cJSON_CreateObject();

		if (payload->measurements[i].name!=NULL){
			cJSON_AddStringToObject(measurment, "n", payload->measurements[i].name);
		}
		if (payload->measurements[i].unit!=NULL){
			cJSON_AddStringToObject(measurment, "u", payload->measurements[i].unit);
		}
		
		// Handling different type of value variable, mutually exclusive.
		
		int int_value = 0;
		switch(payload->measurements[i].type){

			case (SENML_TYPE_V):  
				// Adopted trick to avoid to send in float format.
		    		int_value = (int)(payload->measurements[i].value.float_value*100);
				cJSON_AddNumberToObject(measurment, "v", int_value);
				break;
					
			case (SENML_TYPE_BV):
				cJSON_AddBoolToObject(measurment, "bv", payload->measurements[i].value.boolean_value);
				break;
				
			case (SENML_TYPE_SV):
				cJSON_AddStringToObject(measurment, "sv", payload->measurements[i].value.string_value);
				break;
				
			default:
				printf("Wrong type inserted. Do nothing");
		}

		if (payload->measurements[i].time!=0){
			cJSON_AddNumberToObject(measurment, "t", payload->measurements[i].time);
		}
		cJSON_AddItemToArray(measurments, measurment);
	}
	
	// Append the measurments to the main obj.
	cJSON_AddItemToObject(root, "e", measurments);

	
	// Print the resulting JSON
	//*json_string_payload = cJSON_Print(root); // Well-formatted way, waste of space		
	char *json_payload=cJSON_PrintUnformatted(root);
	
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
 * This function takes as input the JSON(string) of a message and populates the obj. payload to be used for next purposes.
 * @param json_string_payload the string to be interpret as incoming message
 * @param payload is the data structure to be populated and where the json information are decoded. 
 */
/*void parse_senml_payload(char *json_string_payload, senml_payload **payload){

	// Parse the JSON string into a cJSON object
    	cJSON *json = cJSON_Parse(json_string_payload);
    	
	if (json == NULL) {
		printf("Error parsing JSON!\n");
		return;
	}
	
	// Read all the common attributes and populate the data structure
	cJSON *base_name = cJSON_GetObjectItem(json, "bn");
	cJSON *base_time = cJSON_GetObjectItem(json, "bt");
	cJSON *base_unit = cJSON_GetObjectItem(json, "bu");
	cJSON *version = cJSON_GetObjectItem(json, "ver");
	
	cJSON *measurments = cJSON_GetObjectItem(json, "e");
	
	
	if (base_name!=NULL && cJSON_IsString(base_name) && (base_name->valuestring != NULL)) {
        	strcpy((*payload)->base_name, base_name->valuestring);
    	}
    	if (base_time!=NULL && cJSON_IsNumber(base_time)) {
        	(*payload)->base_time=base_time->valueint;
    	}
    	else {
    		(*payload)->base_time=0;
    	}
    	if (base_unit!=NULL && cJSON_IsString(base_unit) && (base_unit->valuestring != NULL)) {
    		int len=strlen(base_unit->valuestring);
    		(*payload)->base_unit = malloc (len* sizeof(char));
    		strcpy((*payload)->base_unit, base_unit->valuestring);
    	}
    	
	if (version!=NULL && cJSON_IsNumber(version)) {
        	(*payload)->version=version->valueint;
    	}
    
    	// Read all the attributes related to measurments and populate the data structure
    	if (cJSON_IsArray(measurments)) {
    		int array_size = cJSON_GetArraySize(measurments);
    		(*payload)->nr_measurments=array_size;
    		
    		senml_measurement *measurments_array = (senml_measurement*) malloc (array_size*sizeof(senml_measurement)); 
    		
    		(*payload)->measurements=measurments_array;
    		
    		for (int i = 0; i < array_size; i++) {
		    cJSON *measurement = cJSON_GetArrayItem(measurments, i);
		    
		    cJSON *name = cJSON_GetObjectItem(measurement, "n");
		    cJSON *unit = cJSON_GetObjectItem(measurement, "u");
		    
		    // Treat different kind of value attributes
		    cJSON *num_value = cJSON_GetObjectItem(measurement, "v");
		    cJSON *boolean_value = cJSON_GetObjectItem(measurement, "bv");
		    cJSON *string_value = cJSON_GetObjectItem(measurement, "sv");
		    
		    cJSON *time= cJSON_GetObjectItem(measurement, "t");
		    
		    if (name!=NULL && cJSON_IsString(name) && (name->valuestring != NULL)) {
		    	int len=strlen(name->valuestring);
    			measurments_array[i].name = malloc (len* sizeof(char));
		    	strcpy(measurments_array[i].name, name->valuestring);
		    }
		    else {
		    	measurments_array[i].name=NULL;
		    }
		   
		    if (unit!=NULL && cJSON_IsString(unit) && (unit->valuestring != NULL)) {
		    	int len=strlen(unit->valuestring);
		    	measurments_array[i].unit=malloc (len* sizeof(char));
		    	strcpy(measurments_array[i].unit, unit->valuestring);
		    }
		    else{
		    	measurments_array[i].unit=NULL;
		    }
		    
		    if (num_value!=NULL && (cJSON_IsNumber(num_value))) {
		    	// Adopted trick to avoid to re-trasform in float format.
		    	float float_value = (float)num_value->valueint / 100;
		    	measurments_array[i].type=SENML_TYPE_V;
		    	measurments_array[i].value.float_value=float_value;
		    }
		     
		    if (boolean_value!=NULL && cJSON_IsBool(boolean_value)){
		    	measurments_array[i].type=SENML_TYPE_BV;
		    	measurments_array[i].value.boolean_value=cJSON_IsTrue(boolean_value)?true:false;
		    	
		    }
		    if (string_value!=NULL && cJSON_IsString(string_value) && (string_value->valuestring != NULL)) {
		    	measurments_array[i].type=SENML_TYPE_SV;
		    	
		    	int len=strlen(string_value->valuestring);
		    	measurments_array[i].value.string_value=malloc (len* sizeof(char));
		    	strcpy(measurments_array[i].value.string_value, string_value->valuestring);
		    }
		    
		    if (cJSON_IsNumber(time)) {
		    	measurments_array[i].time=time->valueint;
		    }
		    else{
		    	measurments_array[i].time=0;
		    }
        	}
    	}
}*/
