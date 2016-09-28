#ifndef TIMER_H
#define TIMER_H
#include <ctime>

class Timer {
public:
    void start();
    unsigned long elapsedTime();
    bool isTimeout(unsigned long milliseconds);

private:
    unsigned long m_startTimer;
};

#endif // TIMER_H

