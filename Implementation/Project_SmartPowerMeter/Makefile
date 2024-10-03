CONTIKI_PROJECT = SmartPowerMeter
all: $(CONTIKI_PROJECT)

MODULES_REL += ./resources

MODULES_REL += ../Project_Utilities
LDFLAGS += -u _printf_float

CONTIKI = ../..

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

MODULES += os/services/shell
include $(CONTIKI)/Makefile.include




