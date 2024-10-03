// Define the ranges of voltage provided
#define MIN_VOLTAGE_PROVIDED 225
#define MAX_VOLTAGE_PROVIDED 230
#define VAR_VOLTAGE_PROVIDED 0.5

// Define the variational range of ampere consumed
#define VAR_AMPERE_CONSUMED 0.05

// Define the range of ampere produced
#define MIN_AMPERE_PRODUCTED 7.0
#define MAX_AMPERE_PRODUCTED 8.0
#define VAR_AMPERE_PRODUCTED 0.1

// Define the range of the power factor
#define MIN_POWER_FACTOR 0.8
#define MAX_POWER_FACTOR 1
#define VAR_POWER_FACTOR 0.05

// Define the unit of consumption of a single load attacched
#define STD_AMP_CONSUMPTION 3.2075 // Computed approximately to obtain 1kW per load attacched


void initialize_sensor_values(float *voltage, float *current_consumed, float *current_produced, float *power_factor, float *MAX_AMPERE_CONSUMABLE,int MAX_POWER_ALLOWED);
float compute_total_instant_power(float voltage, float current_consumed, float current_produced, float power_factor);
float compute_max_ampere_consumable(float max_power, float min_voltage, float min_power_factor);

void generate_correct_smart_power_meter_values(float *voltage, float *current_consumed, float *current_produced, float *power_factor,float *instant_power, int nr_loads_attacched,float MAX_AMPERE_CONSUMABLE);

void reset_sensor_values(float *current_consumed,float *current_produced,float *power_factor,float *instant_power,int *nr_seconds_passed_last_send,float *last_instant_power_send,float MAX_POWER_ALLOWED);

void create_msg_registration(char **json_string_payload);


