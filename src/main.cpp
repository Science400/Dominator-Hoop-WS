/**
 * ----------------------------------------------------------------------------
 * Dominator Hoop Cycle Testing
 * ----------------------------------------------------------------------------
 * © 2021 Isaiah Young
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP_FlexyStepper.h>

// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------

#define LED_PIN 26
#define BTN_PIN 22
#define HTTP_PORT 80
#define LED_BUILTIN 2

// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

// WiFi credentials
const char *WIFI_SSID = "HomeSweetModem";
const char *WIFI_PASS = "silentwind943";

// Stepper Motor
const int MOTOR_STEP_PIN = 21;
const int MOTOR_DIRECTION_PIN = 22;

// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------

AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

ESP_FlexyStepper stepper;

bool startCycleBool = false;
int cycleQuantityRemaining = 0;
int cycleQuantityInitial = 0;
bool hoopDirection = false;

// Basketball Hoop Constants
float minHeight = 0.0; // mm
float maxHeight = 500; // mm
int stepsPerRevolution = 200;
int maxRevolutions = 4;
float clickDistance = 25.4;

// ----------------------------------------------------------------------------
// SPIFFS initialization
// ----------------------------------------------------------------------------

void initSPIFFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("Cannot mount SPIFFS volume...");
        while (1)
        {
        }
    }
}

// ----------------------------------------------------------------------------
// Connecting to the WiFi network
// ----------------------------------------------------------------------------

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}

// ----------------------------------------------------------------------------
// Web server initialization
// ----------------------------------------------------------------------------

String processor(const String &var)
{
    return String(var == "STATE" && true ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

void initWebServer()
{
    server.on("/", onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
}

// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------

void notifyClients()
{
    const uint8_t size = JSON_OBJECT_SIZE(2);
    StaticJsonDocument<size> json;
    json["cQR"] = cycleQuantityRemaining;
    json["cQI"] = cycleQuantityInitial;
    Serial.print("Cycles Remaining: ");
    Serial.println(cycleQuantityRemaining);

    char data[24];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {

        const uint8_t size = JSON_OBJECT_SIZE(2);
        StaticJsonDocument<size> json;
        DeserializationError err = deserializeJson(json, data);
        if (err)
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
        }

        const char *startCycle = json["startCycle"];
        int cycleQuantity = int(json["cycleQuantity"]);
        if (strcmp(startCycle, "true") == 0)
        {
            // led.on = !led.on;
            // notifyClients();
            Serial.print("Start: ");
            Serial.println(cycleQuantity);
            startCycleBool = true;
            cycleQuantityInitial = cycleQuantity;
            cycleQuantityRemaining = cycleQuantity;
        }
        if (strcmp(startCycle, "false") == 0)
        {
            // led.on = !led.on;
            // notifyClients();
            Serial.println("Stop!");
            startCycleBool = false;
            cycleQuantityRemaining = 0;
        }
    }
}

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
{

    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

// ----------------------------------------------------------------------------
// Process Countdown
// ----------------------------------------------------------------------------

void processCountdown()
{
    // Serial.println("Processing Countdown.");
    if (startCycleBool == true)
    {
        // Serial.println("Processing Motion.");
        if (cycleQuantityRemaining > 0)
        {
            // Serial.print("Cycle Quantity: ");
            // Serial.println(cycleQuantityRemaining);
            if (stepper.motionComplete() == true)
            {
                hoopDirection = !hoopDirection;
                if (hoopDirection == true)
                {
                    stepper.setTargetPositionInMillimeters(maxHeight);
                    Serial.println("Going up!");
                    notifyClients();
                }
                else
                {
                    stepper.setTargetPositionInMillimeters(minHeight);
                    Serial.println("Down we go!");
                    cycleQuantityRemaining--;
                }
            }
            else
            {
                stepper.processMovement();
            }
        }
        else
        {
            notifyClients();
            startCycleBool = false;
        }
    }
}

// ----------------------------------------------------------------------------
// Stepper Motor Initialization
// ----------------------------------------------------------------------------
void initStepperMotor()
{
    stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
    stepper.setStepsPerMillimeter(stepsPerRevolution * maxRevolutions / (maxHeight - minHeight));
    stepper.setSpeedInMillimetersPerSecond(100.0);
    stepper.setAccelerationInMillimetersPerSecondPerSecond(50.0);
    stepper.setDecelerationInMillimetersPerSecondPerSecond(50.0);
}

// // ----------------------------------------------------------------------------
// // Task Initialization
// // ----------------------------------------------------------------------------

// void processMotionCallback();

// Task processMotion()

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(500);

    initSPIFFS();
    initWiFi();
    initWebSocket();
    initWebServer();
    initStepperMotor();
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop()
{
    ws.cleanupClients();

    processCountdown();
}