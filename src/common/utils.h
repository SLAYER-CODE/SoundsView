#ifndef UTILS_H
#define UTILS_H
#include <QFontMetrics>
#include <QPainterPath>
#include <QPolygonF>

class Utils {
public:
  Utils();
  static QRectF CalculateRec(const QPolygonF &triangle);
  static QString getLongestWord(const QString &text);
  static QPainterPath roundedPolygonPath(const QPolygonF &poly, qreal radius);
};

#endif // UTILS_H
