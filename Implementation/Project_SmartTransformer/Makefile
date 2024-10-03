CONTIKI_PROJECT = SmartTransformer
all: $(CONTIKI_PROJECT)

MODULES_REL += ./resources
MODULES_REL += ../Project_Utilities

LDFLAGS += -u _printf_float

MODULES_REL += /home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn
TARGET_LIBFILES += -lm
INC += /home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn


CONTIKI = ../..

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

MODULES += os/services/shell
include $(CONTIKI)/Makefile.include




