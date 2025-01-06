#include"state.h"





StateofMotor::StateofMotor(int button1, int button2, int button3,long speed,bool direction)
{
    //this->mode = mode;
    this->button1 = button1;
    this->button2 = button2;
    this->button3 = button3;
    this->speed = speed;
    this->direction = direction;
}
 
// StateofMotor::setMode(int mode)
// {
//     this->mode = mode;
// }

void StateofMotor::setButton1(int button1)
{
    this->button1 = button1;
}

void StateofMotor::setButton2(int button2)
{
    this->button2 = button2;
}

void StateofMotor::setButton3(int button3)
{
    this->button3 = button3;
}


void StateofMotor::setSpeed(long speed)
{
    this->speed = speed;
}

void StateofMotor::setDirection(bool direction)
{
    this->direction = direction;
}

// StateofMotor::getMode()
// {
//     return mode;
// }

int StateofMotor::getButton1()
{
    return button1;
}

int StateofMotor::getButton2()
{
    return button2;
}

int StateofMotor::getButton3()
{
    return button3;
}

int StateofMotor::getSpeed()
{
    return speed;
}

int StateofMotor::getDirection()
{
    return direction;
}





// State::CheckMode()
// {
//     return mode;
// }



int StateofMotor::CheckButton1()
{
    return button1;
}
int StateofMotor::CheckButton2()
{
    return button2;
}
int StateofMotor::CheckButton3()
{
    return button3;
}
long StateofMotor::CheckSpeed()
{
    return speed;
}
int StateofMotor::returnDirection()
{
    if (this->direction == true)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

