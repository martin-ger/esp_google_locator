# esp_google_locator
Self-localization of an ESP8266 using the Google Maps API

This is a simple proof-of-concept implementation of self-localization of an ESP using the Google Maps API. If does a scan of the locally visible SSIDs, formats a request to the Google Maps Web Service and receives latitude and longitude of its current location, if available. These values are printed if the request was successful. You can now enter these values in Google Maps in your browser to find the location on the map.

Of course, this all might be done better with a GPS-receiver, but this is much more expensive, requires more energy, and does not work indoors.

In order to work in other places than in the home WiFi network, the sketch additionally scans for open WiFi netorks and tries to connect via these.

This should serve as a starting point for some useful application ideas, e.g. like an ESP-based tracker that awakes from deep sleep, does a scan and either uploads its position to a server, stores it in flash for later usage, or performs a location dependent action.

# Building
This is an Arduino sketch. For building it you will need the Arduino IDE with the ESP8266 extensions. Installation instructions can be found easily in the Web. In addition it requires the ArduinoJson library from https://github.com/bblanchon/ArduinoJson . To get the localisation running you will also need an API key for the Google Maps API from https://developers.google.com/maps/documentation/geolocation .

Enter your API key in the code. Also you might add your AP_SSID and AP_PASSWORD if you do not want to look for open WiFi networks.
