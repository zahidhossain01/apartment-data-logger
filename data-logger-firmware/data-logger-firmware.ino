#include "Arduino.h"
#include "WiFi.h"
#include "time.h"
#include <DHT.h>
#include <HT_SSD1306Wire.h>

#define LED_PIN 35
#define DHT_PIN 1 // using as DHT22 pin

#define WIFI_SSID "ZA"
#define WIFI_PASSWORD "wlcs2001"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -8*3600;
const int daylightOffset_sec = 0;
struct tm timeinfo;


DHT dht(DHT_PIN, DHT22);

unsigned long lastMillis_dht;
unsigned long lastMillis_display;
unsigned long currentMillis;

float currentTemperature = 0;

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


    dht.begin();

    currentMillis = millis();
    lastMillis_dht = currentMillis;
    lastMillis_display = currentMillis;

}

void logTemp(){
    float temp = dht.readTemperature(true);
    currentTemperature = temp;
    Serial.println(temp);
}


void updateScreen(){
    screen.clear();
    
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

    if(currentMillis - lastMillis_dht >= 5000){
        logTemp();
        lastMillis_dht = currentMillis;
    }

    
}
