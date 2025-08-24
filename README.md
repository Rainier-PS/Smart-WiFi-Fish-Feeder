# Smart-WiFi-Fish-Feeder
A smart Wi-Fi fish feeder built with an ESP32 development board and an SG90 servo. It is controlled via Telegram Bot commands and supports manual feeding, countdown timers, and daily scheduled feeding.

## Features

- **Wi-Fi connectivity** for remote control.
- **Telegram Bot interface**:
  - Manual feeding commands.
  - Countdown timer feeding.
  - Daily scheduled feeding.
  - Status reporting.
- **Authorization system**: only pre-approved Telegram users can control the feeder.
- **Time synchronization via NTP** for accurate scheduling.

## Hardware Requirements

- ESP32 development board
- Servo motor (connected to GPIO 13 by default)
- Stable Wi-Fi network
- Fish food dispensing mechanism (connected to servo)

## Software Requirements

- Arduino IDE with ESP32 board support installed
- Libraries:
  - `WiFi.h`
  - `WiFiClientSecure.h`
  - `UniversalTelegramBot.h`
  - `ESP32Servo.h`
  - `time.h`

## Setup Instructions

1. **Clone or download** this repository into your Arduino projects folder.
2. Open the `.ino` file in Arduino IDE.
3. Install required libraries through the Arduino Library Manager if not already installed.
4. Edit the following variables in the code:
   - `ssid` and `password`: your Wi-Fi credentials.
   - `botToken`: the Telegram Bot token from BotFather.
   - `authorizedUsers[]`: Telegram chat IDs of users allowed to control the feeder.
5. Upload the code to your ESP32.
6. After boot, the ESP32 will connect to Wi-Fi, synchronize time with NTP, and notify the first authorized user that the feeder is online.

## Telegram Commands

### General
- `/start` or `/help` – Show command list and usage.

### Manual Feeding
- `/feed1` – Dispense 1 portion
- `/feed2` – Dispense 2 portions
- `/feed3` – Dispense 3 portions
- `/feed4` – Dispense 4 portions
- `/feed5` – Dispense 5 portions

### Countdown Feeding
- `/countdown N [P]` – Feed after `N` seconds, with `P` portions (default = 1).  
  Example: `/countdown 30 2` → feeds 2 portions after 30 seconds.

### Scheduled Feeding
- `/schedule HH:MM [P]` – Feed daily at a specific time, with `P` portions (default = 1).  
  Example: `/schedule 08:30 3` → feeds 3 portions daily at 08:30.

### Status
- `/status` – Show Wi-Fi SSID, total portions dispensed since startup, and any active countdown or schedule.

## Notes

- Countdown feeding supports durations up to 3600 seconds (1 hour).
- Portions are limited between 1 and 5.
- Scheduling is based on device local time (set to UTC+7 in the code). Adjust in `setupTime()` if needed.
- Only users with IDs listed in `authorizedUsers[]` can issue commands.

## License

This project is licensed under the MIT License.
