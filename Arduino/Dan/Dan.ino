
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0

// Motor 1 (Conveyor Belt)
int bluePin = 8; 
int pinkPin = 9; 
int yellowPin = 10; 
int orangePin = 11; 

// Motor 2 (Basket Alignment)
int bluePin2 = 2;
int pinkPin2 = 3;
int yellowPin2 = 4;
int orangePin2 = 5;

int currentStep = 0;
int currentStep2 = 0;

String currentCommand = "";
unsigned long detectionStartTime = 0;
unsigned long stopStartTime = 0;
bool motorStopped = false;

const int nStepsPerRevolution = 1600; // Steps per revolution for Motor 2
char currentNode = 'B'; // Motor 2's current position (A=Apple, B=Onion, C=Potato)
char targetNode = 'B'; 

void setup() {
    Serial.begin(9600);
    // Motor 1 pins
    pinMode(bluePin, OUTPUT);
    pinMode(pinkPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(orangePin, OUTPUT);
    // Motor 2 pins
    pinMode(bluePin2, OUTPUT);
    pinMode(pinkPin2, OUTPUT);
    pinMode(yellowPin2, OUTPUT);
    pinMode(orangePin2, OUTPUT);
    // Initialize motors to OFF
    digitalWrite(bluePin, LOW);
    digitalWrite(pinkPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(orangePin, LOW);
    digitalWrite(bluePin2, LOW);
    digitalWrite(pinkPin2, LOW);
    digitalWrite(yellowPin2, LOW);
    digitalWrite(orangePin2, LOW);
    Serial.println("Arduino started");
}

void loop() {
    Belt1_Action();
}

// Moves Motor 2 to align the target basket (A/B/C)
void MoveToNode(char targetNode) {
    if (targetNode == currentNode) {
        Serial.println("No movement needed");
        return; // No movement needed
    }

    int stepsToMove = 0;
    int direction = CLOCKWISE;

    // Calculate steps and direction based on current and target nodes
    if (currentNode == 'B') {
        if (targetNode == 'A') { 
            stepsToMove = (nStepsPerRevolution * 4) / 6; // 4 inches CCW
            direction = COUNTERCLOCKWISE;
        } 
        else if (targetNode == 'C') {
            stepsToMove = (nStepsPerRevolution * 4) / 6; // 4 inches CW
            direction = CLOCKWISE;
        }
    } 
    else if (currentNode == 'A') {
        if (targetNode == 'B') {
            stepsToMove = (nStepsPerRevolution * 4) / 6; // 4 inches CW
            direction = CLOCKWISE;
        } 
        else if (targetNode == 'C') {
            stepsToMove = (nStepsPerRevolution * 8) / 6; // 8 inches CW
            direction = CLOCKWISE;
        }
    } 
    else if (currentNode == 'C') {
        if (targetNode == 'B') {
            stepsToMove = (nStepsPerRevolution * 4) / 6; // 4 inches CCW
            direction = COUNTERCLOCKWISE;
        } 
        else if (targetNode == 'A') {
            stepsToMove = (nStepsPerRevolution * 8) / 6; // 8 inches CCW
            direction = COUNTERCLOCKWISE;
        }
    }

    Serial.print("Moving to node ");
    Serial.print(targetNode);
    Serial.print(" (");
    Serial.print(stepsToMove);
    Serial.println(" steps)");

    // Move Motor 2 step-by-step
    for (int i = 0; i < stepsToMove; i++) {
        moveMotor2(direction);
        delay(5); // Small delay for stability
    }

    currentNode = targetNode; // Update position
    Serial.println("Reached node ");
    Serial.println(targetNode);
}

// Controls the conveyor belt (Motor 1)
void Belt1_Action() {
    // Check for new serial data
    if (Serial.available() > 0) {
        String newCommand = Serial.readStringUntil('\n');
        newCommand.trim();
        Serial.print("Received: ");
        Serial.println(newCommand);

        // Map fruit names to nodes
        if (newCommand == "Apple") {
            targetNode = 'A';
            motorStopped = false;
            detectionStartTime = millis(); // Start 3-second countdown
            currentCommand = newCommand;
            Serial.println("Processing Apple (Node A)");
        }
        else if (newCommand == "Onion") {
            targetNode = 'B';
            motorStopped = false;
            detectionStartTime = millis();
            currentCommand = newCommand;
            Serial.println("Processing Onion (Node B)");
        }
        else if (newCommand == "Potato") {
            targetNode = 'C';
            motorStopped = false;
            detectionStartTime = millis();
            currentCommand = newCommand;
            Serial.println("Processing Potato (Node C)");
        }
        else {
            Serial.println("Unknown command");
        }
    }

    // Process valid fruit commands
    if (currentCommand == "Apple" || currentCommand == "Onion" || currentCommand == "Potato") {
        if (!motorStopped) {
            // Run belt for 3 seconds, then stop and align basket
            if (millis() - detectionStartTime >= 3000) {
                motorStopped = true;
                stopStartTime = millis();
                // Stop Motor 1 (belt)
                digitalWrite(bluePin, LOW);
                digitalWrite(pinkPin, LOW);
                digitalWrite(yellowPin, LOW);
                digitalWrite(orangePin, LOW);
                Serial.println("Belt stopped");
                // Align Motor 2 (basket)
                MoveToNode(targetNode);
            } 
            else {
                moveMotor1(COUNTERCLOCKWISE); // Keep belt moving
            }
        } 
        else {
            // After 5 seconds stopped, resume belt
            if (millis() - stopStartTime >= 5000) {
                motorStopped = false;
                currentCommand = ""; // Clear command to resume default
                detectionStartTime = 0;
                Serial.println("Resuming belt");
            }
        }
    } 
    else {
        // Default: run belt continuously
        moveMotor1(COUNTERCLOCKWISE);
        // Serial.println("Running belt (default)");
    }
}

// Steps Motor 1 (conveyor belt)
void moveMotor1(int direction) {
    switch(currentStep) {
        case 0:
            digitalWrite(bluePin, HIGH);
            digitalWrite(pinkPin, HIGH);
            digitalWrite(yellowPin, LOW);
            digitalWrite(orangePin, LOW);
            break;
        case 1:
            digitalWrite(bluePin, LOW);
            digitalWrite(pinkPin, HIGH);
            digitalWrite(yellowPin, HIGH);
            digitalWrite(orangePin, LOW);
            break;
        case 2:
            digitalWrite(bluePin, LOW);
            digitalWrite(pinkPin, LOW);
            digitalWrite(yellowPin, HIGH);
            digitalWrite(orangePin, HIGH);
            break;
        case 3:
            digitalWrite(bluePin, HIGH);
            digitalWrite(pinkPin, LOW);
            digitalWrite(yellowPin, LOW);
            digitalWrite(orangePin, HIGH);
            break;
    }
    currentStep = (direction == CLOCKWISE) ? (currentStep - 1 + 4) % 4 : (currentStep + 1) % 4;
    delayMicroseconds(2000); // Control belt speed
}

// Steps Motor 2 (basket alignment)
void moveMotor2(int direction) {
    switch(currentStep2) {
        case 0:
            digitalWrite(bluePin2, HIGH);
            digitalWrite(pinkPin2, HIGH);
            digitalWrite(yellowPin2, LOW);
            digitalWrite(orangePin2, LOW);
            break;
        case 1:
            digitalWrite(bluePin2, LOW);
            digitalWrite(pinkPin2, HIGH);
            digitalWrite(yellowPin2, HIGH);
            digitalWrite(orangePin2, LOW);
            break;
        case 2:
            digitalWrite(bluePin2, LOW);
            digitalWrite(pinkPin2, LOW);
            digitalWrite(yellowPin2, HIGH);
            digitalWrite(orangePin2, HIGH);
            break;
        case 3:
            digitalWrite(bluePin2, HIGH);
            digitalWrite(pinkPin2, LOW);
            digitalWrite(yellowPin2, LOW);
            digitalWrite(orangePin2, HIGH);
            break;
    }
    currentStep2 = (direction == CLOCKWISE) ? (currentStep2 - 1 + 4) % 4 : (currentStep2 + 1) % 4;
    delayMicroseconds(2000); // Control alignment speed
}
