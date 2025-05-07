// Pin definitions
const int elevatorMotorPin1 = 9; // Elevator motor control pin 1 (L298N IN1)
const int elevatorMotorPin2 = 8; // Elevator motor control pin 2 (L298N IN2)
const int sortingMotorPin1 = 7;  // Sorting motor control pin 1 (L298N IN3)
const int sortingMotorPin2 = 6;  // Sorting motor control pin 2 (L298N IN4)
const int ledApple = 5;          // LED for Apple (Red)
const int ledOnion = 4;          // LED for Onion (Green)
const int ledPotato = 3;         // LED for Potato (Blue)

String receivedFruit = ""; // Stores incoming serial data

// Motor control functions
void runElevator()
{
    digitalWrite(elevatorMotorPin1, HIGH); // Move elevator forward
    digitalWrite(elevatorMotorPin2, LOW);
}

void stopElevator()
{
    digitalWrite(elevatorMotorPin1, LOW);
    digitalWrite(elevatorMotorPin2, LOW);
}

void runSortingForward()
{
    digitalWrite(sortingMotorPin1, HIGH); // Move sorting belt forward
    digitalWrite(sortingMotorPin2, LOW);
}

void runSortingBackward()
{
    digitalWrite(sortingMotorPin1, LOW); // Move sorting belt backward
    digitalWrite(sortingMotorPin2, HIGH);
}

void stopSorting()
{
    digitalWrite(sortingMotorPin1, LOW);
    digitalWrite(sortingMotorPin2, LOW);
}

void turnOffAllLEDs()
{
    digitalWrite(ledApple, LOW);
    digitalWrite(ledOnion, LOW);
    digitalWrite(ledPotato, LOW);
}

void sortProcess(int ledPin, int frequency) {
  digitalWrite(ledPin, frequency); // Turn on Apple LED
  stopElevator();
  runSortingForward();          // Move sorting belt forward
  delay(3000);                  // Run for 3 seconds
  stopSorting();                // Stop sorting belt
  runElevator();
}

void setup()
{
    // Initialize pins
    pinMode(elevatorMotorPin1, OUTPUT);
    pinMode(elevatorMotorPin2, OUTPUT);
    pinMode(sortingMotorPin1, OUTPUT);
    pinMode(sortingMotorPin2, OUTPUT);
    pinMode(ledApple, OUTPUT);
    pinMode(ledOnion, OUTPUT);
    pinMode(ledPotato, OUTPUT);

    // Start serial communication
    Serial.begin(9600);

    // Ensure all motors and LEDs are off initially
    stopElevator();
    stopSorting();
    turnOffAllLEDs();

    // Start elevator motor
    runElevator();
}

void loop()
{
    // Check for incoming serial data
    if (Serial.available() > 0)
    {
        receivedFruit = Serial.readStringUntil('\n');
        receivedFruit.trim(); // Remove any whitespace

        // Turn off all LEDs before setting new state
        turnOffAllLEDs();

        // Process the received fruit
        if (receivedFruit == "Apple")
        {
            sortProcess(ledApple, HIGH); // Turn on Apple LED
        }
        else if (receivedFruit == "Onion")
        {
            sortProcess(ledOnion, HIGH); // Turn on Onion LED
        }
        else if (receivedFruit == "Potato")
        {
            sortProcess(ledPotato, HIGH); // Turn on Potato LED
        }
    }
}