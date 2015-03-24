// Example2 code for Brian Schmalz's Easy Driver Example page
// http://www.schmalzhaus.com/EasyDriver/EasyDriverExamples.html

int Distance = 0;  // Record the number of steps we've taken

void setup() {                
  pinMode(8, OUTPUT);     
  pinMode(9, OUTPUT);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
}

void loop() {
  digitalWrite(9, HIGH);
  delayMicroseconds(100);          
  digitalWrite(9, LOW); 
  delayMicroseconds(100);
  Distance = Distance + 1;   // record this step
  
  // Check to see if we are at the end of our move
  if (Distance == 3600)
  {
    // We are! Reverse direction (invert DIR signal)
    if (digitalRead(8) == LOW)
    {
      digitalWrite(8, HIGH);
    }
    else
    {
      digitalWrite(8, LOW);
    }
    // Reset our distance back to zero since we're
    // starting a new move
    Distance = 0;
    // Now pause for half a second
    delay(500);
  }
}
