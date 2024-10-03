#include "contiki.h"
#include "coap-engine.h"
#include "dev/leds.h"

#include <string.h>

#include "senml-json.h"
#include "printing_floats.h"

#include "cJSON.h" 

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP


float Ia;
float Ib;
float Ic;

float Va;
float Vb;
float Vc;

int type_of_fault;


static senml_payload payload;
static senml_measurement measurements[7];

static void res_event_handler(void);
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* This resource has been introduced additionally because it is strictly correlated to the transformer_state, thus it allows to alter the values of current and voltage in a certain amount of variation.*/
RESOURCE(res_transformer_settings,
         "title=\"transformer_settings\"; PUT; ct=\"application/json\"; rt=\"Control_Transformer_State\";",
         NULL,
         NULL,
         res_put_handler,
         NULL);

        
EVENT_RESOURCE(res_transformer_state_obs,
         "title=\"transformer_state_obs\"; GET; rt=\"Transformer_state\"; ct=\"senml+json\"; if=\"Sensor\"; obs",
         res_get_handler,
         NULL,
         NULL,
         NULL, 
	 res_event_handler);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	  char *string_payload=NULL;
	  int length =0;
	  
	  //senml_payload *payload= (senml_payload*) malloc (sizeof(senml_payload));
	  //senml_measurement measurements[7];
	  
	  measurements[0].name="state";
	  measurements[0].unit="type_fault";
	  measurements[0].time=0;
	  measurements[0].type=SENML_TYPE_V;
	  measurements[0].value.float_value=type_of_fault;
	  
	  measurements[1].name="current_A";
	  measurements[1].unit="MA";
	  measurements[1].time=0;
	  measurements[1].type=SENML_TYPE_V;
	  measurements[1].value.float_value=Ia;
	  
	  measurements[2].name="current_B";
	  measurements[2].unit="MA";
	  measurements[2].time=0;
	  measurements[2].type=SENML_TYPE_V;
	  measurements[2].value.float_value=Ib;
	  
	  measurements[3].name="current_C";
	  measurements[3].unit="MA";
	  measurements[3].time=0;
	  measurements[3].type=SENML_TYPE_V;
	  measurements[3].value.float_value=Ic;
	  
	  measurements[4].name="voltage_A";
	  measurements[4].unit="V";
	  measurements[4].time=0;
	  measurements[4].type=SENML_TYPE_V;
	  measurements[4].value.float_value=Va;
	  
	  measurements[5].name="voltage_B";
	  measurements[5].unit="MV";
	  measurements[5].time=0;
	  measurements[5].type=SENML_TYPE_V;
	  measurements[5].value.float_value=Vb;
	  
	  measurements[6].name="voltage_C";
	  measurements[6].unit="MV";
	  measurements[6].time=0;
	  measurements[6].type=SENML_TYPE_V;
	  measurements[6].value.float_value=Vc;
	  
	  payload.base_time=0;
	  payload.base_unit=NULL;
	  payload.nr_measurments=7;
	  payload.measurements=measurements;
	 
	  create_senml_payload(&payload,&string_payload);
	  length = strlen(string_payload);
	 
	  memcpy(buffer, string_payload, length);

  	  //LOG_DBG(" Sending the payload: %s\n", buffer);
  	   
	  coap_set_header_etag(response, (uint8_t *)&length, 1);
	  coap_set_header_content_format(response, APPLICATION_JSON);
	  coap_set_payload(response, buffer, length);
	  
	  if (string_payload != NULL) {
        	free(string_payload);
        	string_payload = NULL;
    	  }
}




static void res_event_handler(void)
{
   // Notify all the observers
   // Before sending the notification the handler
   // associated with the GET methods is called
   coap_notify_observers(&res_transformer_state_obs);
}


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
	
	cJSON *ia = cJSON_GetObjectItem(json, "ia");
	cJSON *ib = cJSON_GetObjectItem(json, "ib");
	cJSON *ic = cJSON_GetObjectItem(json, "ic");
	
	cJSON *va = cJSON_GetObjectItem(json, "va");
	cJSON *vb = cJSON_GetObjectItem(json, "vb");
	cJSON *vc = cJSON_GetObjectItem(json, "vc");
	
	if (ia!=NULL && cJSON_IsNumber(ia)){
		Ia=Ia+ia->valuedouble;
	}
	
	if (ib!=NULL && cJSON_IsNumber(ib)){
		Ib=Ib+ib->valuedouble;
	}
	
	if (ic!=NULL && cJSON_IsNumber(ic)){
		Ic=Ic+ic->valuedouble;
	}
	
	if (va!=NULL && cJSON_IsNumber(va)){
		Va=Va+va->valuedouble;
	}
	
	if (vb!=NULL && cJSON_IsNumber(vb)){
		Vb=Vb+vb->valuedouble;
	}
	
	if (vc!=NULL && cJSON_IsNumber(vc)){
		Vc=Vc+vc->valuedouble;
	}
	
	coap_set_status_code(response, CHANGED_2_04);

	if (json!=NULL){
		cJSON_Delete(json);
	}
  	
 }

