#include <DNSServer.h>
#include <ESPUI.h>
#include <WiFi.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

const char *ssid = "matrocob";
const char *password = "pleomax1";
const char *hostname = "espui";

#define BOARD_LED 2
#define RELAY_SOIL1 33
#define RELAY_SOIL2 25
#define RELAY_SOIL3 26
#define RELAY_SOIL4 27
#define RELAY_SOIL5 14
#define RELAY_SOIL6 12

#define SOIL_PIN2 32
#define SOIL_PIN3 35
#define SOIL_PIN4 34

const int dry = 0;
const int wet = 4095;

int webSoil2;
int webSoil3;
int webSoil4;

void updateWatering(int soilPin, int relayPin, int webSoil)
{

    int soilValue = analogRead(soilPin);
    int dryPercent = map(soilValue, wet, dry, 100, 0);
    Serial.print("Type=" + String(webSoil));
    Serial.print(" soil=");
    Serial.print(soilValue);
    Serial.print(" soil percent=");
    Serial.print(dryPercent);
    Serial.println();

    ESPUI.updateText(webSoil, ("value=" + String(soilValue) + " percent" + String(dryPercent) + "%"));

    if (dryPercent != 0 && dryPercent > 60){
        digitalWrite(relayPin, HIGH);
    } else {
        digitalWrite(relayPin, LOW);
    }
}

void setup(void)
{
    pinMode(BOARD_LED, OUTPUT);
    // pinMode(RELAY_SOIL1, OUTPUT);
    pinMode(RELAY_SOIL2, OUTPUT);
    pinMode(RELAY_SOIL3, OUTPUT);
    pinMode(RELAY_SOIL4, OUTPUT);
    // pinMode(RELAY_SOIL5, OUTPUT);
    // pinMode(RELAY_SOIL6, OUTPUT);

    ESPUI.setVerbosity(Verbosity::VerboseJSON);
    Serial.begin(115200);

    WiFi.setHostname(hostname);

    // try to connect to existing network
    WiFi.begin(ssid, password);
    Serial.print("\n\nTry to connect to existing network");

    {
        uint8_t timeout = 10;

        // Wait for connection, 5s timeout
        do
        {
            delay(500);
            Serial.print(".");
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);

        // not connected -> create hotspot
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            delay(100);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
            uint32_t chipid = 0;
            for (int i = 0; i < 17; i = i + 8)
            {
                chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
            }

            char ap_ssid[25];
            snprintf(ap_ssid, 26, "ESPUI-%08X", chipid);
            WiFi.softAP(ap_ssid);

            timeout = 5;

            do
            {
                delay(2000);
                Serial.print(".");
                timeout--;
            } while (timeout);
        }
    }

    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("\n\nWiFi parameters:");
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
    Serial.print("IP address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());

    webSoil2 = ESPUI.label("Soil2:", ControlColor::Turquoise, "0");
    webSoil3 = ESPUI.label("Soil3:", ControlColor::Emerald, "0");
    webSoil4 = ESPUI.label("Soil4:", ControlColor::Emerald, "0");

    ESPUI.begin("ESPUI Control");
}

void loop(void)
{
    dnsServer.processNextRequest();

    digitalWrite(BOARD_LED, HIGH);
    delay(2000);

    updateWatering(SOIL_PIN2, RELAY_SOIL2, webSoil2);
    updateWatering(SOIL_PIN3, RELAY_SOIL3, webSoil3);
    updateWatering(SOIL_PIN4, RELAY_SOIL4, webSoil4);
    Serial.println();

    digitalWrite(BOARD_LED, LOW);
    delay(2000);

}