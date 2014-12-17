#pragma once

#include "GL/freeglut.h"

void GetWorldCoordinate( float &_posX, float &_posY, int _screenX, int _screenY );
int round_value( float _value, float _start, float _step );
