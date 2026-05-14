#include "utils.h"
#include <QFontMetrics>
#include <QPolygon>
#include <QString>
#include <QStringList>

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
