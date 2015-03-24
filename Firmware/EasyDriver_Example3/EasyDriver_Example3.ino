// Example3 code for Brian Schmalz's Easy Driver Example page
// http://www.schmalzhaus.com/EasyDriver/EasyDriverExamples.html

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper(1, 9, 8);

int pos = 3600;

void setup()
{  
  stepper.setMaxSpeed(3000);
  stepper.setAcceleration(1000);
}

void loop()
{
  if (stepper.distanceToGo() == 0)
  {
    delay(500);
    pos = -pos;
    stepper.moveTo(pos);
  }
  stepper.run();
}
