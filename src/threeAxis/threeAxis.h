class threeAxis{
    public:
        
        //Constructor
        threeAxis(PinName xStep, PinName yStep, PinName zStep, PinName xDir, PinName yDir, PinName zDir, PinName xEnable, PinName yEnable, PinName zEnable, float stepRatio, float xBound, float yBound, float zBound, PinName xLimit, PinName yLimit, PinName zLimit);
        
        //Accepts arguments in inches
        void goTo(float xVal, float yVal, float zVal);
        
        //Adds values to a buffer, to then be run when runBuffer is executed.
        void addToBuffer(float xVal, float yVal, float zVal);
        void runBuffer();
        
        //Sets current location to 0,0,0
        void setZero();
        
        //Sets current location on an axis to 0 for that axis
        void setZeroX();
        void setZeroY();
        void setZeroZ();
        
        //Reverses motion on a given axis until the limit switch is triggered
        void zeroX();
        void zeroY();
        void zeroZ();
        
        //Sets 1/2 the period of the step signal. Step signal held high for 1*waitPer ms, then low for 1*waitPer ms. Double check that values <3 do not cause skipped steps.
        //3 for "fastest", 5 for "slow", 10 for "molasses" 
        void setWait(int waitPer);
        
        //Sets positive/negative direction for the given axis, based upon argument value. 
        void setXdir(bool val);
        void setYdir(bool val);
        void setZdir(bool val);
        
        //Inverts the current direction on a given axis
        void invertX();
        void invertY();
        void invertZ();
        
        //Enables or disables limit enforcement, based on the argument
        void setLimits(bool val);
        
        //Returns the current state of limit enforcing
        bool getLimitsEn();
        
    protected:
    
        //Desired location
        int toX;
        int toY;
        int toZ;
        
        //Current location
        int currX;
        int currY;
        int currZ;
        
        //Step signals
        DigitalOut _xStep;
        DigitalOut _yStep;
        DigitalOut _zStep;
        
        //Direction signals
        DigitalOut _xDir;
        DigitalOut _yDir;
        DigitalOut _zDir;
        
        //Enable signals. Active low
        DigitalOut _xEnable;
        DigitalOut _yEnable;
        DigitalOut _zEnable;
        
        //Limit switch inputs
        DigitalIn _xLimit;
        DigitalIn _yLimit;
        DigitalIn _zLimit;
        
        //Global step ratio. inches/step
        float _stepRatio;
        
        //Maximum distance that can be traveled in a given axis
        float _xMax;
        float _yMax;
        float _zMax;
        
        bool limitsEnabled;
        
        bool defaultXdir;
        bool defaultYdir;
        bool defaultZdir;
        
        int wait;
        
        int bufferIndex;
        float buffer[][3];
};