# ESP32 Environmental + GPS Data Logger

## Overview
This project is an ESP32-based environmental data logger that outputs sensor data as CSV over serial and logs it using an OpenLog module.

Sensors included:
- GPS (position + time)
- SCD41 (CO₂, temperature, humidity)
- BMP280 (pressure + temperature)
- PMS5003 (PM1, PM2.5, PM10)
- DS3231 RTC (time backup)

---

## Hardware Required
- ESP32 Dev Board  
- OpenLog (required for logging)  
- GPS Module  
- SCD41 CO₂ Sensor  
- BMP280 Pressure Sensor  
- PMS5003 Particulate Sensor  
- DS3231 RTC  

---

## Wiring

### I2C
- SDA → GPIO 21  
- SCL → GPIO 22  

### GPS (UART2)
- RX → GPIO 26  
- TX → GPIO 25  

### PMS5003 (UART1)
- RX → GPIO 16  
- TX → GPIO 17  

### OpenLog (Recommended Setup)
- OpenLog RX → GPIO 17  
- OpenLog GND → ESP32 GND  

(OpenLog TX not required)

---

## OpenLog Configuration
Set config.txt to:

9600,26,3,0,0,0,0

- Baud: 9600  
- Mode: New file per power-up  
- Verbose: OFF  
- Echo: OFF  

---

## Code Requirement (IMPORTANT)

Use a dedicated UART for logging:

HardwareSerial LogSerial(1);
LogSerial.begin(9600, SERIAL_8N1, -1, 17);

Replace ALL:
Serial.print(...)
Serial.printf(...)

WITH:
LogSerial.print(...)
LogSerial.printf(...)

---

## Output Format (CSV)

time_ms,datetime,lat,lon,sats,hdop,co2,temp,humidity,bmpTemp,pressure,pm1,pm25,pm10,status

Example:
1234,2026-03-21 14:25:30,-37.123456,175.123456,8,0.9,420,21.5,55.2,20.8,1013.2,5,8,12,F

---

## How It Works

1. Setup:
- Initializes I2C, UARTs, and sensors
- Starts CO₂ measurements
- Prepares LEDs and timers

2. Loop:
- Continuously reads GPS data
- Parses PMS frames
- Reads SCD41 and BMP280 at set interval
- Syncs RTC using GPS when valid
- Outputs CSV data
- Flashes LED on data output

3. Serial Control:
Send:
anything,interval

Example:
set,0.5

→ updates logging interval (minimum 0.2 seconds)

---

## LED Indicators

| LED | Function |
|-----|--------|
| GPIO2 | Data flash |
| GPIO4 | Error |
| GPIO13 | GPS status |
| GPIO12 | CO₂ status |
| GPIO14 | PM status |
| GPIO27 | Pressure status |

- Error LED ON if any sensor fails  
- Sensor LEDs flash when valid  
- Data LED flashes on each log  
- 5-second heartbeat confirms system running  

---

## File Output

OpenLog creates:
LOG00001.TXT, LOG00002.TXT

Files contain valid CSV and can be opened directly in Excel or Sheets.

---

## Notes

- OpenLog is passive — ESP32 controls all formatting  
- Do NOT print debug data to logging UART  
- CO₂ sensor requires ~5s warm-up  
- GPS must be valid before RTC sync  
- Ensure stable power to avoid SD corruption  

---

## Use Cases
- Air quality monitoring  
- Environmental logging  
- Mobile sensing systems  
- Classroom experiments  

---
