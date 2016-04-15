/*
Welcome to Sink-A-Bank!!!
 
 This is more or less a derivative of the popular board game titled 'Battleships'.
 
 The rules are simple:
 
 -pick a spot in the rows of LEDs using the potentiometer
 -select the spot using the push button
 -hope for the best and hopefully you'll have hit a block in the bank
 -hit 3 blocks in a set and win the game
 
 Enjoy the game!
 
 Author: Anand Subramaniam
 Credit: Laurissa Tokarchuk and Akram Alomainy for the basic code
 */



//All pins that are used are intialised. If changes to the ports are needed, it can be controlled from here
//Some set as constant integers to avoid accidental assignments further down in the code

//Pins used by the shift register
const int data = 2; 
const int clock = 3;
const int latch = 4;

//Pins used by the try indicator LEDs
const int winLED = 5;
const int missLED = 6;
const int hitLED = 7;

//Pins used by other components (potentiometer and button as inputs, speaker pin as an output
const int speakerPin = 8;
const int buttonPin = 9;
const int potpin = 0;

//Used for single LED manipulation
int ledState = 0;
const int ON = HIGH;
const int OFF = LOW;

//Variable used in functions below to help retrieve the guess from the User
int selected;

//Size of the board (number of target LEDs) and the size of a bank slot
const int MAX_BOARD_SIZE = 8; 
const int BANK_SIZE = 3;

//Used to assign the location of cells from the 8 LEDs connected to the shift register
int locationLEDPins[] = {
  0,1,2,3,4,5,6,7};
int locationCells[BANK_SIZE];

//Used later to determine the winning of the game
int numberOfHits = 0;

//number of notes available for any melody, spaces in the notes represent a space
//beats represents the length of 1 note, each number used with its equal in the notes array
//tempo is also set here
int length = 14; 
int beats[] = { 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int tempo = 100;


//function used to play each tone against it's duration
//tones are specified in the playNote() function
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}


//function used to assign frequency of a note to it's name
void playNote(char note, int duration) {
  char names[] = { 
    'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'               };
  int tones[] = { 
    1915, 1700, 1519, 1432, 1275, 1136, 1014, 956               };

  // play the tone corresponding to the note name for the set duration
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}


//Setup is used to run code once the Arduino is switched on
//Use of Serial Monitor is established, whether a pin is used for input or output is also established
//a new game is intialised
void setup()
{
  Serial.begin(9600);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);  
  pinMode(latch, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(potpin, INPUT);
  setupNewGame();  
}

//sets the number of hits to 0, also retrieves the locations of the bank cells
//switches all target LEDs off along with the try indicator LEDs
//prompts the User to select an LED
void setupNewGame(){
  numberOfHits = 0;
  setLocationCells();
  for (int i = 0; i < MAX_BOARD_SIZE; i++){
    changeLED(i,OFF);
    delay(200);
  }
  digitalWrite(winLED, LOW);
  digitalWrite(hitLED, LOW);
  digitalWrite(missLED, LOW);
  Serial.println("Enter a new guess (1-8):");

}


//in-built function that runs over and over again continiously
//the starting melody is played
//sets the outputs, which later need to be put through tests to generate a final result
//boolean is set to determine the start and end of the move (use of conditions)
//uses function to bring in a guess from the User
//checks the guess using function
//returns a result which determines when the game ends
//game resets
void loop()             
{
  performStart();
  int guess = -1;
  String result = "miss";
  boolean isAlive = true;
  while (isAlive){
    guess = getGuess();
    if (guess != 'n') {    
      Serial.print("You guessed:");
      Serial.println(guess);
      result = checkYourself(guess);
      Serial.println(result);
      if (result.compareTo("win") == 0) {
        performWin();
        isAlive = false;
      }
      Serial.println("Enter a new guess (1-8):");     
    }
  }
  setupNewGame();

}


//function used to retrieve an input from the User
//tested to see if the input is valid and within boundaries
//returns the guess
char getGuess(){ 
  int guess = 0;
  int tempGuess;
  pickLED();
  tempGuess = selected;
  if ((tempGuess >= 1 && tempGuess <= (MAX_BOARD_SIZE+'0'))) { 
    guess = tempGuess;  
  }

  return guess;       
}

//function used to select the locations of the cells of the bank
//random number is generated but limited so that the other cells can be generated accordingly whilst fitting withing the number of LEDs
//for loop used to assign the rest of the cells into an array including and based on the random number
void setLocationCells(){
  int startVal = random(0,MAX_BOARD_SIZE-2);
  Serial.print("Let's play SINK-A-BANK!!!");
  for (int i = 0; i < BANK_SIZE; i++) { 
    locationCells[i] = startVal + i;
  }
  Serial.println();
}


//function used to check the guess and return a result accordingly
//the guess is retrieved
//for loop is used to check if the guess matches any of the cells
//if a match is found, number of hits is increased by 1
//the number in the array that was matched is converted into -1, to prevent interuption to the game process
//another test condition, if the number of hits matches the bank size, the user wins
//otherwise the User has hit and the performHit function is used
//if none of the above are met, result is a miss and the performMiss function is performed
//result is returned
String checkYourself(int guess){
  String result = "miss";  
  int gint = guess; 
  for (int i = 0; i < BANK_SIZE; i++){
    if (gint == locationCells[i]){
      numberOfHits++;
      locationCells[i] = -1;
      digitalWrite(locationLEDPins[gint], HIGH);
      if (numberOfHits == BANK_SIZE) {
        result = "win" ;
      } 
      else result = "hit";
      performHit();
      break;
    }   
  }
  if (result == "miss"){
    performMiss();
  }
  return result;
}


//Used with bitwise operations to map against each individual LED
int bits[] = {
  B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000};
int masks[] = {
  B11111110, B11111101, B11111011, B11110111, B11101111, B11011111, B10111111, B01111111};


//function used to control each LED connected to the shift register
//state of the LED in question is cleared
//if the bit is on, it is added to the led state
//the LED is updated by sending the state to the shift register
void changeLED(int led, int state){
  ledState = ledState & masks[led]; 
  if(state == ON){
    ledState = ledState | bits[led];
  } 
  updateLEDs(ledState);
}


//function used to send the state of an LED through to the shift register sequence
//the latch is pulled low to allow data through
//the 8 bits are shifted out to the shift register, with the most significant bit first
//the latch is pulled low to display the data
void updateLEDs(int value){
  digitalWrite(latch, LOW);     
  shiftOut(data, clock, MSBFIRST, value);
  digitalWrite(latch, HIGH); 
}


//function used to allow the User to pick an LED using the potentiometer
//button state is set to off initially
//while loop used to detect the button press
//when not pressed it executes the code
//state of button is checked at the beginning of the loop
//input from the pot is read 
//the value is mapped to allow the selection of an LED through the use of proportions (where 0 reads as 0th LED and 1023 reads 7, 0-7 have 8 LEDs)
//for loop used to light up the respective LED according to the position of the wiper, whilst keeping all the others switched off
//once the button is pressed the temporary chosen variable is assigned to the global variable, ready to be accessed in other functions
//the while loop is broken and the selected LED is returned
int pickLED(){
  int buttontemp = LOW;
  int val = -1;
  int chosen;
  while(buttontemp != HIGH){
    int buttonRead = digitalRead(buttonPin);
    buttontemp = buttonRead;
    val = analogRead(potpin);
    val = map(val, 0, 1023, 0,MAX_BOARD_SIZE-1);
    for(int i=0; i<MAX_BOARD_SIZE;i++){
      if(val == i){
        changeLED(i, ON);
        chosen = i;
      }
      else {
        changeLED(i,OFF);
      }
    }
    if (buttontemp == HIGH){
      selected = chosen;
      delay(1000);
      break;
    }
  }
  return selected;
}


//function to play notes when the game begins
//notes in array are played unless there is a space, which represents a rest
//the delay period is executed between notes
void performStart(){
  char startnotes[] = "c   c   c   C ";
  for (int i = 0; i < length; i++) {
    if (startnotes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote(startnotes[i], beats[i] * tempo);
    }

    delay(tempo / 2); 
  }
}

//function to play notes and turn on hit light when result is a hit
//the performIntenseWait function is played
//there is a small pause for suspense, drama and tension
//the hit light is switched on
//notes in array are played unless there is a space, which represents a rest
//the delay period is executed between notes
//the hit light is switched off
void performHit(){
  performIntenseWait();
  delay(1500);
  digitalWrite(hitLED, HIGH);
  char hitnotes[] = "CCC C";
  for (int i = 0; i < length; i++) {
    if (hitnotes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote(hitnotes[i], beats[i] * tempo);
    }
    delay(tempo / 2); 
  }
  digitalWrite(hitLED, LOW);
}


//function to play notes and turn on miss light when result is a miss
//the performIntenseWait function is played
//there is a small pause for suspense, drama and tension
//the miss light is switched on
//notes in array are played unless there is a space, which represents a rest
//the delay period is executed between notes
//the miss light is switched off
void performMiss(){
  performIntenseWait();
  delay(1500);
  digitalWrite(missLED, HIGH);
  char missnotes[] = "gfe";
  for (int i = 0; i < length; i++) {
    if (missnotes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote(missnotes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 
  }
  digitalWrite(missLED, LOW);
}


//function to play notes and turn on win light when result is a win
//the performIntenseWait function is played
//there is a small pause for suspense, drama and tension
//the win light is switched on
//notes in array are played unless there is a space, which represents a rest
//the delay period is executed between notes
//the target LEDs turn on consecutively in one direction and then turn off consecutively in the opposite direction
//the miss light is switched off
void performWin(){
  char winnotes[] = "c f C f";
  digitalWrite(winLED, HIGH);
  for (int i = 0; i < length; i++) {
    if (winnotes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote(winnotes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 

  }
  for(int i = 0; i<MAX_BOARD_SIZE; i++){
    changeLED(i, HIGH);
    delay(50);
    changeLED(i, LOW);
  }

  for(int i = MAX_BOARD_SIZE; i>0; i--){
    changeLED(i, HIGH);
    delay(50);
    changeLED(i, LOW);
  }
  digitalWrite(winLED, LOW);
}

//function to play notes in a heartbeat pattern to build suspense, drama and tension
//notes in array are played unless there is a space, which represents a rest
//the delay period is executed between notes
void performIntenseWait(){
  char waitnotes[] = "c c  c c  c c  c c ";
  for (int i = 0; i < length; i++) {
    if (waitnotes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote(waitnotes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 
  }
}
