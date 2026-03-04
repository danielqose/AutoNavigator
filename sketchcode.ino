#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // 

#define SCREEN_WIDTH 128 // 
#define SCREEN_HEIGHT 64 // 
#define OLED_RESET    -1 // 
#define SCREEN_ADDRESS 0x3C // 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // 

// --- Pin Definitions based on Schematic ---
const int motorPin = 9;   // Connects to the transistor base to trigger the relay
const int trigPin = 11;   // Ultrasonic TRIG
const int echoPin = 10;   // Ultrasonic ECHO
const int buzzerPin = 8;  // Buzzer 

// --- Settings ---
const int detectionDistance = 15; // Distance in cm to trigger "Artifact Found"
bool isScanning = false;          // Used to prevent OLED flickering

void setup() {
  Serial.begin(9600); // 
  
  // Initialize Pins
  pinMode(motorPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT); // 
  
  digitalWrite(buzzerPin, LOW); // 
  digitalWrite(motorPin, LOW);  // Start with motor off

  // Initialize OLED [cite: 3]
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // [cite: 3]
    Serial.println(F("OLED Failed. Check SDA/SCL wiring.")); // [cite: 3]
    for(;;); // Stops here if screen isn't found [cite: 4]
  }
  
  display.clearDisplay(); // [cite: 4]
  display.display();
}

void loop() {
  long distance = getDistance();

  // If object is detected within threshold
  if (distance > 0 && distance <= detectionDistance) {
    
    // 1. Stop the car immediately
    digitalWrite(motorPin, LOW); 

    // 2. Display Artifact Found [cite: 6]
    display.clearDisplay(); // [cite: 6]
    display.setTextSize(2); // [cite: 4]
    display.setTextColor(SSD1306_WHITE); // [cite: 4]
    display.setCursor(15, 10); // [cite: 6]
    display.println("Artifact"); // [cite: 6]
    display.setCursor(25, 35); // [cite: 6]
    display.println("Found!"); // [cite: 6]
    display.display(); // [cite: 6]

    // 3. Beep for 4 seconds
    tone(buzzerPin, 1000); // Using tone for a clear sound [cite: 7]
    delay(4000);           // 4 second delay as requested
    noTone(buzzerPin);     // [cite: 9]

    // Reset scanning flag so the screen updates when resuming
    isScanning = false; 
    
  } else {
    // No artifact detected -> Scanning & Moving

    // 1. Move the car
    digitalWrite(motorPin, HIGH); 

    // 2. Update Display (only if state changed to prevent flicker)
    if (!isScanning) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(10, 20); // [cite: 5]
      display.println("Scanning..."); // [cite: 5]
      display.display();
      isScanning = true;
    }
    
    delay(50); // Small delay for loop stability
  }
}

// --- Helper Function for Ultrasonic Sensor ---
long getDistance() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Set the trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, returns the sound wave travel time in microseconds
  // The 30000 timeout ensures the code doesn't freeze if nothing is detected
  long duration = pulseIn(echoPin, HIGH, 30000); 
  
  // If duration is 0, it means it timed out (nothing in range)
  if (duration == 0) {
    return 999; 
  }
  
  // Calculate distance in cm (Speed of sound is 0.034 cm/microsecond)
  long distance = duration * 0.034 / 2;
  return distance;
}