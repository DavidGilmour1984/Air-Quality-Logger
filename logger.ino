#include <Wire.h>
#include <TinyGPSPlus.h>
#include <RTClib.h>
#include <Adafruit_BMP280.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

/* ---------------- PINS ---------------- */

#define GPS_RX 26
#define GPS_TX 25

#define PMS_RX 16
#define PMS_TX 17

#define SDA_PIN 21
#define SCL_PIN 22

/* ---------------- OBJECTS ---------------- */

HardwareSerial GPS(2);
HardwareSerial PMS(1);

TinyGPSPlus gps;
RTC_DS3231 rtc;
Adafruit_BMP280 bmp;
SCD4x scd41;

/* ---------------- STATE ---------------- */

bool rtcFound = false;
bool bmpFound = false;
bool scdFound = false;

int gpsValidCount = 0;

/* ---------------- TIMING ---------------- */

unsigned long startTime = 0;
unsigned long lastReadTime = 0;

float interval = 1.0;
bool printTime = true;

/* ---------------- DATA ---------------- */

float co2 = 0;
float scdTemp = 0;
float humidity = 0;

float bmpTemp = 0;
float pressure = 0;

uint16_t pm1 = 0;
uint16_t pm25 = 0;
uint16_t pm10 = 0;

/* ---------------- PMS PARSER ---------------- */

#define PMS_FRAME_LEN 32
#define PMS_H1 0x42
#define PMS_H2 0x4D

uint8_t frame[PMS_FRAME_LEN];

uint16_t u16(int i)
{
  return (uint16_t(frame[i]) << 8) | frame[i + 1];
}

bool readPMS()
{
  static uint8_t idx = 0;

  while (PMS.available())
  {
    uint8_t b = PMS.read();

    if (idx == 0 && b != PMS_H1) continue;
    if (idx == 1 && b != PMS_H2)
    {
      idx = 0;
      continue;
    }

    frame[idx++] = b;

    if (idx == PMS_FRAME_LEN)
    {
      idx = 0;

      if (u16(2) != 28) return false;

      uint16_t sum = 0;
      for (int i = 0; i < PMS_FRAME_LEN - 2; i++)
        sum += frame[i];

      if (sum != u16(30)) return false;

      pm1  = u16(4);
      pm25 = u16(6);
      pm10 = u16(8);

      return true;
    }
  }
  return false;
}

/* ---------------- SETUP ---------------- */

void setup()
{
  Serial.begin(9600);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  if (rtc.begin()) rtcFound = true;
  if (bmp.begin(0x76)) bmpFound = true;

  if (scd41.begin())
  {
    scdFound = true;
    scd41.stopPeriodicMeasurement();
    delay(500);
    scd41.startPeriodicMeasurement();
    delay(5000);
  }

  GPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  PMS.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);

  startTime = millis();
}

/* ---------------- LOOP ---------------- */

void loop()
{
  readGPS();
  syncRTCfromGPS();
  handleSerial();
  readPMS();

  unsigned long now = millis();
  unsigned long elapsed = now - startTime;

  if (elapsed - lastReadTime >= (interval * 1000))
  {
    lastReadTime = elapsed;

    readSCD();
    readBMP();

    outputCSV(elapsed);
  }
}

/* ---------------- GPS ---------------- */

void readGPS()
{
  while (GPS.available())
  {
    gps.encode(GPS.read());
  }
}

/* ---------------- RTC SYNC ---------------- */

void syncRTCfromGPS()
{
  if (!rtcFound) return;

  if (!gps.date.isValid() || !gps.time.isValid())
  {
    gpsValidCount = 0;
    return;
  }

  if (gps.satellites.value() < 4 || gps.hdop.hdop() > 5.0)
  {
    gpsValidCount = 0;
    return;
  }

  gpsValidCount++;

  if (gpsValidCount > 3)
  {
    DateTime rtcNow = rtc.now();

    DateTime gpsTime(
      gps.date.year(),
      gps.date.month(),
      gps.date.day(),
      gps.time.hour(),
      gps.time.minute(),
      gps.time.second()
    );

    int32_t diff = gpsTime.unixtime() - rtcNow.unixtime();

    if (abs(diff) > 1)
    {
      rtc.adjust(gpsTime);
    }
  }
}

/* ---------------- SCD40 ---------------- */

void readSCD()
{
  if (!scdFound) return;

  if (scd41.getDataReadyStatus())
  {
    if (scd41.readMeasurement())
    {
      co2 = scd41.getCO2();
      scdTemp = scd41.getTemperature();
      humidity = scd41.getHumidity();
    }
  }
}

/* ---------------- BMP ---------------- */

void readBMP()
{
  if (!bmpFound) return;

  bmpTemp = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0;
}

/* ---------------- SERIAL CONTROL ---------------- */

void handleSerial()
{
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  int commaIndex = input.indexOf(',');

  if (commaIndex > 0)
  {
    int resetFlag = input.substring(0, commaIndex).toInt();
    float newInterval = input.substring(commaIndex + 1).toFloat();

    startTime = millis();
    lastReadTime = 0;
    printTime = (resetFlag == 1);

    if (newInterval >= 0.2) interval = newInterval;
  }
}

/* ---------------- OUTPUT ---------------- */

void outputCSV(unsigned long elapsed)
{
  DateTime now = rtc.now();

  float lat = gps.location.isValid() ? gps.location.lat() : 0;
  float lon = gps.location.isValid() ? gps.location.lng() : 0;
  int sats = gps.satellites.value();
  float hdop = gps.hdop.hdop();

  Serial.printf(
    "%lu,%04d-%02d-%02d %02d:%02d:%02d,%.6f,%.6f,%d,%.2f,%.1f,%.1f,%.1f,%.1f,%.1f,%u,%u,%u,F\n",
    elapsed,
    now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second(),
    lat, lon,
    sats,
    hdop,
    co2,
    scdTemp,
    humidity,
    bmpTemp,
    pressure,
    pm1,
    pm25,
    pm10
  );
}
