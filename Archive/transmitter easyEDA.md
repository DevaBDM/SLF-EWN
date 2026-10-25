> **Archived draft.** The radio in this note is the RFM95W breakout. The built board uses Ra-02 on the same 2.54 mm headers — see [[Transmitter PCB]].

# Transmitter PCB parts — archived THT list

The transmitter was drawn with through-hole parts only. The radio was the RFM95W breakout, with standard THT connectors and the power-stabilisation parts around it.

## 1. Radio

- Part: RFM95W breakout adapter board.
- EasyEDA search: `RFM95W Breakout`.
- Reason: it turns the 2.0 mm SMD pads of the bare RFM95W into two rows of 2.54 mm through-holes.

## 2. Headers

- Part: 2.54 mm female pin headers.
- EasyEDA search: `HDR-F-2.54` (two 1×8 or 1×10 strips, depending on the breakout footprint).
- Reason: the female sockets are soldered into the board and the radio plugs into them. That keeps soldering heat away from the radio and allows a module swap.

## 3. Antenna connector

- Part: through-hole SMA female, right-angle or straight.
- EasyEDA search: `SMA-TH` or `SMA-JW-P`.
- Reason: five through-hole pins (one centre signal, four chassis ground legs). It sits on the board edge and accepts a standard LoRa whip.

## 4. Power stabilisation

A LoRa transmitter pulls brief current spikes of up to 120 mA when it sends. These parts sit next to the radio VCC pin so the rail does not collapse:

- Electrolytic capacitor, through-hole radial: 10–100 µF (`CAP-TH` or `Capacitor Radial`).
- Ceramic capacitor, through-hole disc: 0.1 µF (`C-DIAL` or `Capacitor Ceramic Disc`) for high-frequency noise.

## 5. Layout checklist

1. Keep the trace from the RFM95W `ANT` pin to the SMA centre pin short and straight.
2. Use a ground pour where the single copper layer allows; bridge crossings with jumpers or a top-side solder blob, not a second layer.
