#ifndef RADIO_H
#define RADIO_H

#include "Navio/RCInput.h"


class Radio {
public:
    Radio();

    float ReadThrottle();
    float ReadSteer();

private:
    RCInput rc;
};

#endif // RADIO_H
