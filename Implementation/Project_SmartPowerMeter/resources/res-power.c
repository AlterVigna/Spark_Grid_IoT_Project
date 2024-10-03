#include "coap-engine.h"
#include "senml-json.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP



float instant_power=0;


static senml_payload payload;
static senml_measurement measurements[1];

static void res_event_handler(void);
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);



/* This file contains the declaration of two resources: the first (standard) to provide the current power (instantaneously); the second allows to observe the resorurce and be notified once it is modified (under certain criterion). */
RESOURCE(res_power,
         "title=\"power\", GET \";rt=\"Power\"; ct=\"senml+json\";",
         res_get_handler,
         NULL,
         NULL,
         NULL);
         
         
EVENT_RESOURCE(res_obs,
         "title=\"power_obs\" GET \";rt=\"Power\"; ct=\"senml+json\";obs",
         res_get_handler,
         NULL,
         NULL,
         NULL, 
	 res_event_handler);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

 
  //printf("Entro la get_handler\n");
  //counter++;
  //printf("NUMERO DI COUNTER : %d \n",counter);
  
  char *string_payload=NULL;
  int length =0;
  
  measurements[0].name="power";
  measurements[0].unit=NULL;
  measurements[0].time=0;
  measurements[0].type=SENML_TYPE_V;
  measurements[0].value.float_value=instant_power;
 
  payload.base_time=0;
  payload.base_unit="W";
  payload.nr_measurments=1;
  payload.measurements=measurements;
 
 
  create_senml_payload(&payload,&string_payload);
  length = strlen(string_payload);
  
  
  memcpy(buffer, string_payload, length);

  
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
    coap_notify_observers(&res_obs);
}

