/**
 * \file
 *        LoRa airtime calculation macros.
 *
 *        Computes preamble, symbol and total packet duration from the
 *        spreading factor, bandwidth, coding rate and payload length. TSCH
 *        uses these to derive TSCH_PACKET_DURATION() for a LoRa PHY.
 * \author
 *        Gavriil Lazaridis
 */

#ifndef LORA_TIMINGS_H_
#define LORA_TIMINGS_H_

#define CEILING_POS(X) ((X - (int)(X)) > 0 ? (int)(X + 1) : (int)(X))
#define LORA_T_SYM(sf, bw) (((1 << (sf)) * 1000000) / (bw))
#define LORA_T_PREAMBLE(sf, bw, prlen) (((prlen) + 4) * LORA_T_SYM((sf), (bw)) + (LORA_T_SYM((sf), (bw)) / 4))
#define LORA_SYM_NB(sf, crc, implicit_header, cr, len) ( \
          8 + MAX( \
            CEILING_POS((float)((8 * (len)) - (4 * (sf)) + 28 + (16 * (crc)) - (20 * (implicit_header))) / (4 * (sf))) \
            * ((cr) + 4), \
            0 \
            ) \
          )
#define LORA_T_PACKET(sf, bw, crc, implicit_header, cr, prlen, len) ( \
          (uint64_t)LORA_T_PREAMBLE(sf, bw, prlen) + (((uint64_t)LORA_SYM_NB(sf, crc, implicit_header, cr, len) * (uint64_t)LORA_T_SYM(sf, bw))) \
          )

#endif /* LORA_TIMINGS_H_ */