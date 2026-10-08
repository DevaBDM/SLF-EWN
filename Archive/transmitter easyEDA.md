To build the Transmitter PCB using exclusively through-hole technology (THT), you will use the ==RFM95W Breakout Board footprint== as the central component, surrounded by standard through-hole connectors and power stabilization components.

Here are the exact components and footprints used for the transmitter PCB design in EasyEDA:

## 1. The Core Radio Component

- The Module: RFM95W Breakout Adapter Board
- EasyEDA Search Term: `RFM95W Breakout`
- Why it is used: It adapts the tiny 2.0mm SMD pads of the bare RFM95W chip into two rows of standard 2.54mm through-holes.

## 2. Connectors for Easy Assembly (No Soldering the Module Directly)

- The Component: 2.54mm Pitch Female Pin Headers
- EasyEDA Search Term: `HDR-F-2.54` (Select two 1x8 or 1x10 strips depending on the exact breakout board footprint chosen).
- Why it is used: You solder these female sockets permanently into the PCB. The RFM95W Breakout Board then plugs directly into them. This protects the sensitive radio chip from soldering heat and allows you to swap modules instantly if one gets damaged.

## 3. RF Antenna Interface

- The Component: Through-Hole SMA Female Connector (90-degree Right Angle or Straight)
- EasyEDA Search Term: `SMA-TH` or `SMA-JW-P`
- Why it is used: It has 5 robust through-hole pins (1 central signal pin and 4 outer chassis ground legs). It solders directly into the edge of your PCB so you can screw on a standard, high-gain LoRa rubber-duck antenna.

## 4. Power Stabilization Components (Critical for Battery Operation)

Because a LoRa transmitter draws brief, heavy spikes of current (up to 120mA) when sending a packet, you must place these components right next to the VCC pin of the radio header to prevent voltage drops from crashing your board:

- Electrolytic Capacitor (Through-hole Radial): 10µF to 100µF Capacitor (EasyEDA Search: `CAP-TH` or `Capacitor Radial`).
- Ceramic Capacitor (Through-hole Disc): 0.1µF Capacitor (EasyEDA Search: `C-DIAL` or `Capacitor Ceramic Disc`) to filter high-frequency noise. [1]

---

## Layout Checklist for your EasyEDA PCB:

1. Keep the trace between the RFM95W "ANT" pin and the SMA connector's central pin as short and straight as possible to prevent signal loss.
2. Create a large GND copper pour (Ground Plane) on both the top and bottom layers of your PCB to ensure a stable radio ground reference. [2, 3]