#include "circulartrianglebutton.h"
#include "iconmanager.h"
#include "utils.h"
#include <QEvent>
#include <QMouseEvent>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QtAwesome.h>
#include <QtMath>

CircularTriangleButton::CircularTriangleButton(
    const QString &text, const QChar &icon, const QChar &iconCheck,
    qreal centerX, qreal centerY, qreal radius, qreal angle, qreal nextangle,
    qreal nextanglestep, QWidget *parent)
    : QWidget(parent), m_text(text), m_radius(50), m_centerX(centerX), m_centerY(centerY),
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
  m_zoomAnimation->setDuration(120);
  m_zoomAnimation->setStartValue(0);
  m_zoomAnimation->setEndValue(0.5);

  m_radiusAnimation = new QPropertyAnimation(this, "radius");
  m_radiusAnimation->setDuration(120);

  m_scaleAnim = new QPropertyAnimation(this, "scale");
  m_scaleAnim->setDuration(250);
  m_scaleAnim->setEasingCurve(QEasingCurve::InOutQuad);

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

  QPointF c;
  qreal maxDist = 0;
  for (const QPointF &pt : m_polygon) {
    c += pt;
    maxDist = qMax(maxDist, (pt - m_originalPolygon.last()).manhattanLength());
  }
  c /= m_polygon.size();

  QRadialGradient grad(c, maxDist * m_scale);
  if (m_grayedOut) {
    grad.setColorAt(0.0, QColor(50, 50, 50, 220));
    grad.setColorAt(0.5, QColor(35, 35, 35, 160));
    grad.setColorAt(1.0, QColor(20, 20, 20, 100));
  } else if (m_isChecked || m_isHovered) {
    grad.setColorAt(0.0, QColor(180, 0, 0, 220));
    grad.setColorAt(0.5, QColor(120, 0, 0, 160));
    grad.setColorAt(1.0, QColor(0, 0, 0, 0));
  } else {
    grad.setColorAt(0.0, QColor(0, 0, 0, 220));
    grad.setColorAt(0.5, QColor(0, 0, 0, 120));
    grad.setColorAt(1.0, QColor(0, 0, 0, 0));
  }

  painter.setBrush(grad);
  painter.setPen(Qt::NoPen);
  painter.drawPolygon(m_polygon);

  // Icon
  int iconSize = qRound(qMax(8.0, 35.0 * m_scale * (1.0 + m_zoom)));
  QFont iconFont = awesome->font(fa::fa_solid, iconSize);
  painter.setFont(iconFont);
  QFontMetrics fm(iconFont);
  QRectF iconRect = fm.boundingRect(QString(m_icon));
  QPointF iconPos(c.x() - iconRect.width() / 2, c.y() - iconRect.height() / 2);
  painter.setPen(m_grayedOut ? QColor(90, 90, 90) : Qt::white);
  painter.drawText(QRectF(iconPos, iconRect.size()), Qt::AlignCenter,
                   QString(m_icon));

  // Text label below icon
  int labelSize = qMax(6, qRound(10 * m_scale));
  QFont labelFont;
  labelFont.setPixelSize(labelSize);
  painter.setFont(labelFont);
  painter.setPen(m_grayedOut ? QColor(90, 90, 90) : Qt::white);

  qreal labelW = qMin(c.x() * 2, maxDist * m_scale * 1.2);
  qreal labelY = c.y() + iconRect.height() / 2 + 2;
  QRectF labelRect(c.x() - labelW / 2, labelY, labelW, 16);
  QFontMetrics lfm(labelFont);
  QString elided = lfm.elidedText(m_text, Qt::ElideRight,
                                  static_cast<int>(labelW));
  painter.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, elided);
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

void CircularTriangleButton::mouseMoveEvent(QMouseEvent *event) {
  event->ignore();
}

void CircularTriangleButton::setGrayedOut(bool grayed) {
  if (m_grayedOut == grayed) return;
  m_grayedOut = grayed;
  if (grayed) {
    m_savedBackgroundColor = m_backgroundColor;
    m_savedHoverBackgroundColor = m_hoverBackgroundColor;
    m_backgroundColor = QColor(20, 20, 20, 200);
  } else {
    m_backgroundColor = m_savedBackgroundColor;
    m_hoverBackgroundColor = m_savedHoverBackgroundColor;
  }
  update();
}

void CircularTriangleButton::setVisualHighlight(bool highlighted) {
  if (m_visualHighlighted == highlighted) return;
  m_visualHighlighted = highlighted;
  m_isHovered = highlighted;
  m_zoomAnimation->stop();
  m_zoomAnimation->setStartValue(m_zoom);
  m_zoomAnimation->setEndValue(highlighted ? 0.5 : 0.0);
  m_zoomAnimation->start();
  m_radiusAnimation->stop();
  m_radiusAnimation->setStartValue(m_radius);
  m_radiusAnimation->setEndValue(highlighted ? 75 : 50);
  m_radiusAnimation->start();
  update();
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
  m_originalPolygon = polygon;
  QRegion region(m_polygon.toPolygon());
  setMask(region);
  update();
}

QColor CircularTriangleButton::backgroundColor() const {
  return m_backgroundColor;
}

void CircularTriangleButton::setChecked(bool checked) {
  m_isChecked = checked;
  updateIcon(checked);
}

void CircularTriangleButton::setBackgroundColor(const QColor &color) {
  m_backgroundColor = color;
  update();
}

void CircularTriangleButton::setTextColor(const QColor &color) {
  m_textColor = color;
  update();
}

void CircularTriangleButton::setScale(qreal s) {
  m_scale = s;
  if (m_originalPolygon.isEmpty()) return;
  QPointF anchor = m_originalPolygon.last();
  m_polygon.clear();
  for (const QPointF &pt : m_originalPolygon)
    m_polygon << anchor + (pt - anchor) * s;
  QRegion region(m_polygon.toPolygon());
  setMask(region);
  update();
}

void CircularTriangleButton::animateScale(qreal target) {
  if (m_scaleAnim->state() == QPropertyAnimation::Running)
    m_scaleAnim->stop();
  m_scaleAnim->setStartValue(m_scale);
  m_scaleAnim->setEndValue(target);
  m_scaleAnim->start();
}
