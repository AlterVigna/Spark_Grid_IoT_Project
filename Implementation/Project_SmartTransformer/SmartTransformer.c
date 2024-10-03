#include "contiki.h"

#include "coap-engine.h"
#include "coap-blocking-api.h"

#include <stdio.h>
#include <string.h>

#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "sys/log.h"

#include "os/dev/leds.h"
#include "os/dev/button-hal.h"

#include "net/netstack.h"

#include "math_utilities.h"
#include "smart_transformer_utilities.h"
#include "smart_transformer_fault_detection.h"
#include "printing_floats.h"

#include "cJSON.h"
#include "global_constants.h"


// Internal paramters of the sensor

#define SENSING_PERIOD 2
#define MAX_SECONDS_TOLLERABLE_FAULT_4 10
#define MAX_ATTEMPTS 3
#define MAX_SECONDS_COUNTDOWN 5


// Resources exposed
extern coap_resource_t res_transformer_state_obs;
extern coap_resource_t res_transformer_settings;



// Server IP and resource path

#ifdef COOJA

#define SERVER_EP "coap://[fd00:0:0:0:202:2:2:2]:"

#else

#define SERVER_EP "coap://[fd00::f6ce:3667:db31:8653]:"

#endif


char *service_url="/status";


PROCESS(smartTransformer, "Smart Transformer");

AUTOSTART_PROCESSES(&smartTransformer);



// Timers

static struct ctimer ctimer_sensing;
static struct etimer et;
static int nr_of_seconds_fault_4=0;

static int seconds_passed_countdown=0;

static coap_endpoint_t server_ep;
static coap_message_t request[1];      /* This way the packet can be treated as pointer as usual. */



//Smart transformer sensor measurments used inside the resource
extern float Ia;
extern float Ib;
extern float Ic;
extern float Va;
extern float Vb;
extern float Vc;
extern int type_of_fault;


// Info used to decide if triggering the resource or not.
static int coap_request_pending = 0;
static bool previous_house_status=true;
static bool house_status_changed=false;
static bool reg_ok=false;









/**
 * This function is used as callback method when a msg is received from the COAP_BLOCKING_REQUEST.
 * It contains the code for handling the registration answer and initialize correctly the values of status and MAX_POWER of the smart power meter.
*/
void client_reg_handler(coap_message_t *response)
{
	if(response == NULL) {
		puts("Request timed out");
		return;
	}
	const char *payload = (char*)response->payload;
	LOG_DBG("client_reg_handler: Received the payload: %s\nResponse code %d \n", payload,response->code);

	if (response->code != COAP_RESPONSE_CODE_CREATED) {
		LOG_DBG("Registration failed. Response code: %d\n", response->code);
		return;
	}
	reg_ok=true;
}


/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void client_chunk_handler(coap_message_t *response)
{
	const uint8_t *chunk;
	if(response == NULL) {
		puts("Request timed out");
		return;
	}

	int len = coap_get_payload(response, &chunk);
	const char *payload = (char*)request->payload;
	LOG_DBG("client_chunk_handler: Received the payload: %s\n", payload);

	// Parse the JSON string into a cJSON object
	cJSON *json = cJSON_Parse(payload);
	if (json == NULL) {
		printf("Error parsing JSON!\n");
		coap_set_status_code(response, BAD_REQUEST_4_00);
		return;
	}

	// Read all the common attributes and populate the data structure
	cJSON *status = cJSON_GetObjectItem(json, "previous_status");
	if (status!=NULL && cJSON_IsBool(status)) {
		previous_house_status=cJSON_IsTrue(status)?true:false;
	}

	cJSON_Delete(json);
	printf("|%.*s", len, (char *)chunk);
	house_status_changed=true;
}


/**
 This function implements the logic on the actuator (smart relay) to adjust wrong values or mimic the correct behaviour of the transformer.
*/
static void change_status_of_actuator(int predicted_class) {

	type_of_fault=predicted_class;
	switch ( predicted_class )
	{
	case FAULT_TYPE_0:
		leds_off(LEDS_ALL);
#ifdef COOJA
		leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
		leds_on(LEDS_GREEN);
#endif
		generate_correct_transformers_values(&Ia, &Ib, &Ic, &Va, &Vb, &Vc);
		break;
		
	case FAULT_TYPE_1:
		leds_off(LEDS_ALL);

#ifdef COOJA
		leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));
#else
		leds_single_on(LEDS_YELLOW);
#endif

		LOG_DBG("Repairing Fault-1: \n");
		repairing_fault(1, &Ia, &Ib, &Ic, &Va, &Vb, &Vc);
		break;
		
	case FAULT_TYPE_2:
		leds_off(LEDS_ALL);

#ifdef COOJA
		leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));
#else
		leds_single_on(LEDS_YELLOW);
#endif

		LOG_DBG("Repairing Fault-2: \n");
		repairing_fault(2, &Ia, &Ib, &Ic, &Va, &Vb, &Vc);
		break;
		
	case FAULT_TYPE_3:

#ifdef COOJA

		// In Cooja the blue led does not exist, simulation only yellow on.
		leds_off(LEDS_ALL);
		leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));
#else
		// On real device blue blinking.
		leds_off(LEDS_GREEN | LEDS_RED);
		leds_single_off(LEDS_YELLOW);
		leds_toggle(LEDS_BLUE);
#endif

		LOG_DBG("Repairing Fault-3: \n");
		repairing_fault(3, &Ia, &Ib, &Ic, &Va, &Vb, &Vc);
		break;

	case FAULT_TYPE_4:
		leds_off(LEDS_ALL);

#ifdef COOJA
		leds_single_on(LEDS_RED);
#else
		leds_on(LEDS_RED);
#endif
		LOG_DBG("Fault-4: not repairable. Required disconnection of part of the grid. \n");
		handling_type_4(&Ia, &Ib, &Ic, &Va, &Vb, &Vc);

		if (nr_of_seconds_fault_4>MAX_SECONDS_TOLLERABLE_FAULT_4) {

			// This due to the impossibility to handle here CoAP client request in non-protothreading function.
			if (coap_request_pending!=1) {
				LOG_DBG("\n\nSTART AUTOMATIC PROCEDURE FOR DISCONNECTION OF THE GRID\n\n");
				coap_request_pending=1;

				// Force wake-up of the smartTransformer process
				process_poll(&smartTransformer);
				nr_of_seconds_fault_4=0;
			}
		}
		nr_of_seconds_fault_4+=SENSING_PERIOD;
		break;
	default:
		printf("Do nothing!\n");
	}
}


/**
 * This callback function is used to simulate a sensing activity by the sensor.
 * The values of current (Ia,Ib,Ic) and voltage (Va,Vb,Vc) are sampled from the smart transformer by the sensor.
 * Their are passed throught the ML model and then the output is used to let take a decision to the actuator.
 */
static void execute_sensing(void *ptr) {
	// Current measurment
	print_smart_transformer_sensing_measurement(Ia,Ib,Ic,Va,Vb,Vc);
	
	// ML task: features and output vector
	float features[] = {Ia,Ib,Ic,Va,Vb,Vc};
	float outputs[5] = {0, 0, 0, 0, 0};
	int predicted_class=-1;


	eml_net_predict_proba(&smart_transformer_fault_detection, features, 6, outputs, 5);
	predicted_class=find_max_index(outputs,5);
	print_probabilities(outputs,predicted_class);
	res_transformer_state_obs.trigger();

	change_status_of_actuator(predicted_class);

	if (coap_request_pending!=1) {
		ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);
	}
}

/**
 * The aim of this function is to initialize in the proper range the starting values detected by the sensor.
 */
static void initialize_sensor_values() {
	generate_initial_transformer_values(&Ia, &Ib, &Ic, &Va, &Vb, &Vc);
}


PROCESS_THREAD(smartTransformer, ev, data) {

	PROCESS_BEGIN();

	initialize_sensor_values();
	printf("%p\n",eml_net_activation_function_strs);

	coap_activate_resource(&res_transformer_state_obs, "transformer_state_obs");
	coap_activate_resource(&res_transformer_settings,"transformer_settings");

	// BEFORE ACTUALLY STARTING
	// register smart transformer on the database in the cloud. "Acting as client"
	static int current_attempts=0;

	while (true) {

		LOG_DBG("Try to register smart transformer attempt: %d \n",current_attempts+1);
		coap_endpoint_parse(SERVER_REG_EP, strlen(SERVER_REG_EP), &server_ep);

		/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
		coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
		coap_set_header_uri_path(request, SERVICE_REG_URL);

		// Create payload info.
		char *json_payload=NULL;
		create_msg_registration_st(&json_payload);

		if (json_payload!=NULL) {
			coap_set_header_accept(request, APPLICATION_JSON);
			coap_set_payload(request, (uint8_t *)json_payload, strlen(json_payload));
			
			COAP_BLOCKING_REQUEST(&server_ep, request, client_reg_handler);

			current_attempts+=1;
			LOG_DBG("Received positive registration message from Server \n");

			if (reg_ok) {
				LOG_DBG("Registration of Smart Transformer 1 successfully: Smart Transformer starts working ..\n");
				break;
			}
			//free(json_payload);
		}
	}

	current_attempts=0;

	// Starting the sensing timer
	ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);
	while(1) {

		PROCESS_YIELD();
		// Handling the special case of disconnecting part of the grid.
		if(coap_request_pending==1) {
			while (current_attempts<MAX_ATTEMPTS) {
				LOG_DBG("Try to disconnect house 1 attempt: %d \n",current_attempts+1);
				coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
				
				/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
				coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
				coap_set_header_uri_path(request, service_url);
				// Create payload info.

				char *json_payload=NULL;
				create_msg_house_change_state(&json_payload,false);
				if (json_payload!=NULL) {
					coap_set_payload(request, (uint8_t *)json_payload, strlen(json_payload));
					COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
					current_attempts++;
					if (house_status_changed) {
						LOG_DBG("Received positive message from House 1 \n");
						break;
					}
					//free(json_payload);
				}
			}
			current_attempts=0;
			if (house_status_changed) {
				house_status_changed=false;
				LOG_DBG("Reconfiguration of the smart trnasformer in a safety mode! \n");
				initialize_sensor_values();
				leds_on(LEDS_ALL);
				
				while (seconds_passed_countdown<MAX_SECONDS_COUNTDOWN) {
					seconds_passed_countdown++;
					etimer_set(&et, CLOCK_SECOND);
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
					etimer_reset(&et);
				}
				
				leds_off(LEDS_ALL);
				seconds_passed_countdown=0;
				LOG_DBG("Reconfiguration successfully! \n");

				if (previous_house_status) {
					//Re-activate the house smart meter.
					while (current_attempts<MAX_ATTEMPTS) {
						LOG_DBG("Try to reconnect House_1 attempt: %d \n",current_attempts+1);
						coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

						/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
						coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
						coap_set_header_uri_path(request, service_url);

						// Create payload info.
						char *json_payload=NULL;
						create_msg_house_change_state(&json_payload,true);
						
						if (json_payload!=NULL) {
							coap_set_payload(request, (uint8_t *)json_payload, strlen(json_payload));
							COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
							current_attempts++;
							if (house_status_changed) {
								break;
							}
							free(json_payload);
						}
					}
					if (house_status_changed) {
						LOG_DBG("Received positive message from House_1: re-activated! \n");
					}
				}
				else {
					LOG_DBG("The House should stay disabled like its previous state! State become consistent!\n");
				}
			}

			coap_request_pending=0;
			ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);
		}

		// Handling button pressing: simulation of generation of faults.

		if (ev == button_hal_periodic_event) {
			ctimer_stop(&ctimer_sensing);
			type_of_fault++;
			if (type_of_fault>4) {
				type_of_fault=1;
			}
			LOG_DBG("Choose the event: Type of fault %d \n",type_of_fault);
		}

		if (ev == button_hal_release_event) {
			LOG_DBG("\n\nChoosed the event: Type of fault %d \n\n",type_of_fault);
			
			// Simulation of unexpected event
			generate_transformer_fault(type_of_fault, &Ia, &Ib, &Ic, &Va, &Vb, &Vc);
			ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);
		}
	}

	PROCESS_END();

}
