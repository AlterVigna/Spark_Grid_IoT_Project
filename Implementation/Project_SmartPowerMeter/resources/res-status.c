#include "coap-engine.h"
#include "cJSON.h" 


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP


bool activated=false;


static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*This file exposes the status of the smart power meter, in order to allows the energy provider to enable/disable remotely the user building depending if he/she has payed the bill or not. */
 

RESOURCE(res_status,
         "title=\"status\"; PUT \";rt=\"Control_status\"; ct=\"application/json\";",
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
	
	// Read all the common attributes and populate the data structure
	cJSON *status = cJSON_GetObjectItem(json, "status");
	
	bool previousStatus=activated;

	if (status!=NULL){
		activated=cJSON_IsTrue(status);
		coap_set_status_code(response, CHANGED_2_04);
	}
	
	// Create a new JSON object to include the previous state
    	cJSON *response_json = cJSON_CreateObject();
    	cJSON_AddBoolToObject(response_json, "previous_status", previousStatus);  // Include previous state in response

    	// Convert the JSON object to a string
    	char *json_response = cJSON_PrintUnformatted(response_json);

    	// Set the response payload
    	coap_set_payload(response, json_response, strlen(json_response));

    	// Free memory
    	if (json!=NULL){
    		cJSON_Delete(json);
    	}
    	if (response_json!=NULL){
    		cJSON_Delete(response_json);
    	}
    	if (json_response!=NULL){
    		free(json_response);
    	}
	
}

