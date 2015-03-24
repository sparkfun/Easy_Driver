/***********************************************************
Arduija: Arduino Controlled Ouija Board
Joel Bartlett
Casey Kuhns
SparkFun Electronics
Oct. 22, 2014
https://github.com/JoelEB/Arduija2_1_1

This sketch uses the AccelStepper Library to controll two stpper motors, 
driven by two Big Easy Driver boards from SparkFun. Letters on a Ouija 
board are mapped out, given coordinates, and then massages are spelled 
out by looking up the coordinates in that table. 

Development environment specifics:
Arduino IDE .0+
Arduino Uno

This code is beerware; if you see me at the local pub, 
and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given. 
***********************************************************/
#include <avr/pgmspace.h>
#include <AccelStepper.h>//include the Accel Stepper Library 
#define MAX_ARRAY_LENGTH 32

// Define two steppers and the pins they will use
AccelStepper stepperX(1, 4, 5);//X axis
AccelStepper stepperY(1, 6, 7);//Y axis
//(driver=true, step, dir)

int enableX = 10;//pins to controll the enable pins on each of the Big Easy Drivers
int enableY = 11;//I tried using the intenral setEnablePin() function in the AccelStepper library
//but couldn't get it to work, even when I inverted them with setPinsInverted().

int posX = 0;//Varibales to keep track of the planchet's current position 
int posY = 0;

int switch_x_left = 8;//Limit switch variables
int switct_y_top = 9;
int switch_x_right = 3;//trying to set these up as interrupts (W.I.P.)
int switch_y_bottom = 2;

int bigButton = 12;//Added big dome pushbutotn to have it display a message when pressed.
//Original plan was to have it run constantly, but the smaller stepper motor (ROB-09238)
//gets hot when ran for even a shoirt while. I'm pushing it past it's max ratings. I would 
//recommend using a 68oz.-in. motor (ROB-10846) on both axes. Make sure you disable all motors 
//when not in use to minimize heat dissipation and power consumption. 

long randomNumber;//Random number for displaying random message when butotn is pressed.

//table to store coordinates for all the letters, #s, etc on the board. 
//Due to the nature of the magnets and drag, getting exact coordinates took 
//lots of trial an error, and is still not perfect when you have the planchet 
//move in from different sides of the letter. 
unsigned int coordinatesTable[39][3] = {
  1000,2600,'a',
  2800,1900,'b',
  4400,1700,'c',
  6100,1500,'d',
  8200,1300,'e',
  9300,1100,'f',
  11800,1200,'g',
  13700,1200,'h',
  15500,1100,'i',
  16800,1500,'j',
  18700,1700,'k',
  20300,2100,'l',
  22800,2500,'m',
  1200,4500,'n',
  2500,3900,'o',
  4300,3400,'p',
  6000,3200,'q',
  7900,2800,'r',
  9700,2600,'s',
  11500,2600,'t',
  13600,2700,'u',
  15400,2700,'v',
  17200,2800,'w',
  19400,3400,'x',
  20800,4000,'y',
  22800,4500,'z',
  0,0,'0',
  0,0,'1',
  0,0,'2',
  0,0,'3',
  0,0,'4',
  0,0,'5',
  0,0,'6',
  0,0,'8',
  0,0,'9',
  4400,10,'+',//YES
  19900,10,'-',//NO
  0,0,'!',//GOODBYE
  11700,100,'.'//SPACE
};
//------------------------------------------------------------------------------
void setup()
{  
  Serial.begin(115200);//Serial print for debug
  
  pinMode(bigButton, INPUT_PULLUP);//Pullup button so external resistor is not needed
  randomSeed(analogRead(0));//Read analog(0) for random number generation 
  
  pinMode(enableX, OUTPUT);//
  pinMode(enableY, OUTPUT);
  
  digitalWrite(enableX, HIGH);//Diable the motors 
  digitalWrite(enableY, HIGH);
  
  //Had to play with these vallues to find a speed and acceleration that looked good 
  //but also still ran smoothly. Tweeking these values and physically changing the 
  //potentiometers on the Big Easy Driver will affect the speed of each axis. 
  //BEWARE, changing these settings will change coordinates and mess up any characters 
  //you have mapped out already. 
  stepperX.setMaxSpeed(6000);
  stepperX.setAcceleration(5000);
  stepperY.setMaxSpeed(4000);
  stepperY.setAcceleration(4000);
  
  //This is the internal AccelStepper enable calls referred to 
  //earlier. Must be called within setup() (W.I.P.)
  //stepperX.setEnablePin(10);
  //stepperY.setEnablePin(11);
  //stepperX.setPinsInverted(0,0,1);//invert the enable pin
  //stepperY.setPinsInverted(0,0,1);
  
  pinMode(switch_x_left, INPUT_PULLUP);
  pinMode(switct_y_top, INPUT_PULLUP);
  pinMode(switch_x_right, INPUT_PULLUP);
  pinMode(switch_y_bottom, INPUT_PULLUP);
  //Switch interrupts to ensure the planchet never goes out of bounds (W.I.P.)
  //attachInterrupt(0, homeing2, FALLING); //switct_y_bottom
  //attachInterrupt(1, homeing2, FALLING);//switch_x_right
  
  homeing();//set the cursor back to its home position at 0,0 first and on every reset

}
//------------------------------------------------------------------------------
void loop()
{
  if(digitalRead(bigButton) == LOW)//Wait until the big dome pusbutton is pressed to start spelling a message
  {
    ouijaMessage();//Select a random message from the list and print it
  }

  //Uncomment this line and comment out everything else in loop() to jog the planchet with keyboard inputs 
  //moveWithKeyboard();
}
//------------------------------------------------------------------------------
void enableMotors()
{
  //Enable the motors 
  digitalWrite(enableX, LOW);
  digitalWrite(enableY, LOW);
}
//------------------------------------------------------------------------------
void disableMotors()
{
  //Disable the motors 
  digitalWrite(enableX, HIGH);
  digitalWrite(enableY, HIGH);
}
//------------------------------------------------------------------------------
void homeing()
{
  enableMotors();
  //Decided to move Y axis first since one of the magnets was hitting if it was all the way away from home. 
  if(digitalRead(switct_y_top) == HIGH)
  {
    while(digitalRead(switct_y_top) == HIGH)
    {
      stepperY.moveTo(posY);
      stepperY.run();
      posY = posY - 1;
      //Serial.println(posY);
    }
    if(digitalRead(switct_y_top) == LOW)
      stepperY.stop();
      posY = 0;
      stepperY.setCurrentPosition(0);
    }
  else
  {
  posY = 0;
  stepperY.setCurrentPosition(0);
  }
  
  if(digitalRead(switch_x_left) == HIGH)//check to make sure this axis isn;t already in the home posotion 
  {
    while(digitalRead(switch_x_left) == HIGH)//if it is not at home (HIGH) ...
    {
      stepperX.moveTo(posX);//start moving the X axis back to the home position
      stepperX.run();
      posX = posX - 1;//decrement the position until we reach 0 (where the switch is triggered) 
    }
    if(digitalRead(switch_x_left) == LOW)//Once the switch is hit and the signal goes LOW...
      stepperX.stop();//stop the motor
      posX = 0;//set this axis position to 0
      stepperX.setCurrentPosition(0);
  }
  else
  {
  posX = 0;
  stepperX.setCurrentPosition(0);
  }
  disableMotors();
}
//------------------------------------------------------------------------------
void ouijaDelay(void){
  //Delay to make it spoooooooky
  //Standard Delay until we can make it more spooky
   delay(200); 
}
//------------------------------------------------------------------------------
//Look through the table until a character matching the current one is found
int findSymbolInTable(char symbol){
  int tableIndex;
  
  for(tableIndex = 0; tableIndex <= 39; tableIndex++){
    //Serial.println(char((coordinatesTable[tableIndex][2])));
    if(symbol == char(coordinatesTable[tableIndex][2])){
      //Serial.print("Char FOUND = ");
      //Serial.println(tableIndex);
      return tableIndex; //Return the table location
    }
  }
     return -1; //if we got to the point
                // we didn't find our value in the table 
}

//------------------------------------------------------------------------------
//Used to move the planchet to the next location on the baord. 
void moveToSymbol(char character)
{
  int tableIndex = findSymbolInTable(character);

  //Serial.print("x = ");
  //Serial.println(coordinatesTable[tableIndex][0]);

  //Serial.print("y = ");
  //Serial.println(coordinatesTable[tableIndex][1]);
  
  //MoveMotors
  stepperX.moveTo(coordinatesTable[tableIndex][0]);
  stepperY.moveTo(coordinatesTable[tableIndex][1]);
  
  while((stepperX.distanceToGo() != 0)||(stepperX.distanceToGo() != 0)){
    //Serial.println(stepperX.distanceToGo());
    stepperY.run();
    stepperX.run();
    //if(digitalRead(switch_x_right) == LOW || digitalRead(switch_y_bottom) == LOW)
      //homeing();
  }

  return;
}


//------------------------------------------------------------------------------
void ouijaPrint(char* charArray){
  int i;
  for(i = 0; i <= MAX_ARRAY_LENGTH; i++){
    //Serial.print(charArray[i]); //Debug the array as we move
    if(charArray[i] != 0){
      ouijaDelay();
      moveToSymbol(charArray[i]);
      ouijaDelay();  
      //if(digitalRead(switch_x_right) == LOW || digitalRead(switch_y_bottom) == LOW)
      //homeing();
    } //Only move if there if valid data, if not let the loop run out.
    else break;
  }
  return;
}
//------------------------------------------------------------------------------
//Stores different messages to be speled out by the Ouija 
void ouijaMessage()
{
    enableMotors();
    randomNumber = random(5);//Generate a random number between 1-5
    switch (randomNumber) //switch messages based on that random number
    {
      case 0:
        {char arrayThing[30] = {'h','e','l','.','l','o',0};
        ouijaPrint(arrayThing);
        homeing();}//Go back to home after each massage is printed
        break;
      case 1:
        {
        char arrayThing2[30] = {'s','p','a','r','k','f','u','n',0};
        ouijaPrint(arrayThing2);
        homeing();}
        break;
      case 2:
        {
        char arrayThing3[30] = {'h','a','p','.','p','y','.','h','a','l','.','l','o','w','e','.','e','n',0};;
        ouijaPrint(arrayThing3);
      homeing();}
        break;
      case 3:
        {
        char arrayThing4[30] = {'g','r','a','n','d','m','a','.','s','a','y','s','.','h','i',0};;
        ouijaPrint(arrayThing4);
        homeing();}
        break;
      case 4:
        {
        char arrayThing5[30] = {'l','e','t','.','y','o','u','r','.','g','e','e','k','.','s','h','i','n','e',0};;
        ouijaPrint(arrayThing5);
      homeing();}
        break;
      default: 
        disableMotors();
     }
    disableMotors();
}
//------------------------------------------------------------------------------
//This function was used to jog each axis 100 "steps" in each direction 
//call in loop and comment out everything else to allow keyboard control
void moveWithKeyboard()
{
  enableMotors();
  if(Serial.available() > 0)
  {
    int temp = Serial.read();
    
    if(temp == 119 || temp == 87)//dec for 'w'
    {
      stepperY.moveTo(stepperY.currentPosition() - 100);
      //stepperY.setSpeed(2000);
      while(stepperY.distanceToGo() != 0)
      {
        stepperY.run();
      }
      Serial.print(stepperX.currentPosition());
      Serial.print("   ");
      Serial.println(stepperY.currentPosition());
    }
    if(temp == 115 || temp == 83)//dec for 's'
    {
      stepperY.moveTo(stepperY.currentPosition() + 100);
      //stepperY.setSpeed(2000);
      while(stepperY.distanceToGo() != 0)
      {
        stepperY.run();
      } 
      Serial.print(stepperX.currentPosition());
      Serial.print("   ");
      Serial.println(stepperY.currentPosition());
    }
    if(temp == 97 || temp == 65)//dec for 'a'
    {
      stepperX.moveTo(stepperX.currentPosition() - 100);
      //stepperX.setSpeed(3000);
      while(stepperX.distanceToGo() != 0)
      {
        stepperX.run();
      }  
      Serial.print(stepperX.currentPosition());
      Serial.print("   ");
      Serial.println(stepperY.currentPosition());
    }
    if(temp == 100 || temp == 68)//dec for 'd'
    {
      stepperX.moveTo(stepperX.currentPosition() + 100.00);
      //stepperX.setSpeed(3000);
      while(stepperX.distanceToGo() != 0)
      {
        stepperX.run();
      }
      Serial.print(stepperX.currentPosition());
      Serial.print("   ");
      Serial.println(stepperY.currentPosition());
    }
    
    //move in larger increments with the right side of the keyboard
    if(temp == 105 || temp == 73)//dec for 'i'
    {
      stepperY.moveTo(stepperY.currentPosition() - 1000);
      while(stepperY.distanceToGo() != 0)
      {
        stepperY.run();
      }
      Serial.print(stepperX.currentPosition()); Serial.print("   "); Serial.println(stepperY.currentPosition());
    }
    if(temp == 107 || temp == 75)//dec for 'k'
    {
      stepperY.moveTo(stepperY.currentPosition() + 1000);
      //stepperY.setSpeed(2000);
      while(stepperY.distanceToGo() != 0)
      {
        stepperY.run();
      } 
      Serial.print(stepperX.currentPosition()); Serial.print("   "); Serial.println(stepperY.currentPosition());
    }
    if(temp == 106 || temp == 74)//dec for 'j'
    {
      stepperX.moveTo(stepperX.currentPosition() - 2000);
      //stepperX.setSpeed(3000);
      while(stepperX.distanceToGo() != 0)
      {
        stepperX.run();
      }  
      Serial.print(stepperX.currentPosition()); Serial.print("   "); Serial.println(stepperY.currentPosition());
    }
    if(temp == 108 || temp == 76)//dec for 'l'
    {
      stepperX.moveTo(stepperX.currentPosition() + 2000);
      while(stepperX.distanceToGo() != 0)
      {
        stepperX.run();
      }
      Serial.print(stepperX.currentPosition()); Serial.print("   "); Serial.println(stepperY.currentPosition());
    }
  
  }
  disableMotors();
}
//------------------------------------------------------------------------------
void drawCircleCW()//(W.I.P.)
{
  enableMotors();
  stepperX.moveTo(12000);
  stepperY.moveTo(1000);
  while((stepperX.distanceToGo() != 0)||(stepperX.distanceToGo() != 0))
  {
    stepperX.run();
    stepperY.run();
  }
  
  stepperX.moveTo(14000);
  stepperY.moveTo(2000);
  while((stepperX.distanceToGo() != 0)||(stepperX.distanceToGo() != 0))
  {
    stepperX.run();
    stepperY.run();
  }
  
  stepperX.moveTo(12000);
  stepperY.moveTo(3000);
  while((stepperX.distanceToGo() != 0)||(stepperX.distanceToGo() != 0))
  {
    stepperX.run();
    stepperY.run();
  }
  
  stepperX.moveTo(8000);
  stepperY.moveTo(2000);
  while((stepperX.distanceToGo() != 0)||(stepperX.distanceToGo() != 0))
  {
    stepperX.run();
    stepperY.run();
  }
  
  stepperX.moveTo(12000);
  stepperY.moveTo(2000);
  while((stepperX.distanceToGo() != 0)||(stepperX.distanceToGo() != 0))
  {
    stepperX.run();
    stepperY.run();
  }
  disableMotors();
}





