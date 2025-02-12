// buttondata.cpp
#include "structdata.h"

ButtonData::ButtonData()
    : button(nullptr)
    , startAngle(0.0)
    , endAngle(0.0)
{}

ButtonData::ButtonData(PolygonButton *btn, double start, double end)
    : button(btn)
    , startAngle(start)
    , endAngle(end)
{}

ButtonDataMenu::ButtonDataMenu()
    : button(nullptr)
    , startAngle(0.0)
    , endAngle(0.0)
{}

ButtonDataMenu::ButtonDataMenu(CircularTriangleButton *btn, double start, double end)
    : button(btn)
    , startAngle(start)
    , endAngle(end)
{}
