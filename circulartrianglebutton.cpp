#include "circulartrianglebutton.h"
#include "iconmanager.h"
#include "utils.h"
#include <QEvent>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QtAwesome.h>
#include <QtMath>

CircularTriangleButton::CircularTriangleButton(
    const QString &text, const QChar &icon, const QChar &iconCheck,
    qreal centerX, qreal centerY, qreal radius, qreal angle, qreal nextangle,
    qreal nextanglestep, QWidget *parent)
    : QWidget(parent), m_radius(50), m_centerX(centerX), m_centerY(centerY),
      m_icon(icon), m_iconUnchecked(icon), m_iconchecked(iconCheck),
      m_backgroundColor(QColor(31, 31, 31, 200)),
      m_hoverBackgroundColor(Qt::black), m_borderColor(Qt::white),
      m_hoverBorderColor(Qt::red), m_isHovered(false), m_zoom(0),
      m_iconLabel(new QLabel(this)), // Crear QLabel
      m_iconSize(32)                 // Default icon size
{
  // setCheckable(true);
  // connect(this, &QPushButton::toggled, this,
  // &CircularTriangleButton::updateIcon);

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setMouseTracking(true);
  setAttribute(Qt::WA_Hover);

  m_zoomAnimation = new QPropertyAnimation(this, "zoom");
  m_zoomAnimation->setDuration(300);
  m_zoomAnimation->setStartValue(0);
  m_zoomAnimation->setEndValue(0.5);

  m_radiusAnimation = new QPropertyAnimation(this, "radius");
  m_radiusAnimation->setDuration(300);

  // Configurar QLabel
  m_iconLabel->setAlignment(Qt::AlignCenter);
  m_iconLabel->setVisible(false);
}

void CircularTriangleButton::updateIcon(bool checked) {
  if (!checked) {
    m_icon = m_iconUnchecked;
    m_backgroundColor = QColor(31, 31, 31, 200);
    m_hoverBackgroundColor = Qt::black;
  } else {
    m_backgroundColor = QColor(240, 10, 10);
    m_hoverBackgroundColor = QColor(200, 10, 31);
    m_icon = m_iconchecked;
  }
  update();
}

void CircularTriangleButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  fa::QtAwesome *awesome = IconManager::instance().awesome();
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setBrush(m_backgroundColor);
  painter.drawRect(rect());

  QPen pen(m_borderColor, 3); // Ajusta el grosor del borde aquí
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  if (m_isHovered) {
    painter.setBrush(m_hoverBackgroundColor);
    painter.drawRect(rect());
  }

  // Configurar el font para el ícono de FontAwesome
  QFont font = awesome->font(
      fa::fa_solid, 35 * (1 + m_zoom)); // Establecer el tamaño de fuente 16
  painter.setFont(font);

  // Configurar el ícono que deseas mostrar
  QString iconText = m_icon; // Utilizar el ícono almacenado en la clase

  // Calcular el centro del polígono
  QPointF center;
  for (const QPointF &point : m_polygon) {
    center += point;
  }
  center /= m_polygon.size();

  // Calcular el rectángulo de los límites del ícono
  QFontMetrics fm(font);
  QRectF iconBoundingRect = fm.boundingRect(iconText);

  // Calcular la posición superior izquierda del ícono para centrarlo en el
  // centro del polígono
  QPointF iconTopLeft(center.x() - iconBoundingRect.width() / 2,
                      center.y() - iconBoundingRect.height() / 2);

  // Dibujar el ícono centrado en el polígono
  painter.drawText(QRectF(iconTopLeft, iconBoundingRect.size()),
                   Qt::AlignCenter, iconText);
}

void CircularTriangleButton::focusInEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  m_isHovered = true;
  m_zoomAnimation->setDirection(QPropertyAnimation::Forward);
  m_zoomAnimation->start();

  m_radiusAnimation->stop();
  m_radiusAnimation->setStartValue(m_radius);
  m_radiusAnimation->setEndValue(75); // El nuevo radio al agrandarse
  m_radiusAnimation->start();

  int x = this->x() + this->width() / 2;
  int y = this->y() + this->height() / 2;
  emit hoverEntered(x, y); // Emit the signal with the center position
}

void CircularTriangleButton::focusOutEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  m_isHovered = false;

  m_zoomAnimation->setDirection(QPropertyAnimation::Backward);
  m_zoomAnimation->start();

  m_radiusAnimation->stop();
  m_radiusAnimation->setStartValue(m_radius);
  m_radiusAnimation->setEndValue(50); // El radio original
  m_radiusAnimation->start();
  emit hoverLeave(); // Emit the signal with the center position
}

qreal CircularTriangleButton::zoom() const { return m_zoom; }

void CircularTriangleButton::setzoom(const qreal &zom) {
  m_zoom = zom;
  update();
}

void CircularTriangleButton::setRadius(const qreal radius) {
  m_radius = radius;
  update();
}

qreal CircularTriangleButton::radius() const { return m_radius; }

qreal CircularTriangleButton::centerX() const { return m_centerX; }

void CircularTriangleButton::setCenterX(qreal centerX) {
  m_centerX = centerX;
  update();
}

qreal CircularTriangleButton::centerY() const { return m_centerY; }

void CircularTriangleButton::setCenterY(qreal centerY) {
  m_centerY = centerY;
  update(); // Esto asegura que paintEvent sea llamado
}

QPolygonF CircularTriangleButton::polygon() const { return m_polygon; }

void CircularTriangleButton::setPolygon(const QPolygonF &polygon) {
  m_polygon = polygon;
  QRegion region(m_polygon.toPolygon());
  setMask(region);
  update();
}

QColor CircularTriangleButton::backgroundColor() const {
  return m_backgroundColor;
}

void CircularTriangleButton::setBackgroundColor(const QColor &color) {
  m_backgroundColor = color;
  update();
}

void CircularTriangleButton::setTextColor(const QColor &color) {
  m_textColor = color;
  update();
}
