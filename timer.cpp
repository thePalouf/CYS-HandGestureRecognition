#include "timer.h"
#include <iostream>

//start the timer
void Timer::start() {
    m_startTimer = clock();
}

//return elapsed time in milliseconds
unsigned long Timer::elapsedTime() {
    return ((unsigned long) clock() - m_startTimer) / (CLOCKS_PER_SEC/1000);
}

//check if elapsed time is greater than milliseconds given
bool Timer::isTimeout(unsigned long milliseconds) {
    return milliseconds < elapsedTime();
}

