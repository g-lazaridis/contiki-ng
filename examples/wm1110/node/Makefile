CONTIKI_PROJECT = node
CONTIKI = ../../..

TARGET = nrf52840
BOARD = wm1110-dk

MAKE_WITH_SECURITY ?= 0

all: $(CONTIKI_PROJECT)

# Orchestra is not optional here: the LoRa-tuned rules are what make the
# 750 ms timeslot usable.
MODULES += os/services/orchestra

MODULES_REL += .

MAKE_MAC = MAKE_MAC_TSCH
MAKE_ROUTING = MAKE_ROUTING_RPL_LITE

CFLAGS += -DNETWORK_PROFILE_INC=10-per-cent-duty-cycle

include $(CONTIKI)/Makefile.include
