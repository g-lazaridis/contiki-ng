# Contiki-NG over LoRa: TSCH and RPL on the Wio-WM1110

A fork of [Contiki-NG](https://github.com/contiki-ng/contiki-ng) that runs its
IPv6 mesh stack — 6LoWPAN, RPL and TSCH — over a LoRa PHY instead of IEEE
802.15.4 at 2.4 GHz.

This is the implementation behind the MSc dissertation [*Integration and
evaluation of LoRa protocol in a real time network operating
system*](https://pergamos.lib.uoa.gr/en/item/uoadl:3471078) (Gavriil
Lazaridis, Department of Physics, University of Athens, 2024), supervised by
Anna Tzanakaki and co-supervised by Markos Anastasopoulos. The full text is
available from Pergamos, the university's digital repository.

## Why this is not straightforward

Contiki-NG's TSCH implementation assumes a 2.4 GHz radio: a 127-byte frame is
on air for roughly 4 ms, so the default timeslot is 10 ms. The same frame sent
with LoRa at SF5 over 125 kHz takes about 73 ms, and European duty-cycle
regulation caps transmit time at 10% in the 869.4–869.65 MHz band. Together
those force a **750 ms timeslot** — 75 times the 802.15.4 default.

A timeslot that long breaks assumptions throughout the stack. Two of them
needed changing:

- **TSCH timing fields are 16-bit.** A 750 ms timeslot does not fit in a
  `uint16_t` of microseconds, so the timing type is configurable
  (`TSCH_CONF_EXTEND_TS_SIZE`).
- **Orchestra's default schedules become unusable.** With a 17-slot unicast
  slotframe, a single ping-pong exchange between the root and a neighbour
  costs 1.5 s at best and **24 s at worst**. That is unacceptable even for
  monitoring traffic, let alone alarms.

The scheduling problem is where most of the work went, and where the results
come from.

## What is in this fork

| Area | Location |
|------|----------|
| Wio-WM1110 DK board port | `arch/platform/nrf52840/wm1110-dk/` |
| LR1110 Contiki-NG radio driver | `arch/dev/radio/lr1110/` |
| Semtech SWDR001 LR11xx driver (vendored) | `arch/dev/radio/lr1110/swdr001/` |
| Radio/TSCH/RPL/Orchestra configuration profiles | `arch/dev/radio/lr1110/configuration-profiles/` |
| TSCH support for long timeslots | `os/net/mac/tsch/` |
| New Orchestra rules | `os/services/orchestra/` |
| Examples | `examples/wm1110/` |
| nRF52840 additions (PPI timestamping, RTC rtimer) | `arch/cpu/nrf52840/` |

### Hardware

The [Wio-WM1110](https://wiki.seeedstudio.com/wio_wm1110_dev_kit/) development
kit, whose module combines an **nRF52840** host MCU with a **Semtech LR1110**
sub-GHz transceiver over SPI. Contiki-NG already supported the nRF52840, so the
porting effort was the board definition and the LR1110.

### Radio configuration

| Parameter | Value |
|-----------|-------|
| Frequency | 869.45 MHz |
| Bandwidth | 125 kHz |
| Spreading factor | SF5 |
| Coding rate | 4/5 |
| Output power | 13 dBm |
| Preamble | 12 symbols |
| Duty cycle budget | 10% |

A 50 kHz guard is left either side of the channel. There is no room left in
the band for a second channel, so TSCH runs single-channel — its channel
hopping is configured with a one-entry hopping sequence.

### Packet timestamping

TSCH synchronisation is only as good as the receive timestamp. The LR1110
raises its IRQ pin on preamble detection; on the WM1110 that pin is wired
internally to the nRF52840's P1.08. The port uses the nRF's **PPI** to clear a
timer directly from that pin transition, with no CPU involvement, and reads the
timer when the frame is read out. Measured over 1000 timeslots, a node's slot
start sits about **1 ms** from its router's.

### New Orchestra rules

Contiki-NG already ships a `special_for_root` rule, which gives the root — a
node that is not energy-constrained — a dedicated slotframe. It has two
problems: each neighbour still waits for its own transmit slot in that
slotframe, and it cannot be combined with the non-storing unicast rule, which
has a lower slotframe handle and therefore wins every slot.

This fork adds:

- **`special_for_root_always_on`** — a slotframe of size **1**. The root
  listens in every timeslot; every other node may transmit to the root in
  every timeslot, unless a higher-priority slotframe has scheduled something.
  Upward traffic to the root no longer waits for a slot to come round.
- **`unicast_per_non_root_neighbor`** — the non-storing unicast rule, modified
  to decline packets addressed to the root, so it stops shadowing the rule
  above.
- **`special_for_root_even_rx_slots`** — a lower-duty-cycle variant in which
  the root receives on even slots only.

Slotframe lengths were also shortened, trading network capacity for latency on
the assumption that a node has no more than 5–6 neighbours:

| Slotframe | Length |
|-----------|--------|
| Beacon (EB) | 29 |
| Common shared | 21 |
| Unicast non-root | 11 |
| Unicast root | 1 |

The default rule set is in
`arch/dev/radio/lr1110/configuration-profiles/10-per-cent-duty-cycle.h`.

## Results

### Cooja simulation — default vs. custom Orchestra rules

Average ICMPv6 RTT in seconds and total packet loss, pinging once per minute.
In the multi-hop rows every node is pinged, not just the most distant one.

| Topology | Node | Default RTT | Default loss | Custom RTT | Custom loss |
|----------|------|-------------|--------------|------------|-------------|
| 1 node, 1 hop | 2 | 12.8 | 1.7% | **4.8** | **0%** |
| 2 nodes, 1 hop | 2 | 16.7 | 0% | **7.3** | 0% |
| | 3 | 15.2 | 1.6% | **10.8** | **0%** |
| 3 nodes, 1 hop | 2 | 17.4 | 19% | **9.0** | **0%** |
| | 3 | 32.8 | 7.6% | **6.7** | **0%** |
| | 4 | 19.2 | 8.9% | **6.4** | **0%** |
| 2 nodes, 2 hops | 2 | 39.4 | 5.3% | **21.8** | **0%** |
| | 3 | 117 | 8% | **32.3** | **0%** |
| 6 nodes, 2 hops | 3 | 115.2 | 29.5% | **17.7** | **0%** |
| | 5 | 122.9 | 4.6% | **18.6** | **0%** |
| | 7 | 117 | 10.8% | **9.3** | **0%** |

Multi-hop is where it matters most: in the 6-node, 2-hop case node 3 goes from
115.2 s and 29.5% loss to 17.7 s and no loss at all. Packet loss is zero in
every simulated scenario with the custom rules.

### Real deployment

Four boards in waterproof enclosures at NCSR Demokritos, Agia Paraskevi,
Athens — no line of sight, trees and buildings in between. Low traffic is one
ping per minute per node; high traffic staggers all three nodes 20 s apart on
a 60 s cycle.

| Topology | Node | Low RTT | Low loss | High RTT | High loss |
|----------|------|---------|----------|----------|-----------|
| Star | 1 | 5.3 | 0% | 5.9 | 1.2% |
| | 2 | 6.1 | 0% | 6.7 | 1.6% |
| | 3 | 5.8 | 0% | 6.3 | 2.3% |
| Two-hop | 1 | 5.1 | 0% | 5.4 | 1.2% |
| | 2 | 14.4 | 1.6% | 21.4 | 1.4% |
| | 3 | 15.7 | 4% | 25.9 | 8% |
| Multi-hop | 1 | 5.1 | 0% | 5.4 | 0% |
| | 2 | 17 | 1% | 21.4 | 1.4% |
| | 3 | 26 | 7.2% | 25.9 | 12% |

## Getting started

Install an ARM toolchain as for any Contiki-NG target — see
[the Contiki-NG documentation](https://docs.contiki-ng.org/en/master/doc/getting-started/index.html).

Contiki-NG carries fifteen submodules, most of them vendor SDKs for hardware
this fork does not touch. Only three are needed here, so it is worth cloning
without `--recurse-submodules` and fetching just those:

```bash
git clone https://github.com/g-lazaridis/contiki-ng.git
cd contiki-ng
git submodule update --init arch/cpu/arm/CMSIS arch/cpu/nrf52840/lib/nrf52-sdk
```

| Submodule | Needed for |
|-----------|------------|
| `arch/cpu/arm/CMSIS` | any ARM target |
| `arch/cpu/nrf52840/lib/nrf52-sdk` | the nRF52840, and so the WM1110 |
| `tools/cooja` | running simulations, not needed to build |

Add `tools/cooja` if you intend to simulate:

```bash
git submodule update --init tools/cooja
```

Build and flash a node:

```bash
cd examples/wm1110/node
make TARGET=nrf52840 BOARD=wm1110-dk node.upload
```

Run the simulation instead, which needs no hardware:

```bash
cd examples/wm1110/cooja-node   && make TARGET=cooja
cd ../cooja-router              && make TARGET=cooja
```

then load the motes in Cooja. See
[`examples/wm1110/README.md`](examples/wm1110/README.md) for what each example
does, and [`doc/platforms/wm1110.md`](doc/platforms/wm1110.md) for the board
port.

## Limitations

These are known and were out of scope for the dissertation:

- **Single channel.** No spectrum was left for a second channel, so TSCH's
  channel hopping is unused.
- **No CCA.** `channel_clear()` always reports the channel free; the LR1110
  supports channel activity detection, but it is not wired up.
- **Duty cycle is not enforced in software.** The 10% budget is respected by
  construction, through the timeslot length, not by a regulatory limiter.
- **The root rule assumes a small, quiet neighbourhood.** With many neighbours
  of the root or high packet rates, giving every node a transmit opportunity in
  every slot will produce collisions.
- **No security.** TSCH link-layer security is present in Contiki-NG but was
  not evaluated here.

## Relationship to upstream

This fork branched from Contiki-NG `develop` at commit `bc0c7a3`. Changes to
files shared with upstream are kept additive and guarded by macros wherever
possible, so that existing platforms and the regression test suite are
unaffected — in particular, the LoRa Cooja profile is opt-in via
`COOJA_CONF_LORA_PHY`.

There is no intention to upstream this work as-is.

## Licensing

Contiki-NG is distributed under the [3-clause BSD license](LICENSE.md), and
contributions in this fork are under the same terms.

`arch/dev/radio/lr1110/swdr001/` is the Semtech
[SWDR001](https://github.com/Lora-net/SWDR001) LR11xx driver, vendored
unmodified except for the HAL implementation, and is distributed by Semtech
under the Clear BSD license. Its copyright headers are retained in each file.

The LR1110 transceiver firmware images under
`examples/wm1110/firmware-update/` are Semtech binaries.

## Citing

> G. Lazaridis, *Integration and evaluation of LoRa protocol in a real time
> network operating system*, MSc dissertation, Department of Physics,
> National and Kapodistrian University of Athens, 2024.
> https://pergamos.lib.uoa.gr/en/item/uoadl:3471078

## Upstream Contiki-NG

* Repository: https://github.com/contiki-ng/contiki-ng
* Documentation: https://docs.contiki-ng.org/
* Web site: http://contiki-ng.org
