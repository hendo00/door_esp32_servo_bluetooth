#include "BluetoothSerial.h"
#include <ESP32Servo.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

Servo doorServo;  
BluetoothSerial SerialBT;

const int servoPin = 15;
unsigned long lastResetTime = 0;  // Track time for auto-reboot
const unsigned long resetInterval = 10800000; // 3 hours in milliseconds

// Move servo (unlock door)
void unlockDoor() {
    doorServo.write(90);  // Move to open position
    delay(1500);
    doorServo.write(0);   // Return to closed position
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32test"); // Bluetooth device name
    Serial.println("The device started, now you can pair it with Bluetooth!");

    // Allow allocation of all timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    doorServo.setPeriodHertz(50);  
    doorServo.attach(servoPin, 1000, 2000);  
    doorServo.write(0);  // Start in closed position

    lastResetTime = millis();  // Store startup time
}

void loop() {
    // Auto-reset ESP32 every 3 hours
    if (millis() - lastResetTime > resetInterval) {
        Serial.println("🔄 Rebooting ESP32...");
        delay(1000);
        esp_restart();  // Restart ESP32
    }

    if (Serial.available()) {
        SerialBT.write(Serial.read());
    }

    if (SerialBT.available()) {
        String receivedData = SerialBT.readStringUntil('\n');  
        receivedData.trim();  
        Serial.println("Received: " + receivedData);

        if (receivedData == "robotics") {
            Serial.println("Correct Code! Unlocking door...");
            unlockDoor();
            SerialBT.disconnect();
        }
    }

    delay(20);
}
