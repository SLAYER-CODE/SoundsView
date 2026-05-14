#include "polygonbutton.h"
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath> // Asegúrate de incluir QPainterPath
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
  QPolygonF polygon;
  polygon << center
          << QPointF(m_centerX + ((rad + m_radius) * m_size) *
                                     cos(qDegreesToRadians(m_angle)),
                     m_centerY + ((rad + m_radius) * m_size) *
                                     sin(qDegreesToRadians(m_angle)))
          << QPointF(m_centerX + ((rad + m_radius) * m_size) *
                                     cos(qDegreesToRadians(m_nextangle)),
                     m_centerY + ((rad + m_radius) * m_size) *
                                     sin(qDegreesToRadians(m_nextangle)));

  QRegion region(polygon.toPolygon());
  setMask(region);
  inscribedRect = Utils::CalculateRec(polygon);
  fontSize =
      calculateFontSize(Utils::getLongestWord(text()), inscribedRect, 15);
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

  // Guardar el estado inicial del pintor
  painter.save();

  // Calcular y dibujar el polígono
  QPointF center(m_centerX, m_centerY);
  QPointF p1(m_centerX +
                 ((rad + m_radius) * m_size) * cos(qDegreesToRadians(m_angle)),
             m_centerY +
                 ((rad + m_radius) * m_size) * sin(qDegreesToRadians(m_angle)));
  QPointF p2(m_centerX + ((rad + m_radius) * m_size) *
                             cos(qDegreesToRadians(m_nextangle)),
             m_centerY + ((rad + m_radius) * m_size) *
                             sin(qDegreesToRadians(m_nextangle)));
  QPolygonF polygon;
  polygon << center << p1 << p2;
  QRectF inscribe = Utils::CalculateRec(polygon);

  // Configurar el gradiente
  QLinearGradient gradient(polygon.at(0), (polygon.at(2) + polygon.at(1)) / 2);
  QLinearGradient transitionGradient(gradient);
  transitionGradient.setColorAt(1, m_gradientColorStart);
  transitionGradient.setColorAt(
      0.5, QColor::fromRgbF(
               m_gradientColorMiddle.redF() * m_gradientProgress +
                   m_gradientColorStart.redF() * (1 - m_gradientProgress),
               m_gradientColorMiddle.greenF() * m_gradientProgress +
                   m_gradientColorStart.greenF() * (1 - m_gradientProgress),
               m_gradientColorMiddle.blueF() * m_gradientProgress +
                   m_gradientColorStart.blueF() * (1 - m_gradientProgress),
               m_gradientColorMiddle.alphaF() * m_gradientProgress +
                   m_gradientColorStart.alphaF() * (1 - m_gradientProgress)));
  transitionGradient.setColorAt(
      0, QColor::fromRgbF(
             m_gradientColorEnd.redF() * m_gradientProgress +
                 m_gradientColorMiddle.redF() * (1 - m_gradientProgress),
             m_gradientColorEnd.greenF() * m_gradientProgress +
                 m_gradientColorMiddle.greenF() * (1 - m_gradientProgress),
             m_gradientColorEnd.blueF() * m_gradientProgress +
                 m_gradientColorMiddle.blueF() * (1 - m_gradientProgress),
             m_gradientColorEnd.alphaF() * m_gradientProgress +
                 m_gradientColorMiddle.alphaF() * (1 - m_gradientProgress)));

  painter.setBrush(transitionGradient);
  painter.setPen(Qt::NoPen); // Deshabilitar el borde
  painter.drawPolygon(polygon);

  // Restaurar el estado del pintor después de dibujar el polígono
  painter.restore();

  // Guardar el estado del pintor para la parte del texto
  painter.save();

  QTextDocument textDoc;
  textDoc.setTextWidth(inscribedRect.width());

  QFont font = painter.font();
  font.setBold(true);
  font.setFamily(
      "CaskaydiaCove Nerd Font"); // Cambia la familia de la fuente aquí
  font.setPointSize(fontSize);
  textDoc.setDefaultFont(font);
  QTextOption textOption;
  textOption.setWrapMode(QTextOption::WordWrap); // Asegura el ajuste del texto
  textOption.setAlignment(Qt::AlignHCenter);
  textDoc.setDefaultTextOption(textOption);

  // Ajustar el tamaño del texto si es necesario
  QString displayText = text();
  QFontMetrics fm(font);
  QRectF textBoundingRect = fm.boundingRect(displayText);
  QRectF boundingRect = polygon.boundingRect();
  qreal textHeight = textDoc.size().height();
  QPointF topLeft(inscribe.left(),
                  inscribe.top() + (inscribe.height() - textHeight) / 2);

  // Configurar el texto
  QString htmlText =
      QString("<strong style=\"color: #ffffff\">%1</strong>").arg(displayText);
  textDoc.setHtml(htmlText);

  painter.translate(topLeft); // Mover el pintor a la esquina superior izquierda
                              // del rectángulo centrado
  textDoc.drawContents(&painter, QRectF(QPointF(0, 0), inscribe.size()));

  // Restaurar el estado del pintor después de dibujar el texto
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
