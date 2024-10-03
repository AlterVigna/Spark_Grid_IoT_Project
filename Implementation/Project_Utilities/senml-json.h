#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include "os/net/linkaddr.h"

#define BASE_NAME_MAX_LEN 30

typedef enum {
	SENML_TYPE_V,
	SENML_TYPE_BV,
	SENML_TYPE_SV
} senml_value_type;

typedef struct {
	char *string_value;
	bool boolean_value;
	float float_value;
} senml_type;


typedef struct {
	char *name;
	char *unit;
	senml_value_type type;
	senml_type value;
	int time;
	//float sum_value; not used in this project
	//int update_time; not used in this project
} senml_measurement;


typedef struct {
	char base_name[BASE_NAME_MAX_LEN];
	int base_time;
	char *base_unit;
	int version;
	int nr_measurments;
	senml_measurement *measurements;
} senml_payload;




void create_base_name_attribute(char *base_name);

void create_senml_payload(senml_payload *payload,char **json_string_payload);
//void parse_senml_payload(char *json_string_payload, senml_payload **payload);
