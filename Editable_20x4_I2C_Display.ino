#include <Wire.h> //<SoftwareWire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
//SoftwareWire init(A0, A1);
hd44780_I2Cexp lcd;//(init);

const int Left = 2, Right = 3, Up = 4, Down = 5, AddChar = 6,
BackSp = 7, PrevLn = 8, NextLn = 9, AddLn = 10, DelLn = 11; // Initialize inputs

unsigned long lastDebounceTime = 0, lastCursorTime = 0, currentTime = 0;
const unsigned long debounceDelay = 125, dispDebounce = 100; // Debounce time in milliseconds

// Define max lines and track current count
const int MAX_LINES = 64;
String text[MAX_LINES] = {
  "Hello world!",
  "This is a display.",
  "And it is super cool!"
};  // Pre-allocate array
int numLines = 4;// Track actual lines used
String line0 = text[0], line1 = text[1], line2 = text[2], currentLine = line0, cursorDir = "up";
int cursorIDX[2] = {0, 0}, textStart[3] = {0, 0, 0}, textEnd[3] = {line0.length(), line1.length(), line2.length()}, charIndex = 0, lineNum = 0, firstLine = 0, lastLine = 3;
char currentChar = 'a';
bool cursorTouch = true;

byte cursorUpTouch[8] = {
0b00100,
0b01110,
0b11111,
0b01110,
0b01110,
0b01110,
0b00000,
0b00000  
};//cursor texture when touching character above it

byte cursorUpNoTouch[8] = {
0b00000,
0b00100,
0b01110,
0b11111,
0b01110,
0b01110,
0b01110,
0b00000
};//cursor texture when NOT touching character above it

byte cursorDownTouch[8] = {
0b00000,
0b01110,
0b01110,
0b01110,
0b11111,
0b01110,
0b00100,
0b00000  
};//cursor texture when touching character below it

byte cursorDownNoTouch[8] = {
0b01110,
0b01110,
0b01110,
0b11111,
0b01110,
0b00100,
0b00000,
0b00000
};//cursor texture when NOT touching character below it

byte textCont[8] = {
0b00000,
0b00000,
0b00000,
0b00000,
0b00000,
0b00000,
0b10101,
0b00000
};//...

byte backSlash[8] = {
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000,
  0b00000
};//handle backslash '\' printing

//ASCII Character dictionary for LCD and String var "text" to use
char ASCII[] = {'a','A','b','B','c','C','d','D','e','E','f','F',
                'g','G','h','H','i','I','j','J','k','K','l','L',
                'm','M','n','N','o','O','p','P','q','Q','r','R',
                's','S','t','T','u','U','v','V','w','W','x','X',
                'y','Y','z','Z','1','2','3','4','5','6','7','8',
                '9','0','!','@','#','$','%','^','&','*','(',')',
                '-','_','=','+','[','{',']','}','\\','|',',','<',
                '.','>','/','?',';',':','\'','"',' '};

// Function to find the index of a character in the ASCII array
int indexOf(char value) {
  for (int i = 0; i < sizeof(ASCII); i++) {
    if (ASCII[i] == value) {
      return i;
    }
  }
  return 0; // Default to first character if not found
}

// Add this function to handle special character display
void lcdPrint(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (str.charAt(i) == '\\') {  // Our custom backslash character code
      lcd.write(byte(3));
    } else {
      lcd.print(str.charAt(i));
    }
  }
}

void setup() {
  Wire.begin(); //init.begin();
  int status = lcd.begin(20,4);
  /*
  byte* customChars[] = {cursorUpTouch, cursorUpNoTouch, cursorDownTouch, cursorDownNoTouch, textCont, backSlash};
  for (int i = 0, len = sizeof(customChars); i < len; i++) {
    lcd.createChar(i, customChars[i]);
  }
  */
  
  lcd.createChar(0, cursorUpTouch);//create a new character
  lcd.createChar(1, cursorUpNoTouch);//create a new character
  lcd.createChar(2, cursorDownTouch);//create a new character
  lcd.createChar(3, cursorDownNoTouch);//create a new character
  lcd.createChar(4, textCont);//create a new character
  lcd.createChar(5, backSlash);//create custom backslash
  
  lcd.begin(20,4);//LCD is 20 chars long and 4 chars wide
  for (int i = 2; i < 12; i++){
    pinMode(i, INPUT_PULLUP);
  }
  lineNum = 0;
  currentLine = text[lineNum];
  //cursorIDX = {0,0};
  currentChar = currentLine.charAt(cursorIDX[1]);
  charIndex = indexOf(currentChar);
}

void loop() {
  currentTime = millis();
  lineNum = constrain(lineNum, 0, numLines);
  
  if (currentLine.length() > 20){
    textStart[lineNum] = constrain(textStart[lineNum], 0, currentLine.length() - 16);
    textEnd[lineNum] = textStart[lineNum] + 19;
    cursorIDX[0] = constrain(cursorIDX[0], 0, 19);
  }else{
    textStart[lineNum] = 0;
    textEnd[lineNum] = currentLine.length()-1;
    cursorIDX[0] = constrain(cursorIDX[0], 0, currentLine.length()-1);
  }
  
  // Handle button presses with debounce
  if (currentTime - lastDebounceTime > debounceDelay - dispDebounce) {

    if ((digitalRead(Left) == LOW) || (digitalRead(Right) == LOW) ||
        (digitalRead(Up) == LOW) || (digitalRead(Down) == LOW) ||
        (digitalRead(AddChar) == LOW) || (digitalRead(BackSp) == LOW) ||
        (digitalRead(PrevLn) == LOW) || (digitalRead(NextLn) == LOW) ||
        (digitalRead(AddLn) == LOW) || (digitalRead(DelLn) == LOW)){
      lastCursorTime = currentTime;
      if ((digitalRead(Left) == LOW) || (digitalRead(Right) == LOW)){
        cursorTouch = false;
      }else{
        cursorTouch = true;
      }
      
      if (currentLine.length() > 0){
        currentChar = currentLine.charAt(cursorIDX[1] + textStart[lineNum]);
        charIndex = indexOf(currentChar);
      }
    }
  lastDebounceTime = currentTime;
  lcd.clear();
    
    // Move cursor left
    if ((digitalRead(Left) == LOW) && (currentLine.length() > 0)){
      if (cursorIDX[0] > 0) {
        cursorIDX[0]--;
      }else if (textStart[lineNum] == 0){//wrap around
        cursorIDX[0] = constrain(cursorIDX[0], currentLine.length() - 1, 19);
        textStart[lineNum] = currentLine.length() - 20;
      }else{
        textStart[lineNum]--;
      }
    }
    
    // Move cursor right
    if ( ((digitalRead(Right) == LOW) && (currentLine.length() > 0)) || 
    ((digitalRead(AddChar) == LOW) && (currentLine.length() > 19)) ){
      if ((cursorIDX[0] < 19) && (cursorIDX[0] < currentLine.length() - 1)){
        cursorIDX[0]++;
      }else if (textEnd[lineNum] == currentLine.length() -1){//wrap around
        cursorIDX[0] = 0;
        textStart[lineNum] = 0;
      }else{
        textStart[lineNum]++;
      }
    }

    // Change character (up)
    if (digitalRead(Up) == LOW) {
      charIndex = (charIndex + 1) % sizeof(ASCII);
      currentLine.setCharAt(cursorIDX[0] + textStart[lineNum], ASCII[charIndex]);
      text[lineNum] = currentLine;
    }
    
    // Change character (down)
    if (digitalRead(Down) == LOW) {
      charIndex = (charIndex - 1 + sizeof(ASCII)) % sizeof(ASCII);
      currentLine.setCharAt(cursorIDX[0] + textStart[lineNum], ASCII[charIndex]);
      text[lineNum] = currentLine;
    }
    
    //Add character to end
    if (digitalRead(AddChar) == LOW){
      currentLine = currentLine + " ";
      if (currentLine.length() > 20){
        textStart[lineNum] = currentLine.length() - 19;
        cursorIDX[0] = 19;
      }
      cursorIDX[0] = constrain(cursorIDX[0], currentLine.length() - 1, 19);
      textStart[lineNum] = constrain(textStart[lineNum], 0, currentLine.length() - 20);
      text[lineNum] = currentLine;
    }
    
    //Remove character at cursor
    if (digitalRead(BackSp) == LOW){
      if (currentLine.length() > 0){
        if (cursorIDX[0] + textStart[lineNum] == 0){
          currentLine = currentLine.substring(1, currentLine.length());
        }else if (cursorIDX[0] + textStart[lineNum] == currentLine.length() - 1){
          currentLine = currentLine.substring(0, currentLine.length() - 1);
          cursorIDX[0] = constrain(cursorIDX[0], currentLine.length() - 1, 19);
          if (currentLine.length() > 20){
            textStart[lineNum]--;
            cursorIDX[0] = 19;
          }
        }else{
          currentLine = currentLine.substring(0, cursorIDX[0] + textStart[lineNum]) +
            currentLine.substring(cursorIDX[0] + textStart[lineNum] + 1, currentLine.length());
        }
      }
      if (currentLine.length() < 2){
        textStart[lineNum] = 0;
      }
      text[lineNum] = currentLine;
    }
    
    //Previous line; Move cursor up
    if ((digitalRead(PrevLn) == LOW) && (numLines > 0)){
      lineNum = (lineNum - 1 + numLines) % numLines;
      currentLine = text[lineNum];
      if (cursorIDX[1] > 0) {
        cursorIDX[1]--;
      }else if (firstLine == 0){//wrap around
        cursorIDX[1] = constrain(cursorIDX[1], 0, 3);
        firstLine = sizeof(text) - 3;
      }else{
        firstLine--;
        lastLine--;
      }
    }
    
    //Next line; Move cursor down
    if ((digitalRead(NextLn) == LOW) && (numLines > 0)){
      lineNum = (lineNum + 1) % numLines;
      currentLine = text[lineNum];
      if ((cursorIDX[1] < 3) && (cursorIDX[1] < numLines - 1)){
        cursorIDX[1]++;
      }else if (lastLine == numLines -1){//wrap around
        cursorIDX[1] = 0;
        firstLine = 0;
      }else{
        firstLine++;
        lastLine++;
      }
    }
    
    //Add new line
if (digitalRead(AddLn) == LOW){
  if (numLines < MAX_LINES) {
    // Shift all lines after current position
    for (int i = numLines; i > lineNum + 1; i--) {
      text[i] = text[i-1];
    }
    text[lineNum + 1] = " ";  // Create empty line
    numLines++;
    lineNum++;  // Move to new line
    currentLine = text[lineNum];
    cursorIDX[0] = 0;
    textStart[lineNum] = 0;
  }
}
    //Delete current line
if (digitalRead(DelLn) == LOW){
  // Shift all lines after current position
  for (int i = lineNum; i < numLines - 1; i++) {
    text[i] = text[i+1];
  }
  numLines--;
  lineNum = constrain(lineNum, 0, numLines - 1);
  currentLine = text[lineNum];
  cursorIDX[0] = 0;
  textStart[lineNum] = 0;
}
    
    lastDebounceTime = currentTime;
  }
  if(currentTime - lastCursorTime > 500){
    cursorTouch = !cursorTouch;
    lastCursorTime = currentTime;
  }
  
  // Display text
  String displayedText[3] = {line0, line1, line2};
  String Line = "";
  lcd.setCursor(0, 0);
  for (int i = 0, j = 0; i < 3; i++) {
    if (i == cursorIDX[1]){
      // Display cursor indicator
      lcd.setCursor(cursorIDX[0], i);
      if(cursorTouch){
        if (currentLine.length() > 0){
          lcd.write(byte(cursorDir == "down" ? 2 : 0));
        }else{
          lcd.setCursor(0, 1);
          if (numLines == 0){
            lcd.print("Add a line.");
          }else{
            lcd.print("Add a character.");
          }
        }
      }else{
        if (currentLine.length() > 0){
          lcd.write(byte(cursorDir == "down" ? 3 : 1));
        }else{
          lcd.clear();
        }
      }
      j++;
    }
    j++;
    Line = displayedText[i];
    lcd.setCursor(0, j-1);
    lcdPrint(Line.substring(textStart[i], textEnd[i] + 1));
    if (textStart[i] > 0){
      lcd.setCursor(0, j-1);
      lcd.write(byte(4));
    }//print "..."
    
    if ((textEnd[i] < Line.length() - 1) && (Line.length() > 19)){
      lcd.setCursor(19, j-1);
      lcd.write(byte(4));
    }// print "..."
  }
  delay(dispDebounce); // Small delay to prevent display flickering
}
