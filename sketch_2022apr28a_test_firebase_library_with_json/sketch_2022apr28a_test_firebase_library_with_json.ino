
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/FirebaseJson
 *
 * Copyright (c) 2022 mobizt
 *
 */

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#endif

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Killing Me Softly 2G"  /* "WIFI_AP" */
#define WIFI_PASSWORD "14mP455w0rd" /* "WIFI_PASSWORD" */

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyD33bPwrv1juO9Sgwp7_T2a0oH5uyMX4VU" /* "API_KEY" */
//#define API_KEY "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx" /* "API_KEY" */

/* 3. Define the RTDB URL */
#define DATABASE_URL "asset-management-lff.firebaseio.com" /* "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app */

/* 4. Define the user Email and password that alreadey registerd or added in your project */
//#define USER_EMAIL "siamroyal.dev@gmail.com" /* "USER_EMAIL" */
//#define USER_PASSWORD "14mP455w0rd" /* "USER_PASSWORD" */
#define USER_EMAIL "" /* "USER_EMAIL" */
#define USER_PASSWORD "" /* "USER_PASSWORD" */

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

//    /* Assign the user sign in credentials */
//    auth.user.email = USER_EMAIL;
//    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Or use legacy authenticate method
    // config.database_url = DATABASE_URL;
    // config.signer.tokens.legacy_token = "<database secret>";
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx";

    Firebase.begin(&config, &auth);

    // Comment or pass false value when WiFi reconnection will control by your code or third party library
    Firebase.reconnectWiFi(true);


    // Optional, set the size of BearSSL WiFi to receive and transmit buffers
    // Firebase may not support the data transfer fragmentation, you may need to reserve the buffer to match
    // the data to be transported.
//    fbdo.setBSSLBufferSize(1024, 1024); // minimum size is 512 bytes, maximum size is 16384 bytes
    fbdo.setBSSLBufferSize(2048, 2048);

    // Optional, set the size of HTTP response buffer
    // Prevent out of memory for large payload but data may be truncated and can't determine its type.
    fbdo.setResponseSize(1024); // minimum size is 1024 bytes
    

    if (Firebase.ready()) {
      if (Firebase.getInt(fbdo, "/test/array/4")) {
  
        if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
          Serial.printf("/test/array/4=%d\n", fbdo.to<int>());
//          Serial.printf("Get json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/json") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
        }

        Serial.printf("Get json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/json") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
        Serial.printf("Get device json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/users/cray/devices/84:CC:A8:88:6E:07/notification") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());


    
      } else {
        Serial.println(fbdo.errorReason());
      }
    }
}

void loop()
{
//    // Flash string (PROGMEM and  (FPSTR), String C/C++ string, const char, char array, string literal are supported
//    // in all Firebase and FirebaseJson functions, unless F() macro is not supported.
//
//    // Firebase.ready() should be called repeatedly to handle authentication tasks.
//
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 60000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

        Serial.printf("Get device json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/users/cray/devices/84:CC:A8:88:6E:07/notification") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
//
//        FirebaseJson json;
//        json.setDoubleDigits(3);
//        json.add("value", count);
//
//        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, "/test/json", &json) ? "ok" : fbdo.errorReason().c_str());
//
//        Serial.printf("Get json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/json") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
//
//        FirebaseJson jVal;
//        Serial.printf("Get json ref... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/json", &jVal) ? jVal.raw() : fbdo.errorReason().c_str());
//
//        FirebaseJsonArray arr;
//        arr.setFloatDigits(2);
//        arr.setDoubleDigits(4);
//        arr.add("a", "b", "c", true, 45, (float)6.1432, 123.45692789);
//
//        Serial.printf("Set array... %s\n", Firebase.RTDB.setArray(&fbdo, "/test/array", &arr) ? "ok" : fbdo.errorReason().c_str());
//
//        Serial.printf("Get array... %s\n", Firebase.RTDB.getArray(&fbdo, "/test/array") ? fbdo.to<FirebaseJsonArray>().raw() : fbdo.errorReason().c_str());
//
//        Serial.printf("Push json... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/test/push", &json) ? "ok" : fbdo.errorReason().c_str());
//
//        json.set("value", count + 0.29745);
//        Serial.printf("Update json... %s\n\n", Firebase.RTDB.updateNode(&fbdo, "/test/push/" + fbdo.pushName(), &json) ? "ok" : fbdo.errorReason().c_str());
//
        count++;
    }
}
