#include <AFMotor.h>

// Motor definitions (Adafruit Motor Shield V1)
AF_DCMotor elevatorMotor(2); // Elevator motor on M1
AF_DCMotor sortingMotor(1);  // Sorting motor on M2

// LED pin definitions
const int ledGinger = 7; // LED for Ginger
const int ledOnion = 5;  // LED for Onion
const int ledPotato = 2; // LED for Potato

// Motor speed (0-255)
const int motorSpeed = 255; // Adjust as needed

String receivedFruit = ""; // Stores incoming serial data
String lastFruit = "";     // Stores the last received fruit

void turnOffAllLEDs()
{
    digitalWrite(ledGinger, LOW);
    digitalWrite(ledOnion, LOW);
    digitalWrite(ledPotato, LOW);
}

void setup()
{
    // Initialize LED pins
    pinMode(ledGinger, OUTPUT);
    pinMode(ledOnion, OUTPUT);
    pinMode(ledPotato, OUTPUT);

    // Start serial communication
    Serial.begin(9600);

    // Set motor speeds
    elevatorMotor.setSpeed(255);
    sortingMotor.setSpeed(255);

    // Ensure motors and LEDs are off initially
    elevatorMotor.run(RELEASE);
    sortingMotor.run(RELEASE);
    turnOffAllLEDs();

    // Start elevator motor (continuous forward)
    elevatorMotor.run(FORWARD);
}

void loop()
{
    turnOffAllLEDs();
    if (Serial.available() > 0)
    {
        receivedFruit = Serial.readStringUntil('\n');
        receivedFruit.trim(); // Remove whitespace

        // Process the received fruit
        Serial.println(receivedFruit); // Debug output

        // Special case: Last fruit was Onion, current is Potato
        if (lastFruit == "Onion" && receivedFruit == "Potato")
        {
            digitalWrite(ledPotato, HIGH);
            sortingMotor.run(FORWARD);
            elevatorMotor.run(RELEASE);
            delay(330); // 300 + 30
            elevatorMotor.run(FORWARD);
            sortingMotor.run(RELEASE);
        }
        else if (lastFruit == "Ginger" && receivedFruit == "Onion")
        {
            digitalWrite(ledPotato, HIGH);
            sortingMotor.run(BACKWARD);
            elevatorMotor.run(RELEASE);
            delay(660); // 300 + 30
            elevatorMotor.run(FORWARD);
            sortingMotor.run(RELEASE);
        }
        else
        {
            // Standard fruit-specific logic
            if (receivedFruit == "Ginger")
            {
                // digitalWrite(ledGinger, HIGH);
                sortingMotor.run(FORWARD);
                elevatorMotor.run(RELEASE);
                if (lastFruit == "Onion")
                {
                    delay(660); // 2 * 300 + 30
                }
                else
                {
                    delay(330); // 300 + 30
                }
                elevatorMotor.run(FORWARD);
                sortingMotor.run(RELEASE);
            }
            else if (receivedFruit == "Onion")
            {
                digitalWrite(ledOnion, HIGH);
                sortingMotor.run(BACKWARD);
                elevatorMotor.run(RELEASE);
                delay(330); // 300 + 30
                elevatorMotor.run(FORWARD);
                sortingMotor.run(RELEASE);
            }
            else if (receivedFruit == "Potato")
            {
                digitalWrite(ledPotato, HIGH);
                if (lastFruit != "")
                {
                    sortingMotor.run(BACKWARD);
                    elevatorMotor.run(RELEASE);
                    delay(330); // 300 + 30
                    sortingMotor.run(RELEASE);
                    elevatorMotor.run(FORWARD);
                }
                // No motor movement if lastFruit is empty
            }
            delay(2000);
        }

        // Update lastFruit if the received fruit is valid
        if (receivedFruit == "Ginger" || receivedFruit == "Onion" || receivedFruit == "Potato")
        {
            lastFruit = receivedFruit;
        }
    }
}