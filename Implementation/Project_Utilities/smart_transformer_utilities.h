#include <stdbool.h>
#include "senml-json.h"

// Fault type costants
#define FAULT_TYPE_0 0 // 0 No Fault 
#define FAULT_TYPE_1 1 // 1 Fault on phase A
#define FAULT_TYPE_2 2 // 2 Fault on phase A,B
#define FAULT_TYPE_3 3 // 3 Fault on phase A,B
#define FAULT_TYPE_4 4 // 4 Fault symmetric 


// Safety range Ia
#define RANGE_MIN_IA -15.00
#define RANGE_MAX_IA 24.00
#define VAR_IA 0.1

// Safety range Ib
#define RANGE_MIN_IB -32.28
#define RANGE_MAX_IB 34.13
#define VAR_IB 0.1

// Safety range Ia+Ib+Ic
#define MIN_SUM_RANGE_CURR_TYPE_0 -3.44
#define MAX_SUM_RANGE_CURR_TYPE_0 -2.90

// Safety range Va
#define RANGE_MIN_VA -0.56
#define RANGE_MAX_VA 0.58
#define VAR_VA 0.01

// Safety range Vb
#define RANGE_MIN_VB -0.29
#define RANGE_MAX_VB 0.51
#define VAR_VB 0.01

// Safety range Va+Vb+Vc
#define MIN_SUM_RANGE_VOLT_TYPE_0 -0.01
#define MAX_SUM_RANGE_VOLT_TYPE_0 +0.01

// Other values used during the fixing of the ranges
#define MAX_RAND_VAR_VOLT 0.1

#define MIN_VAR_IA 5 
#define MAX_VAR_IA 10 

#define MIN_VAR_IB_TYPE_1 5
#define MAX_VAR_IB_TYPE_1 10

#define MIN_VAR_IB_TYPE_2 100
#define MAX_VAR_IB_TYPE_2 200

#define MIN_VAR_IB_TYPE_3 95
#define MAX_VAR_IB_TYPE_3 105

#define SAFETY_CHECK_IB 225

#define RANGE_VC 0.5
#define MIN_VAR_RANGE_VC 0.01


// Values to simulate generatation of a fault

// FAULT TYPE-0 Starting values
#define IA_FAULT_TYPE_0 -2.93
#define IB_FAULT_TYPE_0 -26.16
#define IC_FAULT_TYPE_0 27.00

#define VA_FAULT_TYPE_0 0.58
#define VB_FAULT_TYPE_0 -0.36
#define VC_FAULT_TYPE_0 -0.26


// FAULT TYPE-1 - starting values
#define IA_FAULT_TYPE_1 -7.83
#define IB_FAULT_TYPE_1 47.13
#define IC_FAULT_TYPE_1 -49.91

#define VA_FAULT_TYPE_1 -0.37
#define VB_FAULT_TYPE_1 0.57	
#define VC_FAULT_TYPE_1 -0.19	


// FAULT TYPE-2	- starting values

#define IA_FAULT_TYPE_2 -10.69
#define IB_FAULT_TYPE_2 -759.75
#define IC_FAULT_TYPE_2 -56.36

#define VA_FAULT_TYPE_2 -0.03
#define VB_FAULT_TYPE_2 0.23	
#define VC_FAULT_TYPE_2 -0.20	

// FAULT TYPE-3 - starting values

#define IA_FAULT_TYPE_3 17.70
#define IB_FAULT_TYPE_3 -773.05
#define IC_FAULT_TYPE_3 757.35

#define VA_FAULT_TYPE_3 -0.03
#define VB_FAULT_TYPE_3 0.0009	
#define VC_FAULT_TYPE_3 0.03	

	
// FAULT TYPE-4 - starting values

#define IA_FAULT_TYPE_4 166.59
#define IB_FAULT_TYPE_4 -848.06
#define IC_FAULT_TYPE_4 681.51

#define VA_FAULT_TYPE_4 -0.03
#define VB_FAULT_TYPE_4 -0.006	
#define VC_FAULT_TYPE_4 0.03	



void generate_initial_transformer_values(float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc);
void generate_correct_transformers_values(float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc);
void generate_transformer_fault(int type, float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc);
void create_msg_house_change_state(char **json_string_payload,bool state);
void create_msg_registration_st(char **json_string_payload);

void repairing_fault(int type, float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc);
void handling_type_4(float *Ia, float *Ib, float *Ic, float *Va, float *Vb, float *Vc);



