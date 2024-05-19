#include "Arduino.h"
#include "WiFi.h"
#include "time.h"
#include <Adafruit_BME280.h>
#include <HT_SSD1306Wire.h>

#define LED_PIN 35

#define BME_SCL 47
#define BME_SDA 48
#define SEALEVELPRESSURE_HPA (1013.25)

#define WIFI_SSID "ZA"
#define WIFI_PASSWORD "wlcs2001"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8*3600;
const int daylightOffset_sec = 3600;
struct tm timeinfo;

unsigned long lastMillis_bme;
unsigned long lastMillis_display;
unsigned long currentMillis;

float currentTemperature = 0;

TwoWire BME_Wire(1);
Adafruit_BME280 bme; // I2C

SSD1306Wire  screen(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void VextON(void) {
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext, LOW);
}
void VextOFF(void) {
    // Vext default off
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext, HIGH);
}


void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    // digitalWrite(LED_PIN, HIGH); // LED

    VextON();
    delay(100);
    screen.init();
    screen.clear();
    screen.display();

    screen.setFont(ArialMT_Plain_10);
    screen.drawString(0, 0, "Temperature logger \\^-^/ !");
    screen.display();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi with IP: ");
    Serial.println(WiFi.localIP());

    // without this line, the getLocalTime() function was taking longer than it should have
    // TODO: check if getLocalTime needs internet or not after this initial config (ideally no internet needed just to track time)
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    BME_Wire.begin(BME_SDA, BME_SCL, 100000);
    bool bmestatus = bme.begin(0x76, &BME_Wire);
    // TODO: bme.init()? bme.setSampling?()
    // .getTemperatureCompensation(), .setTemperatureCompensation()
    if (!bmestatus) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");

        screen.clear();
        screen.drawString(0, 0, "ERROR: BME280 SENSOR MISSING?");
        screen.display();

        while (1) { delay(10); }
    }

    currentMillis = millis();
    lastMillis_bme = currentMillis;
    lastMillis_display = currentMillis;

}

void logTemp(){
    float temp_c = bme.readTemperature();
    float temp_f = 1.8*temp_c + 32;
    currentTemperature = temp_f;
    Serial.println(temp_f);
}


void updateScreen(){
    screen.clear();
    
    // blinking dude
    // static bool displayState = false;
    // if(displayState){
    //     screen.setFont(ArialMT_Plain_10);
    //     screen.drawString(0,0, "O_o" );
    // }
    // displayState = !displayState;


    char temp_string[30];
    char timestamp[50];
    char time_display_buf[50]; // could just use timestamp instead... but I wanna keep timestamp separate for use in database logging

    snprintf(temp_string, sizeof(temp_string), "Temp:  %.2f [ºF]", currentTemperature);
    screen.drawString(0, 0, temp_string);

    getLocalTime(&timeinfo);
    strftime(timestamp, 50, "%F %H:%M:%S GMT%z", &timeinfo);
    Serial.println(timestamp);

    strftime(time_display_buf, 50, "%I:%M:%S %p", &timeinfo);
    screen.setFont(ArialMT_Plain_16);
    screen.drawString(0, 22, time_display_buf);

    strftime(time_display_buf, 50, "%A, %b %e %Y", &timeinfo);
    screen.setFont(ArialMT_Plain_10);
    screen.drawString(0, 22+19, time_display_buf);

    screen.display();
}

void loop() {

    currentMillis = millis();

    if(currentMillis - lastMillis_display >= 1000){
        updateScreen();
        lastMillis_display = currentMillis;
    }

    if(currentMillis - lastMillis_bme >= 5000){
        logTemp();
        lastMillis_bme = currentMillis;
    }

    // TODO: pwm fade/pulse the LED? using espressif ledc code?
    // https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/ledc.html
}
