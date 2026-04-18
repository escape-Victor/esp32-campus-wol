# DormWake32 (WIP)

An ESP32-based Wake-on-LAN (WoL) controller designed to remotely wake up a PC in a dormitory environment over the campus network. 

Currently a Work-In-Progress (WIP). The device successfully connects to the campus network, but is undergoing debugging for subsequent logic.

## Features
* **ESP32 Web Server**: Hosts a lightweight HTML UI for triggering the wake command.
* **Wake-on-LAN**: Constructs and broadcasts UDP Magic Packets to wake up the target machine.
* **Campus Network Ready**: Designed to handle campus network connection environments (Authentication logic in progress).

## Hardware Requirements
* ESP32 Development Board
* A target PC with a motherboard/NIC that supports Wake-on-LAN (WoL enabled in BIOS/UEFI)
* Network connection (Target PC must be connected to the router via Ethernet, or have modern Wireless WoL support)

## Project Structure
This project is built using [PlatformIO](https://platformio.org/).

* `src/` - Contains the main C/C++ source code (`main.cpp`).
* `include/` - Header files and configurations.
* `html/` - Web interface files for the ESP32 server.
* `platformio.ini` - PlatformIO project configuration and dependencies.

## Getting Started

1.  Clone this repository.
2.  Open the project in VS Code with the PlatformIO extension installed.
3.  Rename/configure your network credentials (SSID, Password, and any Portal Auth info) in the source code. *(Note: DO NOT commit your real credentials)*
4.  Configure the target PC's MAC address in the WoL broadcast function.
5.  Build and upload to your ESP32.

## TODO / Current Blockers
- [x] Connect to the campus WiFi network.
- [x] Bypass/Automate campus network portal authentication.
- [ ] Successfully broadcast the WoL magic packet across the local subnet.
- [ ] Finalize the Web UI integration.

## Hardware Setup

![ESP32 Hardware Setup](PLACEHOLDER_FOR_YOUR_PHOTO_HERE)
