#ifndef STATE_H
#define STATE_H

class StateofMotor
{
    private:
        //int mode;
        int button1;
        int button2;
        int button3;
        long speed;
        bool direction;

    public:

    StateofMotor(int button1, int button2, int button3,long speed,bool direction);
    //void setMode(int mode);
    void setButton1(int button1);
    void setButton2(int button2);
    void setButton3(int button3);
    void setSpeed(long speed);
    void setDirection(bool direction);
    //void getMode();
    int getButton1();
    int getButton2();
    int getButton3();
    int getSpeed();
    int getDirection();
    //int CheckMode();
    int CheckButton1();
    int CheckButton2();
    int CheckButton3();
    long CheckSpeed();
    int returnDirection();


};


#endif 