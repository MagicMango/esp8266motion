#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPHue.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>

int sensor = D7;  // Digital pin D7

WiFiClient client;
ESPHue hue = ESPHue(client, "[Hue user]", "[Hue ip]", 80);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
static int updatePeriod = 60 * 1000;
static unsigned long time_now1 = 0;
static int hueLightId = 2;
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
static long state;
static int lastHueState;

void setup() {
	Serial.begin(115200);
	WiFi.begin("[SSID]", "[Password]");     //Connect to your WiFi router
	WiFi.mode(WIFI_STA);
	Serial.println("");
	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(F("."));
	}
	Serial.println();
	Serial.println("Connnected with IP: " + WiFi.localIP().toString());
	server.on("/getCurrentState", getState);
	httpUpdater.setup(&server, update_path, update_username, update_password);
	server.begin();
	pinMode(sensor, INPUT);   // declare sensor as input
	lastHueState = hue.getLightState(hueLightId);
}

void loop() {
	server.handleClient();
	state = digitalRead(sensor);
	if (state == HIGH && lastHueState == hue.OFF) {
		Serial.println(F("Motion detected and lights are off!"));
		hue.setLightPower(hueLightId, hue.ON);
		lastHueState = hue.ON;
		Serial.println("Switched Light: "+String(hueLightId)+" on.");
	}
	if (millis() > time_now1 + updatePeriod) {
		time_now1 = millis();
		lastHueState = hue.getLightState(hueLightId);
		if (lastHueState == hue.ON && state == LOW) {
			Serial.println(F("Motion absent and Light is on!"));
			hue.setLightPower(hueLightId, hue.OFF);
			lastHueState = hue.OFF;
			Serial.println("Switched Light: " + String(hueLightId) + " off.");
		}
	}
}

void getState() {
	server.send(200, "application/json", "{ \"motion\":" + String(state)+", \"lightState\":"+String(lastHueState)+"}");
}
