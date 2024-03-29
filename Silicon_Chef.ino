/** 
  * Silicon Chef Competition - 11/17/12
  * Team "The Corn Flowers"
  * Bryant Pong - Computer Science
  * Raymond Tse - Computer Science
  * Thomas Hartmann - Electrical
  * Joe Nied - Electrical
  * Last Modified: 1/26/2013
**/

/** Software Libraries **/

// Library to control the 16x2 LCD Display
#include <LiquidCrystal.h>

// Library to control the Servos
#include <Servo.h>

/** END SOFTWARE LIBRARIES **/

/** Function Prototypes **/
void setPassword(); 
boolean checkPassword();
void getUserPassword();
void generateSecret();
/** END FUNCTION PROTOTYPES **/

/** Global Variables **/

/** 
  * LCD Display Object
  * Pins: 
  * RS - Arduino Digital 12
  * Enable - Arduino Digital 11
  * D4 - Arduino Digital 5
  * D5 - Arduino Digital 4
  * D6 - Arduino Digital 3
  * D7 - Arduino Digital 2
**/
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Array to hold the password (range: 0000 - 9999)
int password[4];

// Array to hold hashed password
int hashedPassword[4];

// Array to hold the secrets
int secrets[10];

// Array to hold the user's attempted password
int attemptedPassword[4];

// Potentiometer Pins
const int POT1 = 0;
const int POT2 = 1;
const int POT3 = 2;
const int POT4 = 3;

// Confirmation Switch Pin
const int ENTER = 13;

// LED Pin
const int ledPin = 7;

// Current Password "Secret" Position
int secretPosition;

// Servo Motor Object
Servo door;

/** END GLOBAL VARIABLES **/

// Setup Function
void setup() {
  // Enable Serial Port for debugging purposes - DISABLE WHEN NOT NEEDED
  Serial.begin(9600);
  
  // Set up the number of LCD's rows/colums (16 x 2)
  lcd.begin(16, 2);
  
  // Have the user input the initial password
  setPassword();
  
  // Generate an initial random secret array
  // Since the secret must be different everytime, we will
  // use the floating readings on A4 to seed the random generator
  generateSecret();
  
  // Debug Statements
  Serial.println("Setup Successfully Completed");
  Serial.println("Your secret array is: ");
  
  for(unsigned int i = 0; i < 10; i++) {
    Serial.println(secrets[i]);
  }
  
  // Prepare the LCD to display the passwords
  lcd.clear();
  
  // Initialize the Servo
  door.attach(9);
  
  // Lock the door
  door.write(117);
  
  // LED pin 
  pinMode(ledPin, OUTPUT);
  
  delay(1000);
}

// Main Loop Function
void loop() {
  
  // Output the secret string 
  for(unsigned int i = 0; i < 10; i++) {
    lcd.setCursor(i, 0);
    lcd.print(secrets[i]);
  }
  
  // Actively check for the next attempted password 
  getUserPassword();
 
  // If the password is incorrect, alert the user to the bad password
  if(checkPassword()) { 
    lcd.clear();
    lcd.print("Please Try Again");
  } else { // The password is correct!
    
    // Tell the user the password is correct
    lcd.clear();
    lcd.print("Correct!");
    
    // Turn on the LED to alert the user that the door is open
    digitalWrite(ledPin, HIGH);
    
    // Open the door for 10 seconds, then close it
    door.write(160);
    delay(10000);
    door.write(117);
    
    // Turn off the LED after the door is closed
    digitalWrite(ledPin, LOW);
  }
  
  delay(1500);
  lcd.clear();
}

// Function to set up the initial password - COMPLETED
void setPassword() {
  int POT1Read;
  int POT2Read;
  int POT3Read;
  int POT4Read;
  
  // Keep waiting for the user to set a password 
  while(digitalRead(ENTER) != 1) {
     // Prompt the user to set a password
    lcd.setCursor(0, 0);
    lcd.print("Set a Password");
    
    // Get ALL Input Readings 
    // Divide by 105 to get the readings between 0 - 9
    POT1Read = analogRead(POT1) / 105;
    POT2Read = analogRead(POT2) / 105;
    POT3Read = analogRead(POT3) / 105;
    POT4Read = analogRead(POT4) / 105;
    
    // Print out all the password digits
    lcd.setCursor(0, 1);
    lcd.print(POT1Read);
    
    lcd.setCursor(4, 1);
    lcd.print(POT2Read);
    
    lcd.setCursor(8, 1);
    lcd.print(POT3Read);
    
    lcd.setCursor(12, 1);
    lcd.print(POT4Read);
  }
  
  // Store the password into the password array
  password[0] = POT1Read;
  password[1] = POT2Read;
  password[2] = POT3Read;
  password[3] = POT4Read;
  
  // Debug information
  Serial.println(POT1Read);
  Serial.println(POT2Read);
  Serial.println(POT3Read);
  Serial.println(POT4Read);
  
  delay(500);
  
  // Ask the user to choose a secret position
  lcd.clear();
  
  while(digitalRead(ENTER) != 1) {
    lcd.setCursor(0, 0);
    lcd.print("Choose SecretPos");
    POT1Read = analogRead(POT1) / 105;
    lcd.setCursor(0, 1);
    lcd.print(POT1Read);
  }
  
  secretPosition = POT1Read;
  
  // Thank the user for their input
  lcd.clear();
  lcd.print("THANK YOU!");
  
  // Debug Information
  Serial.print("Your secret position is: ");
  Serial.println(secretPosition);
  
  delay(1500);
}

// Function to check whether a user's password is correct
boolean checkPassword() {
  // Copy the password into the hashed password
  for(unsigned int i = 0; i < 4; i++) {
    /* 
     * The hashing algorithm is:
     * 1) Take the secretPosition'th digit from the string of randomly generated numbers
     * 2) Add that digit to each digit of the base password.  
     * 3) Take that sum, take the modulus base 10 of that.
     * 4) Repeat for the other 3 digits, this is the hashed password
     *
     * Example:
     * Let the base password be: 1 2 3 4
     * Let the secret position be 0 (start counting at 0, with 0 being the leftmost digit and 9 being the rightmost digit
     * Randomly generated string: 8573298732
     * The 0th digit (leftmost is 8).
     * Add 8 to all digits:
     * 1 + 8 = 9 % 10 = 9
     * 2 + 8 = 10 % 10 = 0
     * 3 + 8 = 11 % 10 = 1
     * 4 + 8 = 12 % 10 = 2
     * Hashed Password to enter in: 9 0 1 2
     */
    
    hashedPassword[i] = (password[i] + secrets[secretPosition]) % 10;
  }
  
  // For all numbers in the password, check for any character mismatches
  for(unsigned int i = 0; i < 4; i++) {
    // If a number does not match, this password is incorrect
    if(hashedPassword[i] != attemptedPassword[i]) {
      return false;
    }
  }  
  
  // Otherwise, this password must be good
  return true;
}

// Function to read in the next password
void getUserPassword() {
  // Variables to hold the potentiometer readings
  int POT1Read;
  int POT2Read;
  int POT3Read;
  int POT4Read;
  
  // Keep waiting for the user to set a password 
  while(digitalRead(ENTER) != 1) {
    /*
     * Get ALL Input Readings.  The potentiometers read from 0 - 1023, 
     * so divide by 105 to get the readings between 0 - 9.
     */
    POT1Read = analogRead(POT1) / 105;
    POT2Read = analogRead(POT2) / 105;
    POT3Read = analogRead(POT3) / 105;
    POT4Read = analogRead(POT4) / 105;
    
    // Print out all the password digits
    lcd.setCursor(0, 1);
    lcd.print(POT1Read);
    
    lcd.setCursor(4, 1);
    lcd.print(POT2Read);
    
    lcd.setCursor(8, 1);
    lcd.print(POT3Read);
    
    lcd.setCursor(12, 1);
    lcd.print(POT4Read);
  }
  
  // Store the next attempted password attempt
  attemptedPassword[0] = POT1Read;
  attemptedPassword[1] = POT2Read;
  attemptedPassword[2] = POT3Read;
  attemptedPassword[3] = POT4Read;
  
  // Debug Only
  Serial.println("You entered: ");
  Serial.println(POT1Read);
  Serial.println(POT2Read);
  Serial.println(POT3Read);
  Serial.println(POT4Read);
}

// Function to generate a new secret string
void generateSecret() {
  // We need to generate 10 random numbers
  for(unsigned int i = 0; i < 10; i++) {
    // For every digit, randomly seed the random function using analog pin 4 (this is a floating pin/not connected)
    randomSeed(analogRead(4));
    secrets[i] = random(0, 10);
  } 
}

