#include "polygonbutton.h"
#include <QBitmap>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QPropertyAnimation>
#include <QTextDocument>

#include "utils.h"
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QTextCursor>
#include <QTextFrameFormat>

#include <QMouseEvent>
#include <QString>
#include <QStringList>

PolygonButton::PolygonButton(const QString &text, qreal centerX_a,
                             qreal centerY_a, qreal radius, qreal angle,
                             qreal nextangle, QWidget *parent)
    : QWidget(parent), m_text(text), m_centerX(centerX_a), m_centerY(centerY_a),
      m_angle(angle), m_nextangle(nextangle), m_backgroundColor(Qt::red),
      m_hoverBackgroundColor(Qt::black), m_borderColor(Qt::black),
      m_hoverBorderColor(Qt::white), m_isHovered(false),
      m_gradientColorStart(QColor(31, 31, 31, 120)),
      m_gradientColorMiddle(QColor(1, 1, 1, 250)),
      m_gradientColorEnd(QColor(0, 0, 0, 206)), m_gradientProgress(0.0),
      m_size(0), m_radius(0), rad(radius), fontSize(13) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setAttribute(Qt::WA_Hover);
  setMouseTracking(true);
  gradientAnimation = new QPropertyAnimation(this, "gradientProgress");
  gradientAnimation->setDuration(500);
  gradientAnimation->setStartValue(0.0);
  gradientAnimation->setEndValue(1.0);

  m_radiusAnimation = new QPropertyAnimation(this, "radius");

  m_radiusAnimation->setDuration(300);
  m_radiusAnimation->setStartValue(0);
  m_radiusAnimation->setEndValue(20);

  // Animation sizeStart
  m_sizeAnimation = new QPropertyAnimation(this, "size");
  m_sizeAnimation->setDuration(300); // Duración de la animación en milisegundos
  m_sizeAnimation->setStartValue(0);
  m_sizeAnimation->setEndValue(1);

  // inscribedRect = Utils::CalculateRec(polygon);
  // fontSize= calculateFontSize(text,inscribedRect,20);
  // setPolygon(polygon);
}

void PolygonButton::updatePolygon() {
  QPointF center(m_centerX, m_centerY);
  qreal R = (rad + m_radius) * m_size;
  bool fullCircle = qAbs(m_nextangle - m_angle) >= 359.9;

  QPointF p1(m_centerX + R * cos(qDegreesToRadians(m_angle)),
             m_centerY + R * sin(qDegreesToRadians(m_angle)));
  QPointF p2(m_centerX + R * cos(qDegreesToRadians(m_nextangle)),
             m_centerY + R * sin(qDegreesToRadians(m_nextangle)));

  QPolygonF polygon;
  polygon << center << p1 << p2;

  QPainterPath arcPath;
  if (fullCircle) {
    QRectF circleRect(m_centerX - R, m_centerY - R, 2 * R, 2 * R);
    arcPath.addEllipse(circleRect);
  } else {
    arcPath.moveTo(center);
    arcPath.lineTo(p1);
    QRectF arcRect(m_centerX - R, m_centerY - R, 2 * R, 2 * R);
    arcPath.arcTo(arcRect, -m_angle, -(m_nextangle - m_angle));
    arcPath.closeSubpath();
  }
  m_arcPath = arcPath;

  QSize sz = QWidget::size();
  QBitmap maskBitmap(sz);
  maskBitmap.fill(Qt::color0);
  QPainter p(&maskBitmap);
  p.setRenderHint(QPainter::Antialiasing);
  p.setBrush(Qt::color1);
  p.setPen(Qt::NoPen);
  p.drawPath(arcPath);
  p.end();
  setMask(maskBitmap);

  m_polygon = polygon;

  double midAngle = qDegreesToRadians((m_angle + m_nextangle) / 2.0);
  double textR = R * 0.55;
  double tx = m_centerX + textR * cos(midAngle);
  double ty = m_centerY + textR * sin(midAngle);
  double tw = R * 0.5;
  double th = R * 0.15;
  inscribedRect = QRectF(tx - tw / 2, ty - th / 2, tw, th);
  fontSize =
      calculateFontSize(Utils::getLongestWord(text()), inscribedRect, 20);
  update();
}

int PolygonButton::calculateFontSize(const QString &text,
                                     const QRectF &inscribedRect,
                                     int maxFontSize) {
  int fontSize = maxFontSize;
  QFont font;
  font.setPointSize(fontSize);
  QFontMetrics fm(font);
  QRectF textBoundingRect = fm.boundingRect(text);

  while ((fontSize > 10) & (textBoundingRect.width() > inscribedRect.width())) {
    fontSize--;
  }
  return fontSize;
}

void PolygonButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.save();

  qreal R = (rad + m_radius) * m_size;
  QPointF center(m_centerX, m_centerY);
  bool fullCircle = qAbs(m_nextangle - m_angle) >= 359.9;

  qreal fade = m_gradientProgress;
  qreal minV = 0.18;
  QLinearGradient grad;

  if (fullCircle) {
    grad.setStart(center.x() - R, center.y());
    grad.setFinalStop(center.x() + R, center.y());
  } else {
    QPointF p1(m_centerX + R * cos(qDegreesToRadians(m_angle)),
               m_centerY + R * sin(qDegreesToRadians(m_angle)));
    QPointF p2(m_centerX + R * cos(qDegreesToRadians(m_nextangle)),
               m_centerY + R * sin(qDegreesToRadians(m_nextangle)));
    QPolygonF polygon;
    polygon << center << p1 << p2;
    grad.setStart(polygon.at(0));
    grad.setFinalStop((polygon.at(2) + polygon.at(1)) / 2);
  }
  int aOuter = qMax(qRound(120 * (1.0 - fade)), qRound(120 * minV));
  int aMid   = qMax(qRound(250 * (1.0 - fade)), qRound(250 * minV));
  int aInner = qMax(qRound(206 * (1.0 - fade)), qRound(206 * minV));
  grad.setColorAt(1, QColor(31, 31, 31, aOuter));
  grad.setColorAt(0.5, QColor(1, 1, 1, aMid));
  grad.setColorAt(0, QColor(0, 0, 0, aInner));

  painter.setBrush(grad);
  painter.setPen(Qt::NoPen);
  painter.drawPath(m_arcPath);

  painter.restore();

  painter.save();

  QFont font = painter.font();
  font.setBold(true);
  font.setFamily("CaskaydiaCove Nerd Font");
  font.setPointSize(fontSize);
  painter.setFont(font);
  painter.setPen(Qt::white);

  QString displayText = text();
  QFontMetrics fm(font);
  QString elided = fm.elidedText(displayText, Qt::ElideRight,
                                 qRound(inscribedRect.width()));
  painter.drawText(inscribedRect, Qt::AlignCenter, elided);

  painter.restore();
}

void PolygonButton::setText(const QString &text) { m_text = text; }

QString PolygonButton::text() const { return m_text; }

void PolygonButton::setSize(qreal size) {
  if (m_size != size) {
    m_size = size;
    updatePolygon();
  }
}

qreal PolygonButton::size() const { return m_size; }

void PolygonButton::setRadius(const qreal radius) {
  if (m_radius != radius) {
    m_radius = radius;
    updatePolygon();
  }
}

qreal PolygonButton::radius() const { return m_radius; }

qreal PolygonButton::centerX() const { return m_centerX; }

void PolygonButton::setCenterX(qreal centerX) {
  m_centerX = centerX;
  update();
}

qreal PolygonButton::centerY() const { return m_centerY; }

void PolygonButton::setCenterY(qreal centerY) {
  m_centerY = centerY;
  update(); // Esto asegura que paintEvent sea llamado
}

QPolygonF PolygonButton::polygon() const { return m_polygon; }

void PolygonButton::setPolygon(const QPolygonF &polygon) {
  if (m_polygon != polygon) {
    m_polygon = polygon;
    update();
  }
}

void PolygonButton::setBackgroundColor(const QColor &color) {
  m_backgroundColor = color;
  update();
}

void PolygonButton::setTextColor(const QColor &color) {
  m_textColor = color;
  update();
}

qreal PolygonButton::gradientProgress() const { return m_gradientProgress; }

void PolygonButton::setGradientProgress(qreal progress) {
  m_gradientProgress = progress;
  update();
}

QColor PolygonButton::gradientColorStart() const {

  return m_gradientColorStart;
}

void PolygonButton::setGradientColorStart(const QColor &color) {
  m_gradientColorStart = color;
  update();
}

QColor PolygonButton::gradientColorMiddle() const {
  return m_gradientColorMiddle;
}

void PolygonButton::setGradientColorMiddle(const QColor &color) {
  m_gradientColorMiddle = color;
  update();
}

QColor PolygonButton::gradientColorEnd() const { return m_gradientColorEnd; }

void PolygonButton::setGradientColorEnd(const QColor &color) {
  m_gradientColorEnd = color;
  update();
}

void PolygonButton::setVisualHighlight(bool highlighted) {
  if (m_visualHighlighted == highlighted) return;
  m_visualHighlighted = highlighted;
  gradientAnimation->stop();
  gradientAnimation->setDirection(highlighted ? QPropertyAnimation::Forward : QPropertyAnimation::Backward);
  gradientAnimation->start();
  m_radiusAnimation->stop();
  m_radiusAnimation->setDirection(highlighted ? QPropertyAnimation::Forward : QPropertyAnimation::Backward);
  m_radiusAnimation->start();
}

void PolygonButton::startSizeAnimation() { m_sizeAnimation->start(); }

void PolygonButton::mousePressEvent(QMouseEvent *event) {
  // Ignora los eventos de mouse
  event->ignore();
}

void PolygonButton::mouseReleaseEvent(QMouseEvent *event) {
  // Ignora los eventos de mouse
  event->ignore();
}

void PolygonButton::mouseMoveEvent(QMouseEvent *event) {
  // Ignora los eventos de mouse
  event->ignore();
}

void PolygonButton::focusInEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  // QPushButton::focusInEvent(event); // Call base class implementation

  m_isHovered = true;
  gradientAnimation->stop();
  gradientAnimation->setDirection(QPropertyAnimation::Forward);
  gradientAnimation->start();

  m_radiusAnimation->stop();

  m_radiusAnimation->setDirection(QPropertyAnimation::Forward);
  m_radiusAnimation->start();

  int x = this->x() + this->width() / 2;
  int y = this->y() + this->height() / 2;
}

// void PolygonButton::enterEvent(QEnterEvent *event)
// {
//     Q_UNUSED(event);
//     QPushButton::enterEvent(event); // Call base class implementation

//     m_isHovered = true;
//     gradientAnimation->stop();
//     gradientAnimation->setDirection(QPropertyAnimation::Forward);
//     gradientAnimation->start();

//     m_radiusAnimation->stop();

//     m_radiusAnimation->setDirection(QPropertyAnimation::Forward);
//     m_radiusAnimation->start();

//     int x = this->x() + this->width() / 2;
//     int y = this->y() + this->height() / 2;
//     //emit hoverEntered(x, y); // Emit the signal with the center position
// }

void PolygonButton::focusOutEvent(QFocusEvent *event) {
  Q_UNUSED(event);

  m_isHovered = false;
  gradientAnimation->stop();
  gradientAnimation->setDirection(QPropertyAnimation::Backward);
  gradientAnimation->start();

  m_radiusAnimation->stop();

  m_radiusAnimation->setDirection(QPropertyAnimation::Backward);
  m_radiusAnimation->start();
}

// void PolygonButton::leaveEvent(QEvent *event) {
//     Q_UNUSED(event);
//     m_isHovered = false;
//     gradientAnimation->stop();
//     gradientAnimation->setDirection(QPropertyAnimation::Backward);
//     gradientAnimation->start();

//     m_radiusAnimation->stop();

//     m_radiusAnimation->setDirection(QPropertyAnimation::Backward);
//     m_radiusAnimation->start();
// }
