#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "string"
#include "threeAxis.h"

//Enable is active low.

//xStep, yStep, zStep, xDir, yDir, zDir, xEnable, yEnable, zEnable, stepRatio, xBound, yBound, zBound, xLimit, yLimit, zLimit
threeAxis table(p23, p25, p21, p24, p26, p22, p19, p18, p20, 0.0007925725, 11.5, 6.75, 1.75, p12, p14, p16);

//"Limit switches"
DigitalOut xLimit(p13);
DigitalOut yLimit(p15);
DigitalOut zLimit(p17);



//Global variables for main
Serial bluetooth(p28,p27);
uLCD_4DGL uLCD(p9,p10,p11);
DigitalOut actualLED(LED1);
Mutex LCD;
bool buffering;
Thread thread;
Thread thread2;

//Method to trip a "limit switch" if a signal is received over Bluetooth. 
//Used in a seperate thread while zero'ing methods are running
void raiseLimit(DigitalOut* arg){
        while(1){
            if(bluetooth.readable()){
                char temp;
                while(bluetooth.readable()){
                    temp = bluetooth.getc();
                }
                *arg = 1;
                break;
            }
        }
}

//Flushes the Bluetooth buffer
void bluetoothFlush(){
    char temp;
    while(bluetooth.readable()){
        temp = bluetooth.getc();
    }
}

//Takes command string, parses coordinate values, then determines whether to pass them in buffered mode or direct mode
void parseCoords(std::string coords){    
    
    int start = 0;
    int end = coords.find(',');
    

    float tempX = atof(coords.substr(start, end-start).c_str());

    
    start = end + 1;
    end = coords.find(',', start);

    float tempY = atof(coords.substr(start, end-start).c_str());
    
    start = end + 1;
    end = coords.length();
    float tempZ = atof(coords.substr(start, end-start).c_str());
    
    if(!buffering){
        table.goTo(tempX, tempY, tempZ);
    }
    else{
        table.addToBuffer(tempX, tempY, tempZ);
    }
    
}

//Thread to handle Bluetooth communication
void bluetooth_thread(){
    
    while(1) {
        std::string coords = "";
        Thread::wait(100);
        
        if (bluetooth.readable()){
            LCD.lock();
            uLCD.cls();
            LCD.unlock();
            while (bluetooth.readable()){
                
                char temp = bluetooth.getc();
                
                //z, b, and e are special command characters. z zeros the machine, b enables/disables buffering, and e enables/disables limtis
                if (temp == 'z'){
                    table.setZero();
                }
                else if(temp == 'b'){
                    buffering = !buffering;
                }
                else if(temp == 'e'){
                    table.setLimits(!table.getLimitsEn());
                }
                
                coords += temp;
            }
        
        //Prints LCD output depending on what was inputted.
        LCD.lock();
        
        //Prints coordinates
        if ((coords.find('z') == -1) && (coords.find('b') == -1) && (coords.find('e') == -1)){                
             uLCD.printf(coords.c_str());
             LCD.unlock();
             parseCoords(coords);
                
        }
        
        //Prints messages for special command characters. Also runs the buffer if buffering is ended.
        else{
                
             if (coords.find('z') != -1){
                 uLCD.printf("Zeroed!");
                 LCD.unlock();
             }
             else if (coords.find('b') != -1){
                 if (buffering){
                     uLCD.printf("Now buffering!");
                     LCD.unlock();
                 }
                 else if (!buffering){
                     uLCD.printf("Running buffer!");
                     LCD.unlock();
                     table.runBuffer();
                 }
             }
             else if (coords.find('e') != -1){
                 if (table.getLimitsEn()){
                     uLCD.printf("Limits enabled!");
                     LCD.unlock();
                 }
                 else if (!table.getLimitsEn()){
                     uLCD.printf("Limits disabled!");
                     LCD.unlock();
                 }
             }
        }
    }
        
        
}

}


int main() {
    //Y and Z axis direction inverted from X axis to get behavior that makes me happy (rewards = negative, forward = positive)
    table.invertY();
    table.invertZ();
    table.setWait(3);
    uLCD.baudrate(300000);
    wait(0.5);
    
    
    //Begin zeroing
    LCD.lock();
    uLCD.printf("Enter null value to begin X zero, anything else to skip");
    //Wait for Bluetooth input
    while(1){
        if(bluetooth.readable()){
            bluetoothFlush();
            break;
        }
    }

    uLCD.printf("\n");
    uLCD.printf("Enter null to stop");
    LCD.unlock();
    
    //Start LimitSwitch thread
    thread2.start(raiseLimit,&xLimit);
    Thread::wait(50);
    //Begin zeroing function
    table.zeroX();

    LCD.lock();
    uLCD.cls();    
    uLCD.printf("Enter null value to begin Y zero, anything else to skip");
    LCD.unlock();
    
    //Repeat process for Y and Z
    while(1){
        if(bluetooth.readable()){
            bluetoothFlush();
            break;
        }
    }
    
    LCD.lock();
    uLCD.printf("\n");
    uLCD.printf("Enter null to stop");
    LCD.unlock();
    
    thread2.start(raiseLimit,&yLimit);
    Thread::wait(50);
    table.zeroY();
    
    LCD.lock();
    uLCD.cls();    
    uLCD.printf("Enter null value to begin Z zero, anything else to skip");
    LCD.unlock();
    
    while(1){
        if(bluetooth.readable()){
            bluetoothFlush();
            break;
        }
    }
    
    LCD.lock();
    uLCD.printf("\n");
    uLCD.printf("Enter null to stop");
    LCD.unlock();
    
    
    thread2.start(raiseLimit,&zLimit);
    Thread::wait(50);
    table.zeroZ();
    
    //Ensure thread2 is killed
    thread2.terminate();
    LCD.lock();
    uLCD.cls();
    
    //Enable bounds checking, make sure the "limit switches" are set to low again
    table.setLimits(true);
    xLimit = 0;
    yLimit = 0;
    zLimit = 0;
        
    uLCD.printf("Waiting...");
    LCD.unlock();
    
    bluetoothFlush();
    
    //Begin Bluetooth thread
    thread.start(bluetooth_thread);
    
    //This probably can be removed to free up a thread
    while(1);
}
