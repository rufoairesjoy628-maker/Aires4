#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DHT.h>
#include <time.h>

// =====================================================
// KIMBERLY ACTIVITY 4
// DHT11 + FIREBASE + ASYNC WEB SERVER
// AIRES FIREBASE / COLOR THEME COMPATIBLE
// =====================================================

// =====================================================
// DHT11
// =====================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =====================================================
// FIREBASE - AIRES ACCOUNT
// =====================================================

#define API_KEY "AIzaSyB0Yy_lveQZwd5rYiuYZglmGXvJ-aglpXY"

#define DATABASE_URL "https://aires-web-default-rtdb.europe-west1.firebasedatabase.app"

#define USER_EMAIL "rufoairesjoy628@gmail.com"

// IMPORTANT:
// Replace this locally with the Firebase Authentication password.
// Do not post the password publicly.
#define USER_PASSWORD "airesjoynavarrarufo@16"

// =====================================================
// FIREBASE OBJECTS
// =====================================================

UserAuth user_auth(
    API_KEY,
    USER_EMAIL,
    USER_PASSWORD,
    3000
);

FirebaseApp app;

WiFiClientSecure ssl_client;

AsyncClientClass aClient(ssl_client);

RealtimeDatabase Database;

// =====================================================
// WEB SERVER
// =====================================================

AsyncWebServer server(80);

// =====================================================
// WIFI MANAGER
// =====================================================

const char *AP_SSID = "ESP-WIFI-MANAGER";

bool wifiManagerMode = false;

// =====================================================
// SENSOR
// =====================================================

unsigned long lastSensorRead = 0;

const unsigned long SENSOR_INTERVAL = 10000;

// =====================================================
// WIFI MANAGER HTML
// Aires design, served by ESP32
// =====================================================

const char WIFI_MANAGER_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>

<meta charset="UTF-8">

<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>ESP32 Wi-Fi Manager</title>

<style>
*{margin:0;padding:0;box-sizing:border-box}

:root{
--pink:#e96b9b;
--pink-dark:#c94f80;
--lavender:#a98bd4;
--text:#493344;
--text-dark:#352431;
--muted:#987f8e;
--border:rgba(216,91,136,.18);
}

body{
min-height:100vh;
padding:20px;
display:flex;
align-items:center;
justify-content:center;
background:
radial-gradient(circle at 10% 10%,rgba(233,107,155,.16),transparent 30%),
radial-gradient(circle at 90% 5%,rgba(169,139,212,.17),transparent 27%),
#fff8fb;
color:var(--text);
font-family:Arial,sans-serif;
}

.wifi-page{width:100%;max-width:570px}

.wifi-card{
position:relative;
overflow:hidden;
padding:35px;
background:rgba(255,255,255,.95);
border:1px solid var(--border);
border-radius:26px;
box-shadow:0 18px 50px rgba(157,85,119,.14);
}

.wifi-card:before{
content:"♡";
position:absolute;
right:20px;
top:-55px;
font:180px Arial,sans-serif;
color:rgba(233,107,155,.06);
}

.wifi-icon{
position:relative;
width:55px;
height:55px;
display:grid;
place-items:center;
margin-bottom:18px;
color:white;
background:linear-gradient(135deg,var(--pink),var(--lavender));
border-radius:17px;
font-size:24px;
box-shadow:0 10px 25px rgba(216,91,136,.22);
}

.system-label{
position:relative;
color:var(--pink);
font-size:9px;
letter-spacing:2px;
font-weight:bold;
}

h1{
position:relative;
margin-top:8px;
color:var(--text-dark);
font-size:29px;
}

.description{
position:relative;
margin-top:10px;
color:var(--muted);
font-size:13px;
line-height:1.7;
}

form{position:relative;margin-top:28px}

.field{margin-bottom:18px}

.field label{
display:block;
margin-bottom:7px;
color:#9b8190;
font-size:9px;
letter-spacing:1.5px;
font-weight:bold;
}

.field input{
width:100%;
height:48px;
padding:0 14px;
border:1px solid rgba(216,91,136,.30);
border-radius:13px;
outline:none;
background:#fffafd;
color:var(--pink-dark);
font-size:15px;
}

.field input:focus{
border-color:var(--pink);
box-shadow:0 0 0 3px rgba(233,107,155,.10);
}

.submit{
width:100%;
height:49px;
border:1px solid var(--pink);
border-radius:13px;
background:linear-gradient(135deg,var(--pink),var(--pink-dark));
color:white;
cursor:pointer;
font-size:14px;
font-weight:bold;
letter-spacing:1px;
box-shadow:0 10px 25px rgba(216,91,136,.18);
}

.note{
position:relative;
margin-top:20px;
padding:15px;
border:1px solid var(--border);
border-radius:14px;
background:#fff4f8;
color:var(--muted);
font-size:12px;
line-height:1.7;
}

.note strong{color:var(--pink-dark)}

@media(max-width:600px){
body{padding:12px}
.wifi-card{padding:25px 20px}
h1{font-size:24px}
}
</style>

</head>

<body>

<div class="wifi-page">

<div class="wifi-card">

<div class="wifi-icon">♡</div>

<div class="system-label">
ESP32 / NETWORK CONFIGURATION
</div>

<h1>Wi-Fi Manager</h1>

<p class="description">
Enter your Wi-Fi credentials below.
The ESP32 will save them automatically.
</p>

<form action="/savewifi" method="POST">

<div class="field">
<label for="ssid">WI-FI SSID</label>

<input
type="text"
id="ssid"
name="ssid"
placeholder="Enter Wi-Fi name"
required>
</div>

<div class="field">
<label for="password">WI-FI PASSWORD</label>

<input
type="password"
id="password"
name="password"
placeholder="Enter Wi-Fi password"
required>
</div>

<button type="submit" class="submit">
♡ SAVE & CONNECT
</button>

</form>

<div class="note">
<strong>Note:</strong>
The ESP32 saves the Wi-Fi credentials
in LittleFS memory.
</div>

</div>
</div>

</body>
</html>
)rawliteral";

// =====================================================
// FILE HELPERS
// =====================================================

String readFile(const char *path)
{
    if (!LittleFS.exists(path))
        return "";

    File file = LittleFS.open(path, "r");

    if (!file)
        return "";

    String data = file.readString();

    file.close();

    data.trim();

    return data;
}

bool writeFile(const char *path, const String &data)
{
    File file = LittleFS.open(path, "w");

    if (!file)
        return false;

    file.print(data);
    file.close();

    return true;
}

void deleteWiFiFiles()
{
    LittleFS.remove("/ssid.txt");
    LittleFS.remove("/pass.txt");

    Serial.println("WiFi settings deleted.");
}

// =====================================================
// CONNECT SAVED WIFI
// =====================================================

bool connectToSavedWiFi()
{
    String ssid = readFile("/ssid.txt");
    String pass = readFile("/pass.txt");

    if (ssid.length() == 0)
    {
        Serial.println("No saved WiFi credentials.");
        return false;
    }

    Serial.println();
    Serial.println("=================================");
    Serial.println("       SAVED WIFI FOUND");
    Serial.println("=================================");

    Serial.print("SSID: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.print("Connecting to WiFi");

    unsigned long startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WIFI CONNECTED");

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());

        return true;
    }

    Serial.println("Failed to connect to saved WiFi.");

    WiFi.disconnect(true);
    delay(1000);

    return false;
}

// =====================================================
// START WIFI MANAGER
// =====================================================

void startWiFiManager()
{
    wifiManagerMode = true;

    Serial.println();
    Serial.println("=================================");
    Serial.println("       WIFI MANAGER MODE");
    Serial.println("=================================");

    WiFi.mode(WIFI_AP);
    delay(300);

    if (!WiFi.softAP(AP_SSID))
    {
        Serial.println("ERROR: Failed to start WiFi Manager AP!");
    }

    delay(500);

    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // WiFi manager page
    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            request->send(
                200,
                "text/html",
                WIFI_MANAGER_HTML
            );
        }
    );

    // Save WiFi using Aires-style /savewifi
    server.on(
        "/savewifi",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            String ssid = "";
            String password = "";

            if (request->hasParam("ssid", true))
            {
                ssid = request->getParam("ssid", true)->value();
            }

            if (request->hasParam("password", true))
            {
                password = request->getParam("password", true)->value();
            }

            ssid.trim();
            password.trim();

            if (ssid.length() == 0 || password.length() == 0)
            {
                request->send(
                    400,
                    "text/plain",
                    "SSID and password are required."
                );

                return;
            }

            writeFile("/ssid.txt", ssid);
            writeFile("/pass.txt", password);

            request->send(
                200,
                "text/html",
                "<!DOCTYPE html>"
                "<html><head>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>WiFi Saved</title>"
                "<style>"
                "body{font-family:Arial;background:#fff8fb;text-align:center;padding:40px;color:#493344}"
                ".box{max-width:500px;margin:auto;background:white;padding:30px;border-radius:22px;box-shadow:0 15px 40px rgba(157,85,119,.15)}"
                "h1{color:#c94f80}"
                "</style></head><body>"
                "<div class='box'>"
                "<h1>♡ Wi-Fi Saved</h1>"
                "<p>The ESP32 will restart and connect to the saved Wi-Fi.</p>"
                "<p>Please wait...</p>"
                "</div></body></html>"
            );

            delay(1500);
            ESP.restart();
        }
    );

    server.begin();

    Serial.println("WIFI MANAGER READY");
    Serial.print("Open: http://");
    Serial.println(WiFi.softAPIP());
}

// =====================================================
// MAIN WEB SERVER
// =====================================================

void startMainWebServer()
{
    wifiManagerMode = false;

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            if (LittleFS.exists("/index.html"))
            {
                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );
            }
            else
            {
                request->send(
                    404,
                    "text/plain",
                    "index.html not found. Upload the data folder to LittleFS."
                );
            }
        }
    );

    server.on(
        "/change-wifi",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            request->send(
                200,
                "text/html",
                "<!DOCTYPE html>"
                "<html><head>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>Changing WiFi</title>"
                "<style>"
                "body{font-family:Arial;background:#fff8fb;text-align:center;padding:40px;color:#493344}"
                ".box{max-width:500px;margin:auto;background:white;padding:30px;border-radius:22px;box-shadow:0 15px 40px rgba(157,85,119,.15)}"
                "h1{color:#c94f80}"
                "</style></head><body>"
                "<div class='box'>"
                "<h1>Wi-Fi Manager</h1>"
                "<p>Wi-Fi settings will be cleared.</p>"
                "<p>The ESP32 will restart.</p>"
                "</div></body></html>"
            );

            delay(1000);

            deleteWiFiFiles();

            ESP.restart();
        }
    );

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );

    server.begin();

    Serial.println();
    Serial.println("=================================");
    Serial.println("      MAIN WEB SERVER READY");
    Serial.println("=================================");

    Serial.print("Website: http://");
    Serial.println(WiFi.localIP());
}

// =====================================================
// FIREBASE CALLBACK
// =====================================================

void processFirebase(AsyncResult &aResult)
{
    if (!aResult.isResult())
        return;

    if (aResult.isEvent())
    {
        Firebase.printf(
            "Firebase Event - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.eventLog().message().c_str(),
            aResult.eventLog().code()
        );
    }

    if (aResult.isDebug())
    {
        Firebase.printf(
            "Firebase Debug - task: %s, msg: %s\n",
            aResult.uid().c_str(),
            aResult.debug().c_str()
        );
    }

    if (aResult.isError())
    {
        Firebase.printf(
            "Firebase Error - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.error().message().c_str(),
            aResult.error().code()
        );
    }
}

// =====================================================
// FIREBASE SETUP
// =====================================================

void setupFirebase()
{
    ssl_client.setInsecure();

    Serial.println("Initializing Firebase...");

    initializeApp(
        aClient,
        app,
        getAuth(user_auth),
        processFirebase,
        "authTask"
    );

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);

    Serial.println("Firebase initialization started.");
}

// =====================================================
// TIME
// =====================================================

String getDateString()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
        return "1970-01-01";

    char buffer[20];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );

    return String(buffer);
}

String getTimeString()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
        return "00:00:00";

    char buffer[20];

    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M:%S",
        &timeinfo
    );

    return String(buffer);
}

// =====================================================
// SEND DHT11 DATA
// =====================================================

void sendSensorData()
{
    if (!app.ready())
    {
        Serial.println("Firebase not ready yet...");
        return;
    }

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("ERROR: Failed to read DHT11");
        return;
    }

    String date = getDateString();
    String time = getTimeString();

    String basePath =
        "/ESP32_Data/" +
        date +
        "/" +
        time;

    String temperaturePath =
        basePath + "/temperature";

    String humidityPath =
        basePath + "/humidity";

    Serial.println();
    Serial.println("=================================");
    Serial.println("       DHT11 SENSOR READING");
    Serial.println("=================================");

    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");

    Serial.print("Date: ");
    Serial.println(date);

    Serial.print("Time: ");
    Serial.println(time);

    Serial.print("Firebase: ");
    Serial.println(basePath);

    Database.set<float>(
        aClient,
        temperaturePath,
        temperature,
        processFirebase,
        "temperatureTask"
    );

    Database.set<float>(
        aClient,
        humidityPath,
        humidity,
        processFirebase,
        "humidityTask"
    );

    Serial.println("Temperature write task sent.");
    Serial.println("Humidity write task sent.");
    Serial.println("=================================");
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println(" KIMBERLY ACTIVITY 4 DHT11");
    Serial.println(" FIREBASE SENSOR MONITOR");
    Serial.println("=================================");

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS mount failed!");

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("LittleFS ready.");

    dht.begin();

    Serial.println("DHT11 initialized on GPIO4.");

    bool connected = connectToSavedWiFi();

    if (!connected)
    {
        startWiFiManager();
        return;
    }

    // Philippines UTC+8
    configTime(
        8 * 3600,
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );

    Serial.print("Waiting for time");

    struct tm timeinfo;
    int retry = 0;

    while (
        !getLocalTime(&timeinfo) &&
        retry < 20
    )
    {
        Serial.print(".");
        delay(500);
        retry++;
    }

    Serial.println();

    if (getLocalTime(&timeinfo))
    {
        Serial.println("Time synchronized.");

        Serial.print("Date: ");
        Serial.println(getDateString());

        Serial.print("Time: ");
        Serial.println(getTimeString());
    }
    else
    {
        Serial.println("WARNING: Time synchronization failed.");
    }

    setupFirebase();

    startMainWebServer();

    Serial.println();
    Serial.println("=================================");
    Serial.println("         SYSTEM READY");
    Serial.println("=================================");

    Serial.print("Website: http://");
    Serial.println(WiFi.localIP());
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    if (!wifiManagerMode)
    {
        app.loop();
    }

    if (
        !wifiManagerMode &&
        millis() - lastSensorRead >= SENSOR_INTERVAL
    )
    {
        lastSensorRead = millis();

        sendSensorData();
    }

    delay(10);
}
