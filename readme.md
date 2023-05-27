# ESP8266 Max7219 Matrix Clock

The ESP8266 Max7219 Matrix Clock is a clock project that utilizes an ESP8266 microcontroller and four Max7219 LED matrix modules. It allows you to control the clock over a local area network (LAN) using UDP commands and supports LAN messaging capabilities.


## Features

- Displays time and date on four Max7219 LED matrix module.
- Adjustable UTC offset to display the correct local time.
- Customizable hold interval to control the duration of displayed messages.
- Adjustable clock interval to update the time display.
- LAN messaging support for sending and receiving messages over UDP.

## Hardware Requirements

To build this project, you will need the following components:

- ESP8266-based microcontroller board (e.g., NodeMCU, Wemos D1 Mini)
- Max7219 LED matrix module
- Jumper wires
- Power supply (depending on the LED matrix module)

## Software Requirements

To program the ESP8266 microcontroller, you will need the following:

- Arduino IDE or platform of your choice
- ESP8266 board support package installed in the Arduino IDE

## Getting Started

1. Connect the Max7219 LED matrix module to the ESP8266 microcontroller according to the wiring instructions provided in the project's source code.
5V > "arduino pin" 5V  labeled 5V
GND > "arduino pin" GND  labeled GND
DIN_PIN > "arduino pin" 15  labeled D8
CS_PIN > "arduino pin" 13  labeled D7
define > CLK_PIN "arduino pin" 12  labeled D6

2. Open the project in the Arduino IDE or your preferred platform.

3. Configure the necessary settings in the source code, such as UTC offset, hold interval, clock interval, and network credentials.

4. Compile and upload the code to the ESP8266 microcontroller.

5. Once the code is successfully uploaded, the ESP8266 will start running the clock and listen for UDP commands on the local network.

6. Use a UDP client or messaging application to send commands to the clock. Refer to the project's source code for the available commands and their syntax.

## Contributing

Contributions to this project are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

- Based on source code from https://techlogics.net/esp8266-clock-with-max7219-matrix-display-date-time-display/
- [Max7219 Led Matrix Display Library](https://github.com/wayoda/LedControl)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)

