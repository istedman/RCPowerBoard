# RCPowerBoard

![Photo of the PCB](https://github.com/istedman/RCPowerBoard/blob/main/IMG_20240406_182102.jpg)

A power distribution and battery monitor PCB for radio control vehicles. It can handle up to 15A, I use it on my smaller boats without issue.
Power connections are via plated through-hole or deans/T connectors. There are 4 pairs of smaller holes, connected via a 3.15A polyfuse, for accessory circuits. This protects the battery from short circuits in the accessory devices.

A PIC12F1840 microcontroller monitors the battery/cell voltages and when it gets too low, a buzzer will sound to let you know.

## Supported battery types

4-8 cell NiMh
1-3 cell Lipo
6V or 12V lead acid.

## Software
The design is minimal. A jumper identifies the battery type to monitor, providing you set this with a freshly charged battery pack, it will correctly determine the cell count and store the required settings in EEPROM. Only needs to be done once.
The BatteryMonitor.X directory contains the full source code, exported from MPLabX 6.20

**The software is designed to monitor the battery pack to these lower levels:**
_Lithium Polymer (LiPo)_
Single cell (1S) Lipo    3.6V
Duall cell (2S) Lipo     7.2V
Triple cell (3S) Lipo    10.8V
#define TRIPLE_LIPO_HIGH    12.600     // 12.6V

_Lead acid/AGM cell voltages_
 100% 2.12-2.15V per cell
 20% 1.93V/cell
 0% 1.75V/cell
 We will warn at 20% or 1.93V/cell
6V lower limit is  5.790V
12V lower limit is 11.580V     

_NiMH battery packs_
4 Cell NiMh    4.000V
5 Cell NiMh    5.000V    
6 Cell NiMh    6.00V
7 Cell NiMh    7.00V
8 Cell NiMh    8.00V

The software uses the internal A/D converter of the PIC microcontroller to sample the cell voltage, average it and if it drops below the limits detailed above, it will sound the buzzer. The A/D sampling is at a low rate, this was found to be effective in filtering out momentary supply blips when they accured, eliminating false failures.

Microchip Code Configurator (MCC) was used to setup the device and calibrate the internal voltage reference.

**Measured accuracy has been +/- 0.8% at room temperature** the 1% resistors used in the potential divider, contribute to the error in measurement, which was around 80mV in a 14V range.

## Known issues

The MCP2030 programming header is wrong, I had to swap signals around, will fix it in the next PCB iteration.
The deans/T connector footprint is off, again will be fixed in next revision.
The software only supports Nimh cells with even number of cells.
The PCB is bigger than it needs to be. I have 25 PCBs so there is no rush to fix the design, yet.

## Future development
A new, smaller design is in development that just monitors the battery and supports a wider range of chemistries/types. It can operate from a single cell Lipo or 3 NiMh cells. Plan to publish this later in the year, when finished.
