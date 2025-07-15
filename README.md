# ESP32-BT-TNC
The ESP32 KISS TNC is a minimalistic design using the fewest necessary components by 2E0UMR.

**This repository is a work in progress. I have not yet built the project so there is no expectation that it will work as-is. There will be changes as the project develops and those changes may make the currect repository obsolete.**

**Note that this version of the 2E0UMR TNC uses the ESP32 DevKit V1 rather than the V4. Adapting the 2E0UMR design to the stripboard layout required changing some GPIO selections.**

# ESP32 KISS TNC Bluetooth setup for APRSdroid  

[https://uhpowerup.com/aprsdroid-esp32-kiss-tnc/](https://uhpowerup.com/aprsdroid-esp32-kiss-tnc/)

The ESP32 KISS TNC is a minimalistic design using the fewest necessary components. An improved version is in development. In this guide, I’ll show you how to connect the ESP32 KISS TNC to APRSdroid via Bluetooth. For detailed configuration settings, refer to the [official APRSdroid website](https://aprsdroid.org/)—I’ll focus solely on the connection process.

**Pairing:**

1. Power on ESP32 KISS TNC and wait 30 seconds (TNC will initialise asap on powerup but it's better to wait).
2. Go to your phone Bluetooth settings and pair with ESP32 KISS TNC. (Pair the TNC here before starting the app.)

**APRSdroid settings:**  

1. Open the APRSdroid app.
2. Press on 3 dots on the top right corner.
3. Got to “Preferences”.
4. Add you details like Callsign, SSID, Location and other setting as you prefer follow the official guide line for office website.
5. For TNC settings tap on “Connection Preference” in APRS Connection.
6. Tap on “Connection Protocol”.
7. Select “TNC(KISS)”.
8. Leave “TNC init string” blank.
9. “TNC init delay” at 300.
10. Tap “Connection type”.
11. Select “Bluetooth SSP”.
12. Tick the “Client Mode” on.
13. Tap on “TNC Bluetooth Device”.
14. Select “ESP32 KISS TNC"
15. Leave “Channel” blank.
16. Leave “Bluetooth settings” as it is.

Now on the main screen of the app tap “Start Tracking” on bottom right. APRSdroid will Decode/Encode APRS packets according to the APRS settings.  

73’s
2E0UMR
