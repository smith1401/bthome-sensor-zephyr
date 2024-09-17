# BTHome Sensor Zephyr

This project contains the code to enable any BLE capable microcontroller to act as a [BTHome](https://bthome.io) sensor using the Zephyr RTOS. The goal is to create a low-power device which reads sensor data (temperature, energy, pressure, etc.), which will be published at a regular interval. This can in the end be used by any BTHome compatible listener, for example [Home Assistant](https://www.home-assistant.io).

Requirements
************

* A board with BLE support
* A BTHome compatible listener with the BTHome integration running.

Building and Running
********************
When the sample is running, navigate to Devices & Services under settings in Home
Assistant. There you will be asked to configure the BTHome sensor if everything
went well.

## Project Idea
The whole project idea started when I set up my first Home Assistant server on a Raspberry Pi. I quickly learned, that there are multiple methods to integrate sensors and actuators into this home automation framework. An easy way seems to be the BTHome protocol, which basically uses the capability of BLE devices to publish data during advertisement. This is perfect for energy effecient sensor nodes, as the MCU can sleep most of the time and therefore save energy.

The perfect hardware for such a sensor node would be a small, battery powered MCU with BLE support. Luckily, I has a few BLE beacons laying around which were used for an old project a few years ago. All in all it seemed like a quick and easy project :D The fact that they used an old and outdated MCU by Nordic Semiconductors and a special hardware layout made the project not so quick and easy. But more about that later.

## Reverse engineering the iBKS 105
The iBKS 105 is a Bluetooth Low Energy (BLE) beacon based on Nordic Semiconductors NRF51822 chipset that uses a CR2477 coin cell battery.

![ibks105](doc/ibks105.jpg)

Reviewing the PCB inside the beacon seemed very promising as a few pins of the MCU have been faned out to a connector of some sort which is not populated. Additionally, there are also some pins of the MCU mapped to a few through-hole pads and SMT pads. Interestingly, the part *U2* is also not populated, but we will focus on the available GPIO on the pads which we could use in multiple ways to connect to sensors, actuators, you name it.

A few minutes poking around with a multimeter revealed, that there are 13 GPIOS of the NRF51822 which we could use. One of those seems to be a push button *(GPIO7)* and another seems to be the input to a MOSFET *(GPIO6)* which could be used to switch higher loads, for example actuators.

![ibks105](doc/ibks_pinout.svg)

The board also features a LFXO (low-frequency crystal oscilllator), which is freat news for low-power applications. The HFXO is a **32 MHz !!!** crystal. This is was I mentioned before and took me hours debugging. The NRF51 architecture has a 16 MHz internal clock. Normally (in all other boards I found) a 16 MHz external HFXO is used. I think, as this design is very small and dense, the 32 MHz crystal, which has a smaller form factor is therefore used. Unfortunatelly, all existing code I tried did not work once I tried to use the RF section of the chip. The much more accurate external HFXO (20 ppm drift) is only used when the RF section is activated, as this has strict timing requirements.

After hours of debugging I finally found the solution in the [Nordic Forum](https://devzone.nordicsemi.com/f/nordic-q-a/6394/use-external-32mhz-crystal). What you basically have to do to get the chip working with an external 32 MHz clock is two things.

1. Write a specific section in the UICR
2. Set the frequency in the *NRF_CLOCK* register

This small section of code does both things and has to be executed once before the RF section is used.

```cpp

  // Write UICR section
  *(uint32_t *)0x10001008 = 0xFFFFFF00;

  // Set the external high frequency clock source to 32 MHz
  NRF_CLOCK->XTALFREQ = 0xFFFFFF00;

  // Start the external high frequency crystal
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  // Wait for the external oscillator to start up
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {}
```

And voilà, BLE finally works on the NRF51 😁