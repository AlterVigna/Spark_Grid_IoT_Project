#include "contiki.h"
#include "net/netstack.h"

#include "coap-engine.h"
#include "coap-blocking-api.h"

#include "os/dev/leds.h"
#include "os/dev/button-hal.h"
#include "os/net/linkaddr.h"

#include <stdio.h>
#include <string.h>

#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/log.h"

#include "math.h"
#include "math_utilities.h"
#include "smart_power_meter_utilities.h"
#include "printing_floats.h"
#include "senml-json.h"

#include "cJSON.h"
#include "global_constants.h"

// Internal paramters of the sensor

#define SENSING_PERIOD 2

#define NR_SECONDS_DISCONNECTION_ALL_THE_LOADS 5
#define MAX_SECONDS_COUNTDOWN 5
#define MAX_TIME_SENDING_SENSING 60


PROCESS(smartPowerMeter, "Smart Power Meter");

AUTOSTART_PROCESSES(&smartPowerMeter);

static coap_endpoint_t server_ep;
static coap_message_t request[1];      /* This way the packet can be treated as pointer as usual. */


// Sensor measurements

static float voltage=0;
static float current_consumed=0;
static float power_factor=0;
static float current_produced=0;

//Computed and exposed by the sensor
extern float instant_power;

// Internal status
extern bool activated;
extern int MAX_POWER_ALLOWED;
extern float MAX_AMPERE_CONSUMABLE;

// Timers
static struct ctimer ctimer_sensing;
static struct ctimer ctimer_restart_sensor;


// Resources exposed
extern coap_resource_t res_power;
extern coap_resource_t res_obs;
extern coap_resource_t res_status;
extern coap_resource_t res_max_power;


// Global utility variables
static int seconds_passed_countdown=0;
static bool max_power_consumption_achieved=false;
static int nr_loads_attacched=0;



// Info used to decide if triggering the resource or not.
static float last_instant_power_send=0;
static int nr_seconds_passed_last_send=0;
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

	// Parse the JSON string into a cJSON object
	cJSON *json = cJSON_Parse(payload);



	if (json == NULL) {
		printf("Error parsing JSON!\n");
		coap_set_status_code(response, BAD_REQUEST_4_00);
		return;
	}

	// Read the field of json and initialize the values consistensly.
	cJSON *status = cJSON_GetObjectItem(json, "status");
	if (status!=NULL && cJSON_IsBool(status)) {
		activated=cJSON_IsTrue(status)?true:false;
		printf("Inizialized status to: %d \n",activated);
	}

	cJSON *max_power = cJSON_GetObjectItem(json, "max_power");
	if (max_power!=NULL && (cJSON_IsNumber(max_power))) {
		printf("Inizialized max power to: %d \n",max_power->valueint);
		MAX_POWER_ALLOWED=max_power->valueint;
	}

	cJSON_Delete(json);
	reg_ok=true;
}


/**
 * This function implements the logic on the actuator linked to the Smart Home.
*/
static void change_status_of_actuator() {
	// The plant is deactivated by the power network manager thought a command received from outside.

	if (activated==false) {
		leds_off(LEDS_ALL);

#ifdef COOJA
		leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));
#else
		leds_single_on(LEDS_YELLOW);
#endif
	}

	else {

		// The building is producing more energy than the consumed.
		if (instant_power<0) {
			leds_off(LEDS_ALL);

#ifdef COOJA
			leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
#else
			leds_on(LEDS_GREEN);
#endif
		}
		else {

			// The building is consuming energy in the allowed safe range
			if (instant_power<(MAX_POWER_ALLOWED-1000)) {
				leds_off(LEDS_ALL);
				
#ifdef COOJA
				// In Cooja the blue led does not exist, simulation green and yellow on togheter.
				leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN) | LEDS_NUM_TO_MASK(LEDS_YELLOW));
#else
				leds_on(LEDS_BLUE);
#endif
			}
			else {
				// The building is consuming energy in the allowed range, however it signals that if additional loads are plugged, the system could stop.
				if (instant_power>=(MAX_POWER_ALLOWED-1000) && instant_power<=MAX_POWER_ALLOWED) {
#ifdef COOJA
					leds_off(LEDS_NUM_TO_MASK(LEDS_GREEN) | LEDS_NUM_TO_MASK(LEDS_YELLOW));
					leds_toggle(LEDS_NUM_TO_MASK(LEDS_RED));
#else
					leds_off(LEDS_GREEN | LEDS_BLUE);
					leds_single_off(LEDS_YELLOW);
					leds_toggle(LEDS_RED);
#endif
				}

				else {
					// The building is without energy at the moment because the range of maximum energy consumption is overcome.
					leds_off(LEDS_ALL);
#ifdef COOJA
					leds_on(LEDS_NUM_TO_MASK(LEDS_RED));
#else
					leds_on(LEDS_RED);
#endif
					max_power_consumption_achieved=true;
				}
			}
		}
	}
}


/**
 * This callback function is used to simulate a sensing activity by the sensor.
 * The values of current, voltage and power factor are sampled from the electrical grid by the sensor and the instant power is computed.
 * The actuator behaviour is consiquently changed due to the value of the computed instant power.
 */
static void execute_sensing(void *ptr) {
	// Generate new values
	
	if (activated==true) {
		generate_correct_smart_power_meter_values(&voltage,&current_consumed,&current_produced,&power_factor,&instant_power,nr_loads_attacched,MAX_AMPERE_CONSUMABLE);
		print_smart_power_meter_sensing_measurement(voltage,current_consumed,current_produced,power_factor,instant_power,nr_loads_attacched);
	} else {
		//This garantees a correct reading in case of disabled situation.
		instant_power=0;
	}
	change_status_of_actuator();
	
	// Condition that generates triggering of the power resource:
	// the new measure of instant power differs from the last sent one at least by 1kW
	// or the number of seconds passes from the last sending is greater the 60 seconds (MAX_TIME_SENDING_SENSING).
	nr_seconds_passed_last_send+=SENSING_PERIOD;

	if ((abs(last_instant_power_send-instant_power))>=1000 || nr_seconds_passed_last_send>=MAX_TIME_SENDING_SENSING) {
		res_obs.trigger();
		last_instant_power_send=instant_power;
		nr_seconds_passed_last_send=0;
	}

	// This is the case where a local black-out happens due to the over usage of the power provided.
	if (max_power_consumption_achieved) {
		reset_sensor_values(&current_consumed,&current_produced,&power_factor,&instant_power,&nr_seconds_passed_last_send,&last_instant_power_send,MAX_POWER_ALLOWED);

	} else {
		// If the max power is not achieved restart the timer associated to the sensing activity.
		ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);
	}
}


/**
 * This callback function is used to simulate a gradual restart of the main sensing activity of the sensor after the user disconnected all the devices
 * "simulated by pressing the button for more then 5 seconds".
 */
static void count_down_restart_sensor(void *ptr) {

	seconds_passed_countdown++;
	if (seconds_passed_countdown!=MAX_SECONDS_COUNTDOWN) {
		LOG_DBG("Restart from  %d sec \n", MAX_SECONDS_COUNTDOWN-seconds_passed_countdown);
		ctimer_set(&ctimer_restart_sensor, CLOCK_SECOND, count_down_restart_sensor, NULL);
	}
	else {
		LOG_DBG("System Restarted!\n");
		leds_on(LEDS_ALL);
		max_power_consumption_achieved=false;
		current_produced=random_value_generation(MIN_AMPERE_PRODUCTED,MAX_AMPERE_PRODUCTED);
		ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);
	}
}



PROCESS_THREAD(smartPowerMeter, ev, data) {

	PROCESS_BEGIN();

	initialize_sensor_values(&voltage,&current_consumed,&current_produced,&power_factor,&MAX_AMPERE_CONSUMABLE,MAX_POWER_ALLOWED);
	
	// Activation of a resource
	coap_activate_resource(&res_power, "power");
	coap_activate_resource(&res_obs, "power_obs");
	coap_activate_resource(&res_status, "status");
	coap_activate_resource(&res_max_power, "max_power");

	// BEFORE ACTUALLY STARTING
	// register smart power meter or get the max_power/status from the application in the cloud. "Acting as client"
	static int current_attempts=0;

	while (true) {

		LOG_DBG("Try to register house 1 attempt: %d \n",current_attempts+1);
		coap_endpoint_parse(SERVER_REG_EP, strlen(SERVER_REG_EP), &server_ep);

		/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
		coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
		coap_set_header_uri_path(request, SERVICE_REG_URL);

		// Create payload info.
		char *json_payload=NULL;
		create_msg_registration(&json_payload);

		if (json_payload!=NULL) {

			coap_set_header_accept(request, APPLICATION_JSON);
			coap_set_payload(request, (uint8_t *)json_payload, strlen(json_payload));
			COAP_BLOCKING_REQUEST(&server_ep, request, client_reg_handler);

			current_attempts+=1;
			LOG_DBG("Received positive registration message from Server \n");
			
			if (reg_ok) {
				LOG_DBG("Registration of House 1 successfully: smart power meter starts working ..\n");
				break;
			}
			//free(json_payload);
		}
	}

	current_attempts=0;

	ctimer_set(&ctimer_sensing, SENSING_PERIOD*CLOCK_SECOND, execute_sensing, NULL);

	while(1) {

		PROCESS_YIELD();
		if (activated==true) {
			if (ev == button_hal_periodic_event) {

				button_hal_button_t *btn;
				btn = (button_hal_button_t *)data;
				
				LOG_DBG("The user is disconnecting all the loads, %d \n",btn->press_duration_seconds);

				if(btn->press_duration_seconds > NR_SECONDS_DISCONNECTION_ALL_THE_LOADS) {

					nr_loads_attacched=0;
					LOG_DBG("The user has disconnected all the loads \n");
					seconds_passed_countdown=0;

					// To be sure to have stopped the previous sensing timer before restarting a new one.
					if(!ctimer_expired(&ctimer_sensing)) {
						ctimer_stop(&ctimer_sensing);
						LOG_DBG("Timer stopped \n");
					}
					ctimer_set(&ctimer_restart_sensor, CLOCK_SECOND, count_down_restart_sensor, NULL);
				}
			}

			else {

				if(ev == button_hal_press_event) {
					if (!max_power_consumption_achieved) {
						nr_loads_attacched=nr_loads_attacched+1;
						LOG_DBG("User has plugged a new load! \n");
					}
				}
			}
		}
	}

	PROCESS_END();

}
