#include "mbed.h"
#include "threeAxis.h"
#include "rtos.h"


threeAxis::threeAxis(PinName xStep, PinName yStep, PinName zStep, PinName xDir, PinName yDir, PinName zDir, PinName xEnable, PinName yEnable, PinName zEnable, float stepRatio, float xBound, float yBound, float zBound, PinName xLimit, PinName yLimit, PinName zLimit)
: _xStep(xStep), _yStep(yStep), _zStep(zStep), _xDir(xDir), _yDir(yDir), _zDir(zDir), _xEnable(xEnable), _yEnable(yEnable), _zEnable(zEnable), _xLimit(xLimit), _yLimit(yLimit), _zLimit(zLimit)
{
    //Assigning variables
    _stepRatio = stepRatio;
    _xMax = xBound;
    _yMax = yBound;
    _zMax = zBound;

    
    //Enable pins are active low, so drive them high to keep them off for now
    _xEnable = 1;
    _yEnable = 1;
    _zEnable = 1;
    
    //Set default wait to 5
    wait = 5;
    
    //Set default directions
    defaultXdir = 0;
    defaultYdir = 0;
    defaultZdir = 0;
    
    //Assign default directions to current directions
    _xDir = defaultXdir;
    _yDir = defaultYdir;
    _zDir = defaultZdir;
    
    //Disable limits, set buffer index to 0
    limitsEnabled = false;
    bufferIndex = 0;
}

void threeAxis::addToBuffer(float xVal, float yVal, float zVal){
    
    //Add value to buffer
    buffer[bufferIndex][0] = xVal;
    buffer[bufferIndex][1] = yVal;
    buffer[bufferIndex][2] = zVal;
    
    //Index buffer by 1
    bufferIndex = bufferIndex + 1;
}

void threeAxis::runBuffer(){
    //Run from first buffer entry to last
    int i = 0;
    while(i < bufferIndex){
        goTo(buffer[i][0], buffer[i][1], buffer[i][2]);
        i = i+1;
    }
    
    bufferIndex = 0;
}

void threeAxis::invertX(){
    defaultXdir = !defaultXdir;
    _xDir = defaultXdir;
}

void threeAxis::invertY(){
    defaultYdir = !defaultYdir;
    _yDir = defaultYdir;
}

void threeAxis::invertZ(){
    defaultZdir = !defaultZdir;
    _zDir = defaultZdir;
}

void threeAxis::setZeroX(){
    toX = 0;
    currX = 0;
}

void threeAxis::setZeroY(){
    toY = 0;
    currY = 0;
}

void threeAxis::setZeroZ(){
    toZ = 0;
    currZ = 0;
}

void threeAxis::setZero(){
    this->setZeroX();
    this->setZeroY();
    this->setZeroZ();
}

void threeAxis::setWait(int waitPer){
    wait = waitPer;
}

void threeAxis::setXdir(bool val){
    defaultXdir = val;
    _xDir = val;
}

void threeAxis::setYdir(bool val){
    defaultYdir = val;
    _yDir = val;
}

void threeAxis::setZdir(bool val){
    defaultZdir = val;
    _zDir = val;
}

//Zeroing for X axis. Enable driver, set direction to negative, and step one at a time until limit switch tripped
//Then define that position as 0 (not actually necessary, no positional updates actually occuring while zeroing)
//Then disable driver
void threeAxis::zeroX(){
    _xEnable = 0;
    _xDir = !defaultXdir;
    
    while(!_xLimit){
        _xStep = 1;
        Thread::wait(wait);
        _xStep = 0;
        Thread::wait(wait);
    }
    
    this->setZeroX();
    _xEnable = 1;

}


//Zeroing for Y axis. Enable driver, set direction to negative, and step one at a time until limit switch tripped
//Then define that position as 0 (not actually necessary, no positional updates actually occuring while zeroing)
//Then disable driver

void threeAxis::zeroY(){
    _yEnable = 0;
    _yDir = !defaultYdir;
    
    while(!_yLimit){
        _yStep = 1;
        Thread::wait(wait);
        _yStep = 0;
        Thread::wait(wait);
    }
    
    this->setZeroY();
    _yEnable = 1;

}


//Zeroing for Z axis. Enable driver, set direction to negative, and step one at a time until limit switch tripped
//Then define that position as 0 (not actually necessary, no positional updates actually occuring while zeroing)
//Then disable driver

void threeAxis::zeroZ(){
    _zEnable = 0;
    _zDir = !defaultZdir;
    
    while(!_zLimit){
        _zStep = 1;
        Thread::wait(wait);
        _zStep = 0;
        Thread::wait(wait);
    }
    
    this->setZeroX();
    _zEnable = 1;

}

void threeAxis::setLimits(bool val){
    limitsEnabled = val;
}

bool threeAxis::getLimitsEn(){
    return limitsEnabled;
}

/*void threeAxis::goTo(float xVal, float yVal, float zVal){
    int xSteps = (int)xVal*_stepRatio;
    int ySteps = (int)yVal*_stepRatio;
    int zSteps = (int)zVal*_stepRatio;
    
    this->goToRaw(xSteps, ySteps, zSteps);
}*/


void threeAxis::goTo(float xVal, float yVal, float zVal){
    
    //Convert inches to # steps
    int xSteps = (int)(xVal/_stepRatio);
    int ySteps = (int)(yVal/_stepRatio);
    int zSteps = (int)(zVal/_stepRatio);
    
    
    //Assign # steps to desired location
    toX = xSteps;
    toY = ySteps;
    toZ = zSteps;
    
    //Bounds checking if bounds enabled
    if(limitsEnabled){
        if (toX > (int)(_xMax / _stepRatio)){
            toX = (int)(_xMax / _stepRatio);
        }
    
        if (toY > (int)(_yMax / _stepRatio)){
            toY = (int)(_yMax / _stepRatio);
        }
    
        if (toZ > (int)(_zMax / _stepRatio)){
            toZ = (int)(_zMax / _stepRatio);
        }
        
        if (toX<0){
            toX = 0;
        }
        
        if (toY<0){
            toY = 0;
        }
        
        if (toZ<0){
            toZ = 0;
        }
    }    
    
    //Define default difference between Step N and Step N-1 as potiive
    int xStepVal = 1;
    int yStepVal = 1;
    int zStepVal = 1;
    
    //Assign directions to positive by default
    _xDir = defaultXdir;
    _yDir = defaultYdir;
    _zDir = defaultZdir;
    
    //Calculate differrence between desired location and current location
    int x = toX - currX;
    int y = toY - currY;
    int z = toZ - currZ;
    

        
        
    //If any difference on any axis is not 0, begin setting up that axis' driver
    if (x != 0 || y != 0 || z != 0){
         
         //Enable X driver if difference between desired X and current X is greater than abs(0)
         if (x != 0){
             _xEnable = 0;
             
             //Check for negative difference (e.g. desired X < current X)
             if (x < 0){
                 //Invert X direction
                 _xDir = !defaultXdir;
                 //Invert x, so that the difference is positive
                 x = x* -1;
                 //Invert xStepVal, as this is what will be added to the value for current X location every step
                 xStepVal = xStepVal * -1;
             }
         }
        
         //Enable Y driver if difference between desired Y and current Y is greater than abs(0)
         if (y != 0){
             _yEnable = 0;
             
             //Check for negative difference (e.g. desired Y < current Y)
             if (y < 0){
                 //Invert Y direction
                 _yDir = !defaultYdir;
                 //Invert y, so that the difference is positive
                 y = y* -1;
                 //Invert yStepVal, as this is what will be added to the value for current Y location every step
                 yStepVal = yStepVal * -1;
             }
         }
         
         //Enable Z driver if difference between desired Z and current Z is greater than abs(0)
         if (z != 0){
             _zEnable = 0;
             
             //Check for negative difference (e.g. desired Z < current Z)  
             if (z < 0){
                 //Invert Z direction
                 _zDir = !defaultZdir;
                 //Invert z, so that the difference is positive
                 z = z* -1;
                 //Invert zStepVal, as this is what will be added to the value for current Z location every step
                 zStepVal = zStepVal * -1;
             }
         }
         
         //While any of the differences are positive, begin moving
         while (x > 0 || y > 0 || z > 0){
             
             //Check X difference, send step command, increment current location, and decrement difference
             if (x>0){
                 _xStep = 1;
                 x=x-1;
                 currX += xStepVal;
             }
             
             //Check Y difference, send step command, increment current location, and decrement difference
             if (y>0){
                 _yStep = 1;
                 y=y-1;
                 currY += yStepVal;
             }
             
             //Check Z difference, send step command, increment current location, and decrement difference
             if (z>0){
                 _zStep = 1;
                 z=z-1;
                 currZ += zStepVal;
             }
             
             //Wait for the global wait time
             Thread::wait(wait);
             
             //Set all step signals to low.
             _xStep = 0;
             _yStep = 0;
             _zStep = 0;
             
             //Wait for the global wait time before repeating
             Thread::wait(wait);
         }
     }
     
     //Disable all driver boards, set direction to positive
     _xEnable = 1;
     _yEnable = 1;
     _zEnable = 1;
     _xDir = defaultXdir;
     _yDir = defaultYdir;
     _zDir = defaultZdir;
     
}   
