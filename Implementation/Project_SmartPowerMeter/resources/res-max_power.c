#include "coap-engine.h"
#include "smart_power_meter_utilities.h"
#include "cJSON.h" 


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP


int MAX_POWER_ALLOWED;
float MAX_AMPERE_CONSUMABLE;


static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* This file allows to expose the max power resource, in case of the user wants to change his/her contract with the power provider. This parameter changes the range of working of the smart power meter  actuator.
*/

RESOURCE(res_max_power,
         "title=\"max_power\", PUT \";rt=\"Control_max_power\"; ct=\"application/json\";",
         NULL,
         NULL,
         res_put_handler,
         NULL);
         
        
static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

  	const char *payload = (char*)request->payload;
  	
  	LOG_DBG("res_put_handler: Received the payload: %s\n", payload);
  	
  	// Parse the JSON string into a cJSON object
  	cJSON *json = cJSON_Parse(payload);
    	
	if (json == NULL) {
		printf("Error parsing JSON!\n");
		coap_set_status_code(response, BAD_REQUEST_4_00);
		return;
	}
	
	cJSON *max_power = cJSON_GetObjectItem(json, "max_power");

	// Assign new value of Max_Power and recompute the value of the maximum ampere consumable.
	if (max_power!=NULL && cJSON_IsNumber(max_power)){
		MAX_POWER_ALLOWED=max_power->valueint;
		MAX_AMPERE_CONSUMABLE=compute_max_ampere_consumable(MAX_POWER_ALLOWED,MIN_VOLTAGE_PROVIDED,MIN_POWER_FACTOR);
		coap_set_status_code(response, CHANGED_2_04);
	}
	else{
		coap_set_status_code(response, BAD_REQUEST_4_00);
	}
	
	if (json != NULL) {
		cJSON_Delete(json);
	}

}

