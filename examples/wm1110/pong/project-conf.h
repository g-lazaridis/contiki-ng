/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
#include "lr11xx-radio-conf.h"

/* LoRa timeslots do not fit in the 16-bit TSCH timing fields. */
#define TSCH_CONF_EXTEND_TS_SIZE 1
/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
