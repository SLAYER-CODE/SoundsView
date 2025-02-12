// buttondata.h
#ifndef BUTTONDATA_H
#define BUTTONDATA_H

#include "circulartrianglebutton.h"
#include "polygonbutton.h"
struct ButtonData
{
    ButtonData(); // Constructor por defecto
    ButtonData(PolygonButton *button,
               double startAngle,
               double endAngle); // Constructor personalizado

    PolygonButton *button;
    double startAngle;
    double endAngle;
};

struct ButtonDataMenu
{
    ButtonDataMenu();
    ButtonDataMenu(CircularTriangleButton *button, double startAngle, double endAngle);
    CircularTriangleButton *button;
    double startAngle;
    double endAngle;
};

#endif // BUTTONDATA_H
