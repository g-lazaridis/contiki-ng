#ifndef LR11XX_TSCH_TIMING_H_
#define LR11XX_TSCH_TIMING_H_

#include "net/mac/tsch/tsch.h"

#if MAC_CONF_WITH_TSCH == 1
extern const tsch_timeslot_timing_usec tsch_timing_lr11xx;
#endif /* MAC_CONF_WITH_TSCH */

#endif /* LR11XX_TSCH_TIMING_H_ */
