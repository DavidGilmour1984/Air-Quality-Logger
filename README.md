# ESP32 Environmental + GPS Data Logger

## Overview

This project is an **ESP32-based environmental data logger** that collects and outputs data from multiple sensors as a **CSV stream over serial**.

It integrates:
- GPS positioning and time
- CO₂, temperature, and humidity
- Atmospheric pressure
- Particulate matter (PM1, PM2.5, PM10)

The system uses an **OpenLog module to record all data to an SD card** and includes a **multi-LED status system** for real-time diagnostics.

---

## Hardware Components

### Microcontroller
- ESP32 Dev Board

---

### Sensors

- GPS Module (UART)
- RTC Module (DS3231 via I2C)
- CO₂ Sensor (SCD41 via I2C)
- Pressure Sensor (BMP280 via I2C)
- Particulate Sensor (PMS5003 via UART)

---

### Data Logger (Required)

- OpenLog (SparkFun OpenLog or compatible)

---

### Connections

#### I2C Bus
- SDA → GPIO 21  
- SCL → GPIO 22  

#### GPS (UART2)
- RX → GPIO 26  
- TX → GPIO 25  

#### PMS Sensor (UART1)
- RX → GPIO 16  
- TX → GPIO 17  

#### OpenLog (Serial Logging)

- ESP32 TX (Serial output) → OpenLog RX  
- ESP32 GND → OpenLog GND  

(OpenLog TX is not required)

---

### LEDs (Status Indicators)

| Function     | GPIO |
|-------------|------|
| Data Flash   | 2    |
| Error        | 4    |
| GPS          | 13   |
| CO₂          | 12   |
| PM           | 14   |
| Pressure     | 27   |

---

## OpenLog Configuration

The OpenLog must be configured correctly to ensure clean CSV logging.
