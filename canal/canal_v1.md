# Canal V1.

## Components

- Arduino
- 3 x Btns
- Liquid Crystal Display
- SD Card
- GPS
- Accelerometer

## Wiring

- GPS -> Arduino

  - GND -> GND
  - TX -> 6
  - RX -> 5
  - VCC -> 5V

- Btn -> Arduino

  - See image (can be a bit fildey)

- LCD -> Arduino

  - K -> GND
  - A -> VCC (5V) with Resistor
  - D7 -> 2
  - D6 -> 3 
  - D5 -> 4
  - D4 -> 9
  - E -> 8
  - RW -> GND
  - RS -> 7
  - VO, VDD, VSS -> See image

- SD Card -> Arduino

  - GND -> GND
  - MISO -> 12
  - SCK -> 13
  - MOSI -> 11
  - CS -> 10
  - +5V -> +5V

- Accelerometer -> Arduino

  - SOA -> Above AREF
  - SCL -> Two above Aref
  - GND -> GND
  - VCC -> 5V

## Discussion

- A lot of issues surrounding memory
- In the end couldn't keep saving data to file - kept crashing
- An alternative is to use the ESP32 