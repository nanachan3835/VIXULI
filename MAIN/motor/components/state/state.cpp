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

StateofMotor::setButton1(int button1)
{
    this->button1 = button1;
}

StateofMotor::setButton2(int button2)
{
    this->button2 = button2;
}

StateofMotor::setButton3(int button3)
{
    this->button3 = button3;
}


StateofMotor::setSpeed(long speed)
{
    this->speed = speed;
}

StateofMotor::setDirection(bool direction)
{
    this->direction = direction;
}

// StateofMotor::getMode()
// {
//     return mode;
// }

StateofMotor::getButton1()
{
    return button1;
}

StateofMotor::getButton2()
{
    return button2;
}

StateofMotor::getButton3()
{
    return button3;
}

StateofMotor::getSpeed()
{
    return speed;
}

StateofMotor::getDirection()
{
    return direction;
}





// State::CheckMode()
// {
//     return mode;
// }



State::CheckButton1()
{
    return button1;
}
State::CheckButton2()
{
    return button2;
}
State::CheckButton3()
{
    return button3;
}
State::CheckSpeed()
{
    return speed;
}
State::returnDirection()
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

