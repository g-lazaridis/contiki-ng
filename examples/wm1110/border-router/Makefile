CONTIKI_PROJECT = border-router
CONTIKI = ../../..

TARGET = nrf52840
BOARD = wm1110-dk

MAKE_WITH_SECURITY ?= 0

all: $(CONTIKI_PROJECT)

# Orchestra is not optional here: the root must run the same LoRa-tuned
# schedule as the nodes for them to reach it.
MODULES += os/services/orchestra
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router

MODULES_REL += .

MAKE_MAC = MAKE_MAC_TSCH
MAKE_ROUTING = MAKE_ROUTING_RPL_LITE

CFLAGS += -DNETWORK_PROFILE_INC=10-per-cent-duty-cycle

include $(CONTIKI)/Makefile.include
