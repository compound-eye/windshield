#ifndef _RGBLED_H_
#define _RGBLED_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"

namespace Colors {
  enum T {
    Black,
    Red,
    Green,
    Blue,
    Cyan,
    Magenta,
    Yellow,
    White
  };
}

class RGBled {
public:
    RGBled();

    bool initialize();
    void setColor(Colors::T color);

private:
    Navio::Pin *pinR;
    Navio::Pin *pinG;
    Navio::Pin *pinB;
};

#endif //_RGBLED_H_
