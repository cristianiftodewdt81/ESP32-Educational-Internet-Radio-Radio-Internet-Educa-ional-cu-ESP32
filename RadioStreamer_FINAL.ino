#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <VS1053.h>
#include <driver/i2s.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "WebInterface.h"

// --- CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define VS1053_CS    5
#define VS1053_DCS   16
#define VS1053_DREQ  4
#define VS1053_RESET 0 

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
Preferences preferences;

// --- CIRCULAR BUFFER (64KB) ---
#define BUFFER_SIZE 65536
#define START_THRESHOLD 24576
uint8_t staticBuffer[BUFFER_SIZE]; 
volatile uint32_t head = 0;
volatile uint32_t tail = 0;
volatile uint32_t storedCount = 0;

// --- STREAM ---
WiFiClient streamClient;
bool isPlaying = false;
volatile bool shouldStop = false;
String currentStationName = ""; 
String currentStationURL = ""; 
bool pendingStationChange = false;
bool audioEnhancementApplied = false;  // SCI_AUDAT se aplică abia după ce decodarea a început

// Defaults - 13 stații
const char* defaultName1 = "Hit IASI";
const char* defaultURL1 = "http://136.255.208.182:8000/hit.mp3";
const char* defaultName2 = "Bu FM";
const char* defaultURL2 = "http://stream2.srr.ro:8032";
const char* defaultName3 = "R Cultural";
const char* defaultURL3 = "http://stream2.srr.ro:8012";
const char* defaultName4 = "R Iasi AM";
const char* defaultURL4 = "http://89.238.227.6:8200";
const char* defaultName5 = "R Iasi FM";
const char* defaultURL5 = "http://stream2.srr.ro:8202";
const char* defaultName6 = "R Pro FM";
const char* defaultURL6 = "http://edge126.rdsnet.ro:84/profm/profm.mp3";
const char* defaultName7 = "Radio ZU";
const char* defaultURL7 = "http://zuicast.digitalag.ro:9420/zu";
const char* defaultName8 = "Digi FM";
const char* defaultURL8 = "http://edge76.rdsnet.ro:84/digifm/digifm.mp3";
const char* defaultName9 = "Kiss FM";
const char* defaultURL9 = "https://live.kissfm.ro/kissfm.aacp";
const char* defaultName10 = "R Actualitati";
const char* defaultURL10 = "http://stream2.srr.ro:8000/rra";
const char* defaultName11 = "R Trinitas";
const char* defaultURL11 = "http://live.radiotrinitas.ro:8000/";
const char* defaultName12 = "Magic FM";
const char* defaultURL12 = "https://live.kissfm.ro/magicfm.aacp";
const char* defaultName13 = "Europa FM";
const char* defaultURL13 = "http://astreaming.edi.ro:8000/EuropaFM_aac"; 

String host = "";
String path = "";
int port = 80;
int currentVolume = 96;  // Scală librarie: 0=tăcut, 127=max. 96 = ~75%
volatile int newVolume = -1;
int currentTreble = 8;   // 0-15, default 8
int currentBass   = 10;  // 0-15, default 10
volatile bool newAudatPending = false;  // flag: loop() scrie registrul
unsigned long lastUiUpdate = 0;
unsigned long lastConnectAttempt = 0;
unsigned long lastDataTime = 0; // WATCHDOG: Track last time we got data

AsyncWebServer server(80);
bool shouldConnectWifi = false;
String wifiSSID = "";
String wifiPass = "";

// --- MICROPHONE I2S ---
#define I2S_WS 15
#define I2S_SD 13
#define I2S_SCK 2
#define I2S_PORT I2S_NUM_0

void parseUrl(String url) {
    int idx = url.indexOf("://");
    if(idx > 0) url = url.substring(idx+3);
    
    int colon = url.indexOf(':');
    int slash = url.indexOf('/');
    
    if(colon > 0) {
        host = url.substring(0, colon);
        port = url.substring(colon+1, slash).toInt();
    } else {
        host = url.substring(0, slash);
        port = 80;
    }
    path = url.substring(slash);
    if(path.length() == 0) path = "/";
}

void saveLastStation() {
    if(currentStationName.length() > 0 && currentStationURL.length() > 0) {
        preferences.begin("radio", false);
        preferences.putString("lastStation", currentStationName);
        preferences.putString("lastURL", currentStationURL);
        preferences.putInt("lastVolume", currentVolume);  // Salvăm și volumul
        preferences.putInt("lastTreble", currentTreble);
        preferences.putInt("lastBass", currentBass);
        preferences.end();
        Serial.println("Saved last station: " + currentStationName);
        Serial.print("Saved volume: ");
        Serial.println(currentVolume);
    }
}

void loadLastStation() {
    preferences.begin("radio", true);
    String savedName = preferences.getString("lastStation", "");
    String savedURL = preferences.getString("lastURL", "");
    int savedVolume = preferences.getInt("lastVolume", 96);  // Default 96 (valoare inițială)
    int savedTreble = preferences.getInt("lastTreble", 8);
    int savedBass   = preferences.getInt("lastBass", 10);
    preferences.end();
    
    if(savedName.length() > 0 && savedURL.length() > 0) {
        currentStationName = savedName;
        currentStationURL = savedURL;
        currentVolume = savedVolume;
        currentTreble = savedTreble;
        currentBass   = savedBass;
        Serial.println("Loaded last station: " + currentStationName);
        Serial.print("Loaded volume: ");
        Serial.println(currentVolume);
    }
}

void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "[";
        int n = WiFi.scanNetworks();
        for(int i=0; i<n; ++i){
            if(i) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("ssid") && request->hasParam("pass")) {
            wifiSSID = request->getParam("ssid")->value();
            wifiPass = request->getParam("pass")->value();
            shouldConnectWifi = true; 
            request->send(200, "text/plain", "Connecting...");
        } else {
            request->send(400, "text/plain", "Missing params");
        }
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"ssid\":\"" + WiFi.SSID() + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
        json += "\"playing\":" + String(isPlaying ? "true" : "false") + ",";
        json += "\"station\":\"" + currentStationName + "\",";
        json += "\"volume\":" + String(currentVolume) + ",";
        json += "\"treble\":" + String(currentTreble) + ",";
        json += "\"bass\":" + String(currentBass);
        json += "}";
        request->send(200, "application/json", json);
    });

    server.on("/volume", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("val")) {
            int webPercent = constrain(request->getParam("val")->value().toInt(), 0, 100);
            // Librarie VS1053: setVolume(0)=tăcut, max funcțional=100
            // Mapare directă: slider 0% → 0, slider 100% → 100
            newVolume = webPercent;
            Serial.print("Volume: "); Serial.println(webPercent);
            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Missing val");
        }
    });

    server.on("/eq", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("treble") && request->hasParam("bass")) {
            currentTreble = constrain(request->getParam("treble")->value().toInt(), 0, 15);
            currentBass   = constrain(request->getParam("bass")->value().toInt(), 0, 15);
            newAudatPending = true;  // loop() va scrie registrul
            
            // Salvăm în Preferences
            preferences.begin("radio", false);
            preferences.putInt("lastTreble", currentTreble);
            preferences.putInt("lastBass", currentBass);
            preferences.end();
            
            Serial.print("EQ: treble="); Serial.print(currentTreble);
            Serial.print(" bass="); Serial.println(currentBass);
            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Missing treble or bass");
        }
    });

    // STREAMS API
    server.on("/streams", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("action")) {
            String action = request->getParam("action")->value();
            if(action == "add") {
                 if(request->hasParam("name") && request->hasParam("url")) {
                    File f = SPIFFS.open("/stations.json", "r");
                    JsonDocument doc;
                    if(f) {
                        deserializeJson(doc, f);
                        f.close();
                    }
                    
                    JsonObject obj = doc.add<JsonObject>();
                    obj["name"] = request->getParam("name")->value();
                    obj["url"] = request->getParam("url")->value();
                    
                    File fw = SPIFFS.open("/stations.json", "w");
                    serializeJson(doc, fw);
                    fw.close();
                    request->send(200, "text/plain", "Added");
                 }
            } else if(action == "del") {
                 if(request->hasParam("index")) {
                     int idx = request->getParam("index")->value().toInt();
                     File f = SPIFFS.open("/stations.json", "r");
                     JsonDocument doc;
                     if(f) {
                         deserializeJson(doc, f);
                         f.close();
                     }
                     JsonArray arr = doc.as<JsonArray>();
                     arr.remove(idx);
                     File fw = SPIFFS.open("/stations.json", "w");
                     serializeJson(doc, fw);
                     fw.close();
                     request->send(200, "text/plain", "Deleted");
                 }
            }
        } else {
            if(SPIFFS.exists("/stations.json")) {
                request->send(SPIFFS, "/stations.json", "application/json");
            } else {
                 request->send(200, "application/json", "[]");
            }
        }
    });

    server.on("/play", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("url") && request->hasParam("name")) {
            String newURL = request->getParam("url")->value();
            String newName = request->getParam("name")->value();
            
            Serial.println("=== PLAY REQUEST ===");
            Serial.print("Statie: ");
            Serial.println(newName);
            Serial.print("URL: ");
            Serial.println(newURL);
            
            currentStationURL = newURL;
            currentStationName = newName;
            shouldStop = false;
            pendingStationChange = true;
            
            // Salvăm stația pentru următoarea pornire
            saveLastStation();
            
            request->send(200, "text/plain", "Playing: " + newName);
        } else {
            request->send(400, "text/plain", "Missing name or URL");
        }
    });
    
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("=== STOP REQUEST ===");
        shouldStop = true;
        request->send(200, "text/plain", "Stopped");
    });
    
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Resetting to defaults...");
        delay(500);
        
        // Ștergem fișierul cu stațiile salvate
        if(SPIFFS.exists("/stations.json")) {
            SPIFFS.remove("/stations.json");
            Serial.println("Deleted stations.json - will recreate defaults");
        }
        
        // Resetăm WiFi
        WiFi.disconnect(true, true);
        delay(500);
        
        // La reboot, setup() va recrea cele 13 stații default
        ESP.restart();
    });

    server.begin();
}

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, NULL);
}

void setup() {
    Serial.begin(115200);
    
    if(!SPIFFS.begin(true)){ 
        Serial.println("SPIFFS Fail"); 
    }
    
    // Default stations if missing
    if(!SPIFFS.exists("/stations.json")) {
        Serial.println("Creating default stations...");
        File f = SPIFFS.open("/stations.json", "w");
        if(f) {
            JsonDocument doc;
            JsonArray arr = doc.to<JsonArray>();
            
            JsonObject o1 = arr.add<JsonObject>();
            o1["name"] = defaultName1; o1["url"] = defaultURL1;
            
            JsonObject o2 = arr.add<JsonObject>();
            o2["name"] = defaultName2; o2["url"] = defaultURL2;
            
            JsonObject o3 = arr.add<JsonObject>();
            o3["name"] = defaultName3; o3["url"] = defaultURL3;
            
            JsonObject o4 = arr.add<JsonObject>();
            o4["name"] = defaultName4; o4["url"] = defaultURL4;
            
            JsonObject o5 = arr.add<JsonObject>();
            o5["name"] = defaultName5; o5["url"] = defaultURL5;
            
            JsonObject o6 = arr.add<JsonObject>();
            o6["name"] = defaultName6; o6["url"] = defaultURL6;
            
            JsonObject o7 = arr.add<JsonObject>();
            o7["name"] = defaultName7; o7["url"] = defaultURL7;
            
            JsonObject o8 = arr.add<JsonObject>();
            o8["name"] = defaultName8; o8["url"] = defaultURL8;
            
            JsonObject o9 = arr.add<JsonObject>();
            o9["name"] = defaultName9; o9["url"] = defaultURL9;
            
            JsonObject o10 = arr.add<JsonObject>();
            o10["name"] = defaultName10; o10["url"] = defaultURL10;
            
            JsonObject o11 = arr.add<JsonObject>();
            o11["name"] = defaultName11; o11["url"] = defaultURL11;
            
            JsonObject o12 = arr.add<JsonObject>();
            o12["name"] = defaultName12; o12["url"] = defaultURL12;
            
            JsonObject o13 = arr.add<JsonObject>();
            o13["name"] = defaultName13; o13["url"] = defaultURL13;
            
            serializeJson(doc, f);
            f.close();
            Serial.println("Created 13 default stations");
        }
    }
    
    // Încărcăm ultima stație
    loadLastStation();
    
    pinMode(VS1053_RESET, OUTPUT);
    digitalWrite(VS1053_RESET, LOW); delay(100);
    digitalWrite(VS1053_RESET, HIGH); delay(200);

    SPI.begin();
    player.begin();
    player.switchToMp3Mode();
    player.setVolume(currentVolume);

    setupI2S();

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("OLED Fail")); 
    } else {
        Serial.println("OLED OK");
    }
    display.clearDisplay();
    display.display();
    
    // WiFi Setup
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    
    // Start Access Point
    WiFi.softAP("R_WDT81", "123456789");
    Serial.println("AP Started: R_WDT81");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    
    // Try to connect to saved WiFi
    WiFi.begin();
    Serial.println("Trying saved WiFi...");
    
    setupWebServer();
    
    // Parse URL doar dacă avem o stație salvată
    if(currentStationURL.length() > 0) {
        parseUrl(currentStationURL);
        // Setăm flag pentru auto-play
        pendingStationChange = true;
        Serial.println("Auto-play enabled for last station");
    }
    
    Serial.println("Setup complete!");
}

void loop() {
    // STOP - prioritate maximă
    if(shouldStop) {
        shouldStop = false;
        
        Serial.println("=== STOPPING STREAM ===");
        
        isPlaying = false;
        streamClient.stop();
        
        player.softReset();
        delay(200);
        player.switchToMp3Mode();
        delay(50);
        player.setVolume(currentVolume);
        audioEnhancementApplied = false;
        
        storedCount = 0; 
        head = 0; 
        tail = 0;
        
        currentStationName = "Stopped";
        
        Serial.println("Stream stopped successfully!");
    }

    // Station Change Logic
    if(pendingStationChange) {
        pendingStationChange = false;
        
        Serial.println("=== CHANGING STATION ===");
        
        isPlaying = false;
        streamClient.stop();
        
        player.softReset();
        delay(200);
        player.switchToMp3Mode();
        delay(50);
        player.setVolume(currentVolume);
        player.startSong();
        audioEnhancementApplied = false;
        newVolume = currentVolume;  // forțăm re-aplicare volum după ce chip e gata
        
        storedCount = 0; 
        head = 0; 
        tail = 0;
        
        parseUrl(currentStationURL);
        lastConnectAttempt = 0;
        
        Serial.println("Station changed successfully!");
    }

    // Volume Update
    if(newVolume >= 0) {
        currentVolume = newVolume;
        player.setVolume(currentVolume);
        
        // Salvăm volumul nou
        preferences.begin("radio", false);
        preferences.putInt("lastVolume", currentVolume);
        preferences.end();
        
        newVolume = -1; 
    }

    // EQ Update (SCI_AUDAT) — scrie registrul live, funcționează în timp real
    if(newAudatPending) {
        newAudatPending = false;
    // Registru 0x02 (SCI_BASS): [15:12]=Treble amp, [11:8]=Treble freq (fix 2 = 4kHz)
    //                           [7:4]=Bass amp,     [3:0]=Bass freq (set 15 = 150Hz max freq)
    // Treble 1-7 = Boost. 8-15 = Cut. Slider UI limits to 0-7.
    uint16_t audat = ((uint16_t)currentTreble << 12) | (2 << 8) |
                     ((uint16_t)currentBass  << 4)  |  15;
    player.writeRegister(0x02, audat); 
    audioEnhancementApplied = true;  
    Serial.print("AUDAT written to 0x02: 0x"); Serial.println(audat, HEX);
    }

    // WiFi Connect
    if(shouldConnectWifi) {
        shouldConnectWifi = false;
        Serial.print("Connecting to: ");
        Serial.println(wifiSSID);
        WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if(WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nFailed to connect");
        }
    }

    // Stream handling
    if(WiFi.status() == WL_CONNECTED && !shouldStop && currentStationName != "Stopped") {
        if(!streamClient.connected() && (millis() - lastConnectAttempt > 3000)) {
             lastConnectAttempt = millis();
             if(streamClient.connect(host.c_str(), port)) {
                 Serial.println("Connected to stream");
                 streamClient.print(String("GET ") + path + " HTTP/1.0\r\n" +
                                    "Host: " + host + "\r\n" + 
                                    "Connection: close\r\n\r\n");
                 lastDataTime = millis(); // Reset watchdog on connect
                 unsigned long t = millis();
                 while(streamClient.connected() && (millis() - t < 2000)) {
                     if(streamClient.available()) {
                         String line = streamClient.readStringUntil('\n');
                         if(line == "\r" || line.length() < 2) break; 
                     }
                 }
             }
        }
        
        int avail = streamClient.available();
        if(avail > 0) {
            int space = BUFFER_SIZE - storedCount - 1;
            if(space > 1024) {
                uint8_t temp[1024];
                int toRead = min(avail, min(1024, space));
                int n = streamClient.read(temp, toRead);
                for(int i=0; i<n; i++) {
                     staticBuffer[head] = temp[i];
                     head = (head + 1) % BUFFER_SIZE;
                }
                storedCount += n;
                lastDataTime = millis(); // WATCHDOG: We got data!
            }
        }
    }

    // ============ DATA WATCHDOG (FIX FOR 5 MIN STOP) ============
    // Daca stația NU e oprită (vrem să cânte) și nu am primit date de 15 secunde -> Reconnect
    if(currentStationName != "Stopped" && (millis() - lastDataTime > 15000)) {
        Serial.println("!!! DATA STALL (Zombie Stream) - No data for 15s - Forcing Reconnect !!!");
        streamClient.stop(); // Asta va forța conectarea în următorul loop
        isPlaying = false;
        lastDataTime = millis(); // Reset pentru a nu spama
    }

    // Playback
    if(isPlaying && !shouldStop) {
        if(storedCount >= 32 && player.data_request()) {
            uint8_t chunk[32];
            for(int i=0; i<32; i++) {
                chunk[i] = staticBuffer[tail];
                tail = (tail + 1) % BUFFER_SIZE;
            }
            player.playChunk(chunk, 32);
            storedCount -= 32;
        }
        if(storedCount < 512) isPlaying = false; 
    } else {
        if(storedCount >= START_THRESHOLD && !shouldStop && currentStationName != "Stopped") {
            isPlaying = true;
            Serial.println("=== STARTING PLAYBACK ===");
            
            // SCI_AUDAT se aplică ABIA după ce decodarea a început (VS1053 spec)
            // Registru 0x02 (SCI_BASS): [15:12]=Treble amp, [11:8]=Treble freq×2kHz
            //                           [7:4]=Bass amp,     [3:0]=Bass freq×10Hz  
            // Treble: freq=2 (4kHz). Bass: freq=15 (150Hz).
            if(!audioEnhancementApplied) {
                uint16_t audat = ((uint16_t)currentTreble << 12) | (2 << 8) |
                                 ((uint16_t)currentBass  << 4)  |  15;
                player.writeRegister(0x02, audat); 
                audioEnhancementApplied = true;
                Serial.print("AUDAT at start (0x02): 0x"); Serial.println(audat, HEX);
            }
        }
    }

    // ============ OLED UPDATE ============
    if(millis() - lastUiUpdate > 500) {
        lastUiUpdate = millis();
        display.clearDisplay();
        
        display.setTextColor(SSD1306_WHITE);
        
        if(WiFi.status() != WL_CONNECTED) {
            // MODE AP - IP și parolă cu SIZE 2
            display.setTextSize(2);
            display.setCursor(0, 0);
            display.print("192.168.4.1");
            
            // Parolă pe randul 2
            display.setCursor(0, 20);
            display.print("123456789");
            
            // Mesaj jos cu SIZE 1
            display.setTextSize(1);
            display.setCursor(10, 48);
            display.print("Connect to WiFi");
        } else {
            // MODE CONECTAT - Afișăm numele stației
            display.setTextSize(2);
            display.setCursor(0, 0);
            
            if(currentStationName.indexOf("Radio Hit") >= 0 || 
               currentStationName.indexOf("RADIO HIT") >= 0) {
                display.print("RADIO HIT");
            } else if(currentStationName == "Stopped" || currentStationName.length() == 0) {
                display.print("STOPPED");
            } else {
                String shortName = currentStationName;
                if(shortName.length() > 10) {
                    shortName = shortName.substring(0, 10);
                }
                display.print(shortName);
            }

            // IP
            String ip = WiFi.localIP().toString();
            display.setTextSize(2);
            display.setCursor(0, 20);
            display.print(ip);

            // RSSI
            long rssi = WiFi.RSSI();
            display.setTextSize(2);
            display.setCursor(67, 44);
            display.print("R:");
            display.print(rssi);
            
            // Linie buffer pe ultimul rând (pixelul 63)
            int buffPerc = (storedCount * 100) / BUFFER_SIZE;
            int lineWidth = (buffPerc * 60) / 100;  // Max 60 pixels width
            if(lineWidth > 0) {
                display.fillRect(0, 63, lineWidth, 1, SSD1306_WHITE);
            }
        }
        
        display.display();
    }

    // ============ DEBUG & WATCHDOG (Kevin's Request) ============
    static unsigned long lastDebugTime = 0;
    static unsigned long bufferEmptyStartTime = 0;

    if (millis() - lastDebugTime > 5000) {
        lastDebugTime = millis();
        Serial.print("[HEARTBEAT] FreeHeap: "); Serial.print(ESP.getFreeHeap());
        Serial.print(" | WiFi: "); Serial.print(WiFi.status() == WL_CONNECTED ? "OK" : "Lost");
        Serial.print(" | Stream: "); Serial.print(streamClient.connected() ? "Conn" : "Disc");
        Serial.print(" | Buff: "); Serial.print(storedCount); 
        Serial.print("/"); Serial.print(BUFFER_SIZE);
        Serial.print(" | Playing: "); Serial.println(isPlaying);
        
        // WATCHDOG: Dacă suntem "Playing" dar bufferul e gol de mult timp -> Force Reconnect
        if(isPlaying && storedCount == 0) {
            if(bufferEmptyStartTime == 0) bufferEmptyStartTime = millis();
            
            // Dacă au trecut 10 secunde de "liniște"
            if(millis() - bufferEmptyStartTime > 10000) {
                Serial.println("!!! STALL DETECTED - Forcing Reconnect !!!");
                streamClient.stop(); // Asta va forța logica de reconnect din loop() main
                isPlaying = false;   // Resetăm starea
                bufferEmptyStartTime = 0;
            }
        } else {
            bufferEmptyStartTime = 0; // Resetăm contorul dacă avem date
        }
    }
    
    yield(); 
}
