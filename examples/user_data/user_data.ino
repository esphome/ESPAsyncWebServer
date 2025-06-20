//
// A simple server implementation showing how to:
//  * serve arbitrary user data
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <string>
#include <vector>

AsyncWebServer server(80);

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const std::vector<std::string> tokens {
    "Hello", " ", "World", "!"
};

void setup() {

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginUserDataResponse("text/plain", [&](uint8_t *buffer, size_t maxLen, std::any& userData) -> size_t {
            // If user data is not set yet, we start with index 0 and try again
            if (!userData.has_value()) {
                userData = (size_t)0;
                return RESPONSE_TRY_AGAIN;
            } else if (std::any_cast<size_t>(userData) >= tokens.size()) {
                return 0;
            }
            // Write up to "maxLen" bytes into "buffer" and return the amount written.
            // You will be asked for more data until 0 is returned
            size_t i = std::any_cast<size_t>(userData);
            const size_t tokenSize = tokens.at(i).size();
            if (tokenSize > maxLen) {
                std::cout << "token " << i << " with length: " << tokenSize << " exceeds max length: " << maxLen << std::endl;
                return 0;
            }

            size_t bytesWritten = 0;
            while (i < tokens.size() && (bytesWritten + tokens.at(i).size()) <= maxLen) {
                const auto& token = tokens.at(i);
                std::memcpy(buffer + bytesWritten, token.c_str(), token.size());
                bytesWritten += token.size();
                ++i;
            }

            userData = i;
            return bytesWritten;
        });
        request->send(response);
    });

    server.begin();
}

void loop() {
}
