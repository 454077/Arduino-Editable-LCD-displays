#include <LiquidCrystal.h>

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
const int Left = 2, Right = 3, Up = 4, Down = 5, AddChar = 6,
BackSp = 7, PrevLn = 8, NextLn = 9, AddLn = 10, DelLn = 11; // Initialize inputs

unsigned long lastDebounceTime = 0, lastCursorTime = 0, currentTime = 0;
const unsigned long debounceDelay = 125, dispDebounce = 100; // Debounce time in milliseconds

// Define max lines and track current count
const int MAX_LINES = 32;
String *text[MAX_LINES] = {
  "Hello world!"
};  // Pre-allocate array
int numLines = 1;// Track actual lines used
String line = text[0];
int cursorIDX = 0, charIndex = 0, lineNum = 0, textStart = 0, textEnd = line.length();
char currentChar = 'a';
bool cursorTouch = true;

byte cursorUp[8] = {
0b00100,
0b01110,
0b11111,
0b01110,
0b01110,
0b01110,
0b00000,
0b00000  
};//cursor texture when touching character above it

byte cursorDown[8] = {
0b00000,
0b00100,
0b01110,
0b11111,
0b01110,
0b01110,
0b01110,
0b00000
};//cursor texture when NOT touching character above it

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
  lcd.createChar(0, cursorUp);//create a new character
  lcd.createChar(1, cursorDown);//create a new character
  lcd.createChar(2, textCont);//create a new character
  lcd.createChar(3, backSlash);//create custom backslash
  lcd.begin(16, 2);//LCD is 16 chars long and 2 chars wide
  for (int i = 2; i < 12; i++){
    pinMode(i, INPUT_PULLUP);
  }
  lineNum = 0;
  line = text[lineNum];
  cursorIDX = 0;
  currentChar = line.charAt(cursorIDX);
  charIndex = indexOf(currentChar);
}

void loop() {
  currentTime = millis();
  lineNum = constrain(lineNum, 0, numLines);
  
  if (line.length() > 16){
    textStart = constrain(textStart, 0, line.length() - 16);
    textEnd = textStart + 15;
    cursorIDX = constrain(cursorIDX, 0, 15);
  }else{
    textStart = 0;
    textEnd = line.length()-1;
    cursorIDX = constrain(cursorIDX, 0, line.length()-1);
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
      
      if (line.length() > 0){
        currentChar = line.charAt(cursorIDX + textStart);
        charIndex = indexOf(currentChar);
      }
    }
  lastDebounceTime = currentTime;
  lcd.clear();
    
    // Move cursor left
    if ((digitalRead(Left) == LOW) && (line.length() > 0)){
      if (cursorIDX > 0) {
        cursorIDX--;
      }else if (textStart == 0){//wrap around
        cursorIDX = constrain(cursorIDX, line.length() - 1, 15);
        textStart = line.length() - 16;
      }else{
        textStart--;
      }
    }
    
    // Move cursor right
    if ( ((digitalRead(Right) == LOW) && (line.length() > 0)) || 
    ((digitalRead(AddChar) == LOW) && (line.length() > 15)) ){
      if ((cursorIDX < 15) && (cursorIDX < line.length() - 1)){
        cursorIDX++;
      }else if (textEnd == line.length() -1){//wrap around
        cursorIDX = 0;
        textStart = 0;
      }else{
        textStart++;
      }
    }
    
    // Change character (up)
    if (digitalRead(Up) == LOW) {
      charIndex = (charIndex + 1) % sizeof(ASCII);
      line.setCharAt(cursorIDX + textStart, ASCII[charIndex]);
      text[lineNum] = line;
    }
    
    // Change character (down)
    if (digitalRead(Down) == LOW) {
      charIndex = (charIndex - 1 + sizeof(ASCII)) % sizeof(ASCII);
      line.setCharAt(cursorIDX + textStart, ASCII[charIndex]);
      text[lineNum] = line;
    }
    
    //Add character to end
    if (digitalRead(AddChar) == LOW){
      line = line + " ";
      if (line.length() > 16){
        textStart = line.length() - 15;
        cursorIDX = 15;
      }
      cursorIDX = constrain(cursorIDX, line.length() - 1, 15);
      textStart = constrain(textStart, 0, line.length() - 16);
      text[lineNum] = line;
    }
    
    //Remove character at cursor
    if (digitalRead(BackSp) == LOW){
      if (line.length() > 0){
        if (cursorIDX + textStart == 0){
          line = line.substring(1, line.length());
        }else if (cursorIDX + textStart == line.length() - 1){
          line = line.substring(0, line.length() - 1);
          cursorIDX = constrain(cursorIDX, line.length() - 1, 15);
          if (line.length() > 16){
            textStart--;
            cursorIDX = 15;
          }
        }else{
          line = line.substring(0, cursorIDX + textStart) +
            line.substring(cursorIDX + textStart + 1, line.length());
        }
      }
      if (line.length() < 17){
        textStart = 0;
      }
      text[lineNum] = line;
    }
    
    //Previous line
    if ((digitalRead(PrevLn) == LOW) && (numLines > 0)){
      lineNum = (lineNum - 1 + numLines) % numLines;
      line = text[lineNum];
    }
    
    //Next line
    if ((digitalRead(NextLn) == LOW) && (numLines > 0)){
      lineNum = (lineNum + 1) % numLines;
      line = text[lineNum];
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
    line = text[lineNum];
    cursorIDX = 0;
    textStart = 0;
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
  line = text[lineNum];
  cursorIDX = 0;
  textStart = 0;
}
    
    lastDebounceTime = currentTime;
  }
  if(currentTime - lastCursorTime > 500){
    cursorTouch = !cursorTouch;
    lastCursorTime = currentTime;
  }
  
  // Display text
  lcd.setCursor(0, 0);
  lcdPrint(line.substring(textStart, textEnd + 1));
  if (textStart > 0){
    lcd.setCursor(0, 0);
    lcd.write(byte(2));
  }//print "..."
  
  if ((textEnd < line.length() - 1) && (line.length() > 16)){
    lcd.setCursor(15, 0);
    lcd.write(byte(2));
  }// print "..."
  
  // Display cursor indicator
  lcd.setCursor(cursorIDX, 1);
  if(cursorTouch){
    if (line.length() > 0){
      lcd.write(byte(0));
    }else{
      lcd.setCursor(0, 1);
      if (numLines == 0){
        lcd.print("Add a line.");
      }else{
        lcd.print("Add a character.");
      }
    }
  }else{
    if (line.length() > 0){
      lcd.write(byte(1));
    }else{
      lcd.clear();
    }
  }
  delay(dispDebounce); // Small delay to prevent display flickering
}
