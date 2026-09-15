CONTIKI_PROJECT = cooja-node
CONTIKI = ../../..

TARGET = cooja

MAKE_WITH_SECURITY ?= 0

all: $(CONTIKI_PROJECT)

# Orchestra is not optional here: the LoRa-tuned rules are what make the
# 750 ms timeslot usable.
MODULES += os/services/orchestra
# Shared LoRa radio/TSCH/Orchestra configuration profiles.
MODULES += $(CONTIKI_NG_DRIVERS_RADIO_DIR)/lr1110/configuration-profiles
MODULES_REL += .

MAKE_MAC = MAKE_MAC_TSCH
MAKE_ROUTING = MAKE_ROUTING_RPL_LITE

include $(CONTIKI)/Makefile.include
