#include <TroykaGPS.h>

#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 5);

// serial-порт к которому подключён GPS-модуль
#define GPS_SERIAL mySerial

// создаём объект класса GPS и передаём в него объект GPS_SERIAL
GPS gps(GPS_SERIAL);

// -- Объем памяти для времени, даты, широты и долготы
#define MAX_SIZE_MASS 16

// -- Массив для хранения текущего времени
char strTime[MAX_SIZE_MASS];

// -- Массив для хранения текущей даты
char strDate[MAX_SIZE_MASS];

// -- Массив для хранения широты в градусах, минутах и секундах
char latitudeBase60[MAX_SIZE_MASS];

// -- Массив для хранения долготы в градусах, минутах и секундах
char longitudeBase60[MAX_SIZE_MASS];

// -- Кол-во итераций
int i = 0;

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
  }

//  Serial.print("Serial init OK\r\n");
  GPS_SERIAL.begin(115200);
//  Serial.println("GPS init is OK on speed 115200");
  GPS_SERIAL.write("$PMTK251,9600*17\r\n");
  GPS_SERIAL.end();
  GPS_SERIAL.begin(9600);
//  Serial.print("GPS init is OK on speed 9600");
//  GPS_SERIAL.write("$PMTK220,5000*1B");
}

void loop()
{
  // -- Если пришли данные с gps-модуля
  if (gps.available()) {
    // -- Считываем данные и парсим их
    gps.readParsing();
    switch (gps.getState()) {
      case GPS_OK:
        Serial.println("GPS is OK");
        Serial.print("Iteration ");
        Serial.println(i);
        // -- Выводим координаты широты и долготы
        // -- 1. В градусах, минутах и секундах
        // -- 2. Градусах в виде десятичной дроби
        Serial.println("GPS coordinates: ");
        gps.getLatitudeBase60(latitudeBase60, MAX_SIZE_MASS);
        gps.getLongitudeBase60(longitudeBase60, MAX_SIZE_MASS);
        Serial.print("Latitude:\t");
        Serial.print(latitudeBase60);
        Serial.print("\t\t");
        Serial.println(gps.getLatitudeBase10(), 12);
        Serial.print("Longitude:\t");
        Serial.print(longitudeBase60);
        Serial.print("\t\t");
        Serial.println(gps.getLongitudeBase10(), 12);

        // -- Выводим количество видимых спутников
        Serial.print("Count sets: ");
        Serial.println(gps.getSat());

        // -- Выводим текущую скорость
        Serial.print("Speed(km/h): ");
        Serial.println(gps.getSpeedKm());

        // -- Выводим высоту над уровнем моря
        Serial.print("Altitude: ");
        Serial.println(gps.getAltitude());

        // -- Выводим HDOP
        Serial.print("HDOP: ");
        Serial.println(gps.getHDOP());

        // -- Выводим текущее время
        Serial.print("Time: ");
        gps.getTime(strTime, MAX_SIZE_MASS);
        gps.getDate(strDate, MAX_SIZE_MASS);
        Serial.write(strTime);
        Serial.println();

        // -- Выводим текущую дату
        Serial.print("Data: ");
        Serial.write(strDate);
        Serial.println("\r\n");
        i++;
        break;

      // -- Обработка ошибки
      case GPS_ERROR_DATA:
        Serial.println("GPS error data");
        break;
      // -- Если нет соединения со спутниками
      case GPS_ERROR_SAT:
        Serial.println("GPS no connect to satellites!");
        break;
    }
  }
}
