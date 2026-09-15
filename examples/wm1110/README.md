# Wio-WM1110 examples

Examples for running Contiki-NG's IPv6 mesh stack over LoRa on the Wio-WM1110
development kit. See [`doc/platforms/wm1110.md`](../../doc/platforms/wm1110.md)
for the board port and [the top-level README](../../README.md) for what this
fork changes.

Unless stated otherwise, build and flash with:

```bash
make TARGET=nrf52840 BOARD=wm1110-dk <project>.upload
```

## Bring-up

Run these in order on a new board; each one depends on less than the last.

| Example | What it does |
|---------|--------------|
| `hello-world` | Prints over USB serial. Confirms the board port, clock and console. |
| `lr11xx-version` | Reads the LR1110 hardware and firmware version over SPI. Confirms the SPI link and the LR11xx HAL without involving the network stack. |
| `firmware-update` | Flashes a Semtech firmware image onto the LR1110. The images are compiled in; pick one by editing the `#include` in `firmware-update.c`. |

If `lr11xx-version` reports a firmware older than the images bundled with
`firmware-update`, update it before going further — the radio driver is
developed against 04.01.

## Radio tests

These drive `NETSTACK_RADIO` directly, with no MAC or network stack. They are
the quickest way to tell whether two boards can hear each other at all.

| Example | What it does |
|---------|--------------|
| `ping` | Transmits a fixed payload every 5 s and waits for a reply, logging the length received. |
| `pong` | Listens, and echoes back whatever `ping` sends. |
| `ping-pong` | Both roles in one image: a board that hears nothing starts pinging, and the two then alternate. Flash the same binary to two boards. |

Use `ping` and `pong` as a pair on two boards, or `ping-pong` on both.

## Network

The full stack: TSCH over LoRa, RPL routing, 6LoWPAN, with the Orchestra rules
this fork adds.

| Example | What it does |
|---------|--------------|
| `border-router` | RPL root and PAN coordinator, bridged to a host over USB serial with `tunslip6`. |
| `node` | Leaf node. Joins the PAN, logs when it becomes reachable, then idles — ping it from the host to measure the network. |

Bring the network up by flashing `border-router` to one board and `node` to the
rest, then bridge the root to the host:

```bash
cd border-router
make TARGET=nrf52840 BOARD=wm1110-dk connect-router PORT=/dev/ttyACM0
```

That runs `tunslip6` and creates a `tun0` interface on the `fd00::1/64` prefix
(override with `PREFIX=`). Node addresses are logged by the border router; ping
one to reproduce the measurements in the top-level README. Expect the first
replies to take a while — with a 750 ms timeslot, joining the PAN is not
instant.

Both use the `10-per-cent-duty-cycle` configuration profile from
`arch/dev/radio/lr1110/configuration-profiles/`, selected with
`-DNETWORK_PROFILE_INC=` in their Makefiles. That profile is the single place
where the radio parameters, TSCH settings and Orchestra rule set are defined,
so a change there applies to every node in the network — which is what you
want, since they must agree.

## Simulation

| Example | What it does |
|---------|--------------|
| `cooja-router` | The border router, built for Cooja. |
| `cooja-node` | The leaf node, built for Cooja. |

```bash
cd cooja-node   && make TARGET=cooja
cd ../cooja-router && make TARGET=cooja
```

Then load `build/cooja/*.cooja` as mote types in Cooja. To reach the simulated
network from the host, give the router mote a serial socket and run:

```bash
cd cooja-router
make TARGET=cooja connect-router-cooja
```

These share the same configuration profile as the hardware examples, so the
schedule under test is the real one. What differs is the PHY: Cooja has no
LR1110, so the examples set `COOJA_CONF_LORA_PHY`, which slows the simulated
radio to LoRa air times and selects the matching timeslot template in
`lora-sim-tsch-timing.c`. Without that macro the Cooja platform behaves as it
does upstream, so the stock examples and the regression tests are unaffected.

Note that the simulated timeslot timings are not identical to the hardware
ones: Cooja's radio turns on instantly, so the ACK delays are shorter than the
values the real boards need.
