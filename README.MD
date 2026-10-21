# Public Transport Information and Interaction on ESP32

A departure display for public transport in the Karlskrona region, running on a LilyGO T4-S3 (ESP32-S3 with AMOLED touchscreen). The device shows upcoming departures for a chosen stop, using live data from the Trafiklab Realtime API.

Project in **PA1484 Software Engineering**, Blekinge Institute of Technology (BTH), autumn 2026. **Group 12:** Albert Rogö, Anton Öhlander, Felix Forsman, Jeremy Andersson, Paulius Galiauskas.

## Status

| Version | Content | Status |
| --- | --- | --- |
| v0.4.0 | Start, settings and departure screens with demo data, swipe navigation (FR-01, FR-03, FR-04) | Done |
| v0.4.1 | Trafiklab test request for all stops, output in the serial monitor (part of FR-02) | Done |
| v0.5.0 | Live departures on the departure screen (FR-02) | Planned, Sprint 1 |

Work is planned and tracked on our [Scrum board](https://github.com/users/fefo25bth/projects/1).

## Screens

Swipe left and right to move between the screens:

1. **Start screen:** project name, firmware version, group number and members.
2. **Settings:** choose a stop and a transport type (all, bus, train, ferry).
3. **Departures:** upcoming departures for the chosen stop. Delayed departures are shown in yellow, cancelled ones in red.

Stops: Campus Gräsvik, Karlskrona C, Karlskrona centrum, Handelshamnen, Lyckeby centrum.

## Setup

1. Install Visual Studio Code and the PlatformIO IDE extension
2. Clone the repository:\
   git clone https://github.com/fefo25bth/PA1484\_Project1.git
3. Copy `project/secrets.example.h` to `project/secrets.h` and fill in:
    - `WIFI_SSID` and `WIFI_PASSWORD`: a Wi-Fi network or phone hotspot
    - `TRAFIKLAB_API_KEY`: a key for **Trafiklab Realtime APIs** from [developer.trafiklab.se](https://developer.trafiklab.se).
4. `secrets.h` is listed in `.gitignore` and must never be committed.
5. Open `project/project.ino`, then build and upload to the board.
6. Open the serial monitor at 115200 baud to see Wi-Fi and Trafiklab status messages.

## Project structure

| Path | Content |
| --- | --- |
| `project/` | Our application code (`project.ino`) and the secrets template |
| `docs/` | Project documentation: coding standard and test log |
| `src/`, `boards/`, `platformio.ini` | Course template and board support. Not changed by us |

## How we work

- All changes go through a branch and a pull request with one approving review. `main` is protected.
- Code follows our [coding standard](docs/coding-standard.md).
- Test results are recorded in the [test log](docs/testing.md).

## Data source and attribution

Departure data: Trafiklab.se, licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

---

*The rest of this README is the original guide from the course template.*

# How to get started

This project will work regardless of the operating system you use, and there are appended guides for each below. Each of these tutorials is done on a fresh install of Visual Studio Code, which is the IDE you must use to complete this project. Setting up and configuring the project will take multiple minutes, up to 15 min so be patient with the software.

## PlatformIO

1. Install Visual Studio Code
    - Visit [Visual Studio Code's website](https://code.visualstudio.com/download) and download the latest version or use the package manager of your system.
    - Run `Visual Studio Code` and follow the steps.
2. Install the correct extension.
    - Head over to the extensions tab on your left.
    - Search for ["PlatformIO IDE"](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) and install it.

## How to run the program

1. Open the locally cloned repository with Visual Studio Code
    - If the "Do you trust the authors of the files in this folder?" dialog appears, click on "Yes, I trust the authors"
2. Open up the file `project/project.ino`
3. Connect your ESP32 to your computer via a USB cable.
4. Build and upload the project to the device. See screenshot.

screenshot

---

# What to add where

Your code belongs in the `/project` folder. The only place you should add, change, or remove things from is the [**/project**](project/project.ino) folder. Changing anything else might break the code and cause a lot of headaches for all involved parties. The `project.ino` file is the entry point for the application.

You are meant to write a small application for the computer that retrieves the weather for a given location, and most of that code belongs in your own functions and the `void loop()` function at the bottom of the `project.ino` file.

## How to Connect to WiFi

The T4-S3 has built-in Wi-Fi, but it cannot connect to eduroam. Use another network or a phone hotspot.

Add or modify the `project/secrets.h` file, then set the SSID (network name) and password.

```
static const char* WIFI_SSID     = "SSID";
static const char* WIFI_PASSWORD = "PWD";
```

Never commit real credentials to GitHub. `project/secrets.h` is excluded by the supplied `.gitignore`.

---

# Troubleshooting

## The screen still shows the old interface, and touch does nothing

The board is probably still in upload mode. Tap **RESET/RST once**. A retained image on the AMOLED does not mean the application is running.

## Upload cannot connect

1. Stop PlatformIO Monitor with `Ctrl+C`.
2. Disconnect and reconnect the USB cable if the port is locked.
3. Repeat the BOOT/RESET upload-mode sequence.
4. Start Upload again.

## The screen is black

Open PlatformIO Monitor after resetting. Startup errors are printed at 115200 baud and also stored as `/debug.log` on the device.

## LilyGo tutorial

Visit LilyGO [link](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series)
