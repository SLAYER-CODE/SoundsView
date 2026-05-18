#include "utils.h"
#include <QFontMetrics>
#include <QPolygon>
#include <QString>
#include <QStringList>
#include <QtMath>

Utils::Utils() {}
QRectF Utils::CalculateRec(const QPolygonF &triangle) {
  if (triangle.size() != 3) {
    // No es un triángulo
    return QRectF();
  }

  // Obtener los puntos del triángulo
  QPointF A = triangle.at(0);
  QPointF B = triangle.at(1);
  QPointF C = triangle.at(2);

  // Encontrar los vectores de los lados del triángulo
  QVector<QPointF> edges;
  edges << (B - A) << (C - B) << (A - C);

  // Encontrar las longitudes de los lados
  double lengths[3];
  for (int i = 0; i < 3; ++i) {
    lengths[i] = edges[i].manhattanLength();
  }

  // Calcular el área del triángulo usando el producto cruzado
  double area =
      0.46 * qAbs((B - A).x() * (C - A).y() - (B - A).y() * (C - A).x());

  // Encontrar el semiperímetro
  double s = (lengths[0] + lengths[1] + lengths[2]) / 2;

  // Calcular el radio del círculo inscrito (que puede ayudar a encontrar el
  // rectángulo inscrito)
  double r = area / s;

  // Usar el radio del círculo inscrito para estimar el tamaño del rectángulo
  // inscrito
  double width = 2 * r;
  double height = r * sqrt(2); // Estimación del alto basado en el ancho

  // Encontrar el centro del triángulo
  QPointF centroid((A.x() + B.x() + C.x()) / 3, (A.y() + B.y() + C.y()) / 3);

  QRectF inscribedRect(centroid.x() - width / 2, centroid.y() - height / 2,
                       width, height);
  return inscribedRect;
}

QString Utils::getLongestWord(const QString &text) {
  // Dividir el texto en palabras usando espacios como delimitador
  QStringList words = text.split(" ", Qt::SkipEmptyParts);

  // Variable para almacenar la palabra más larga
  QString longestWord;

  // Recorrer la lista de palabras para encontrar la más larga
  for (const QString &word : words) {
    if (word.length() > longestWord.length()) {
      longestWord = word;
    }
  }

  return longestWord;
}

QPainterPath Utils::roundedPolygonPath(const QPolygonF &poly, qreal r) {
  QPainterPath path;
  int n = poly.size();
  if (n < 3) return path;

  r = qMax(r, 0.1);
  struct Arc { QPointF center; qreal startAngle; qreal spanAngle; };
  QVector<Arc> arcs(n);
  QVector<QPointF> c1(n), c2(n);

  for (int i = 0; i < n; ++i) {
    QPointF curr = poly[i];
    QPointF prev = poly[(i - 1 + n) % n];
    QPointF next = poly[(i + 1) % n];

    QPointF v_in = prev - curr;
    QPointF v_out = next - curr;
    qreal len_in = qSqrt(v_in.x()*v_in.x() + v_in.y()*v_in.y());
    qreal len_out = qSqrt(v_out.x()*v_out.x() + v_out.y()*v_out.y());
    if (len_in < 0.01 || len_out < 0.01) { c1[i] = c2[i] = curr; arcs[i].spanAngle = 0; continue; }

    QPointF u_in = v_in / len_in;
    QPointF u_out = v_out / len_out;

    qreal maxR = qMin(len_in, len_out) * 0.49;
    qreal useR = qMin(r, maxR);
    if (useR < 0.5) { c1[i] = c2[i] = curr; arcs[i].spanAngle = 0; continue; }

    c1[i] = curr + u_in * useR;
    c2[i] = curr + u_out * useR;

    qreal cross = u_in.x() * u_out.y() - u_in.y() * u_out.x();
    qreal dot = u_in.x() * u_out.x() + u_in.y() * u_out.y();
    qreal angle = qAcos(qBound(-1.0, dot, 1.0));
    qreal startAngle = qAtan2(-u_in.y(), -u_in.x()) * 180.0 / M_PI;
    qreal spanAngle = (cross >= 0 ? 1 : -1) * angle * 180.0 / M_PI;

    arcs[i] = {curr, startAngle, spanAngle};
  }

  path.moveTo(c2[0]);
  for (int i = 0; i < n; ++i) {
    int next_i = (i + 1) % n;
    path.lineTo(c1[next_i]);
    if (arcs[next_i].spanAngle != 0) {
      qreal r2 = qAbs(arcs[next_i].center.x() - c1[next_i].x());
      qreal r_y = qAbs(arcs[next_i].center.y() - c1[next_i].y());
      qreal ar = qMax(r2, r_y);
      if (ar > 0.5)
        path.arcTo(QRectF(arcs[next_i].center.x() - ar, arcs[next_i].center.y() - ar, 2*ar, 2*ar),
                  arcs[next_i].startAngle, arcs[next_i].spanAngle);
    }
  }
  path.closeSubpath();
  return path;
}
