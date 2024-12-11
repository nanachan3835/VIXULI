#include"Lib.h"





State::State()
{
    this->mode = mode;
    this->button1 = button1;
    this->button2 = button2;
    this->button3 = button3;
    this->speed = speed;
    this->direction = direction;
}
 
State::setMode(int mode)
{
    this->mode = mode;
}

State::setButton1(int button1)
{
    this->button1 = button1;
}

State::setButton2(int button2)
{
    this->button2 = button2;
}

State::setButton3(int button3)
{
    this->button3 = button3;
}


State::setSpeed(long speed)
{
    this->speed = speed;
}

State::setDirection(bool direction)
{
    this->direction = direction;
}

State::getMode()
{
    return mode;
}

State::getButton1()
{
    return button1;
}

State::getButton2()
{
    return button2;
}

State::getButton3()
{
    return button3;
}

State::getSpeed()
{
    return speed;
}

State::getDirection()
{
    return direction;
}





State::CheckMode()
{
    return mode;
}



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

