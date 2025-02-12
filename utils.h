#ifndef UTILS_H
#define UTILS_H
#include <QFontMetrics>

class Utils
{
public:
    Utils();
    static QRectF CalculateRec(const QPolygonF &triangle);
    static QString getLongestWord(const QString &text);
};

#endif // UTILS_H
