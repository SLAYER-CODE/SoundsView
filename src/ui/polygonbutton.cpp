#include "polygonbutton.h"
#include "iconmanager.h"
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
                             qreal nextangle, QWidget *parent,
                             const QChar &icon)
    : QWidget(parent), m_text(text), m_icon(icon.isNull() ? QChar(static_cast<char16_t>(fa::fa_music)) : icon),
      m_centerX(centerX_a), m_centerY(centerY_a),
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
  qreal innerR = R * m_innerRadiusRatio;
  bool fullCircle = qAbs(m_nextangle - m_angle) >= 359.9;

  QPointF p1(m_centerX + R * cos(qDegreesToRadians(m_angle)),
             m_centerY + R * sin(qDegreesToRadians(m_angle)));
  QPointF p2(m_centerX + R * cos(qDegreesToRadians(m_nextangle)),
             m_centerY + R * sin(qDegreesToRadians(m_nextangle)));
  QPointF p1_inner(m_centerX + innerR * cos(qDegreesToRadians(m_angle)),
                   m_centerY + innerR * sin(qDegreesToRadians(m_angle)));
  QPointF p2_inner(m_centerX + innerR * cos(qDegreesToRadians(m_nextangle)),
                   m_centerY + innerR * sin(qDegreesToRadians(m_nextangle)));

  QPolygonF polygon;
  polygon << p1_inner << p1 << p2 << p2_inner;

  QPainterPath arcPath;
  if (fullCircle) {
    QRectF outerRect(m_centerX - R, m_centerY - R, 2 * R, 2 * R);
    QRectF innerRect(m_centerX - innerR, m_centerY - innerR,
                     2 * innerR, 2 * innerR);
    QPainterPath outerP, innerP;
    outerP.addEllipse(outerRect);
    innerP.addEllipse(innerRect);
    arcPath = outerP - innerP;

    m_innerArcPath = QPainterPath();
    m_innerArcPath.addEllipse(innerRect);
  } else {
    QRectF outerRect(m_centerX - R, m_centerY - R, 2 * R, 2 * R);
    QRectF innerRect(m_centerX - innerR, m_centerY - innerR,
                     2 * innerR, 2 * innerR);
    arcPath.moveTo(p1_inner);
    arcPath.lineTo(p1);
    arcPath.arcTo(outerRect, -m_angle, -(m_nextangle - m_angle));
    arcPath.lineTo(p2_inner);
    arcPath.arcTo(innerRect, -m_nextangle, +(m_nextangle - m_angle));
    arcPath.closeSubpath();

    m_innerArcPath = QPainterPath();
    m_innerArcPath.moveTo(p2_inner);
    m_innerArcPath.arcTo(innerRect, -m_nextangle, +(m_nextangle - m_angle));
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
  double textR = R * 0.72;
  double tx = m_centerX + textR * cos(midAngle);
  double ty = m_centerY + textR * sin(midAngle);
  double tw = R * 0.4;
  double th = R * 0.22;
  inscribedRect = QRectF(tx - tw / 2, ty - th / 2, tw, th);
  fontSize =
      calculateFontSize(Utils::getLongestWord(text()), inscribedRect, 16);
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
  qreal innerR = R * m_innerRadiusRatio;
  QPointF center(m_centerX, m_centerY);

  qreal fade = m_gradientProgress;
  qreal minV = 0.18;

  if (m_fillOverride.isValid()) {
    QColor c = m_fillOverride;
    if (m_visualHighlighted || m_isHovered)
      c = c.lighter(180);
    QColor fillCol(c.red(), c.green(), c.blue(), 220);
    painter.setBrush(fillCol);
    if (m_bluntCorners)
      painter.setPen(QPen(fillCol, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else
      painter.setPen(Qt::NoPen);
  } else {
    QRadialGradient grad(center, R, center);
    grad.setFocalPoint(center);
    int aOuter = qMax(qRound(120 * (1.0 - fade)), qRound(120 * minV));
    int aMid   = qMax(qRound(250 * (1.0 - fade)), qRound(250 * minV));
    int aInner = qMax(qRound(206 * (1.0 - fade)), qRound(206 * minV));

    {
      QColor cInner = m_gradientColorEnd;
      QColor cMid   = m_gradientColorMiddle;
      QColor cOuter = m_gradientColorStart;
      if (m_visualHighlighted || m_isHovered) {
        cInner = cInner.lighter(150);
        cMid   = cMid.lighter(180);
        cOuter = cOuter.lighter(200);
      }
      grad.setColorAt(0.44, QColor(cInner.red(), cInner.green(), cInner.blue(), aInner));
      grad.setColorAt(0.72, QColor(cMid.red(), cMid.green(), cMid.blue(), aMid));
      grad.setColorAt(1.0, QColor(cOuter.red(), cOuter.green(), cOuter.blue(), aOuter));
    }
    painter.setBrush(grad);
  }
  painter.setPen(Qt::NoPen);
  painter.drawPath(m_arcPath);

  if (m_visualHighlighted || m_isHovered) {
    QPen whitePen(m_highlightColor, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(whitePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(m_innerArcPath);
  }

  painter.restore();

  painter.save();

  if (m_rotateWithAngle) {
    double midAngle = fmod((m_angle + m_nextangle) / 2.0, 360.0);
    QPointF c = inscribedRect.center();
    painter.translate(c);
    painter.rotate(midAngle);
    painter.translate(-c);
  }

  fa::QtAwesome *awesome = IconManager::instance().awesome();
  bool focused = m_visualHighlighted || m_isHovered;

  if (m_iconLeftTextRight) {
    // Icon on the left, text on the right, side by side
    qreal half = inscribedRect.width() * 0.35;
    QRectF iconRect(inscribedRect.x(), inscribedRect.y(),
                    half, inscribedRect.height());
    QRectF textRect(inscribedRect.x() + half, inscribedRect.y(),
                    inscribedRect.width() - half, inscribedRect.height());

    QFont iconFont = awesome->font(fa::fa_solid, fontSize * 1.6);
    painter.setFont(iconFont);
    if (focused) {
      painter.setPen(m_focusColor.darker(150));
      painter.drawText(iconRect.adjusted(2, 2, 2, 2), Qt::AlignCenter, m_icon);
      painter.setPen(m_focusColor);
    } else {
      painter.setPen(m_nonFocusColor);
    }
    painter.drawText(iconRect, Qt::AlignCenter, m_icon);

    QFont tf = m_customFont.family().isEmpty() ? painter.font() : m_customFont;
    painter.setFont(tf);
    QString displayText = text();
    if (focused) {
      painter.setPen(m_focusColor.darker(150));
      painter.drawText(textRect.adjusted(1, 1, 1, 1), Qt::AlignLeft | Qt::AlignVCenter, displayText);
      painter.setPen(m_focusColor);
    } else {
      painter.setPen(m_nonFocusColor);
    }
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, displayText);
  } else {
    QRectF iconRect(inscribedRect.x(), inscribedRect.y(),
                    inscribedRect.width(), inscribedRect.height() * 0.45);

    QFont iconFont = awesome->font(fa::fa_solid, fontSize * 1.4);
    painter.setFont(iconFont);
    if (focused) {
      painter.setPen(m_focusColor.darker(150));
      painter.drawText(iconRect.adjusted(0, 0, 2, 2), Qt::AlignCenter,
                       m_icon);
      painter.setPen(m_focusColor);
    } else {
      painter.setPen(m_nonFocusColor);
    }
    painter.drawText(iconRect, Qt::AlignCenter,
                     m_icon);

    QFont textFont = painter.font();
    textFont.setBold(true);
    textFont.setFamily("CaskaydiaCove Nerd Font");
    textFont.setPointSize(fontSize * 0.8);
    if (!m_customFont.family().isEmpty())
      textFont = m_customFont;
    painter.setFont(textFont);

    QString displayText = text();
    QFontMetrics fm(textFont);
    QString elided = fm.elidedText(displayText, Qt::ElideRight,
                                   qRound(inscribedRect.width()));

    QRectF textRect(inscribedRect.x(), inscribedRect.y() + inscribedRect.height() * 0.45,
                    inscribedRect.width(), inscribedRect.height() * 0.55);

    if (focused) {
      painter.setPen(m_focusColor.darker(150));
      painter.drawText(textRect.adjusted(0, 1, 0, 1), Qt::AlignCenter, elided);
      painter.setPen(m_focusColor);
    } else {
      painter.setPen(m_nonFocusColor);
    }
    painter.drawText(textRect, Qt::AlignCenter, elided);
  }

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

void PolygonButton::animateSizeTo(qreal target) {
  if (m_sizeAnimation->state() == QPropertyAnimation::Running)
    m_sizeAnimation->stop();
  m_sizeAnimation->setStartValue(m_size);
  m_sizeAnimation->setEndValue(target);
  m_sizeAnimation->start();
}

void PolygonButton::setAngle(qreal a) {
  m_angle = a;
  updatePolygon();
  update();
}

void PolygonButton::setnextAngle(qreal a) {
  m_nextangle = a;
  updatePolygon();
  update();
}

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
