#include "cirlebutoneditconvert.h"
#include "circularbutton.h"
#include "iconmanager.h"
#include <QLabel>
#include <QPainter>
#include <QStackedWidget>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFrame>
#include <QVBoxLayout>

CirleButonEditConvert::CirleButonEditConvert(const QString &text, qreal radius,
                                             qreal centerx, qreal centery,
                                             const QChar &icon,
                                             const QChar &iconCheck,
                                             QWidget *parent)
    : QWidget(parent), m_text(text), radius(radius), m_icon(icon),
      m_centerX(centerx), m_centerY(centery), m_iconSize(0), m_isHovered(false),
      m_hoverBackgroundColor(Qt::black),
      m_backgroundColor(QColor(31, 31, 31, 200)), m_iconUnchecked(iconCheck),
      m_iconchecked(icon), exp(1), pointMessage(centerx, centery),
      mov(QPointF(0, 0)), m_zoom(0), m_check(false), m_opacity(0),
      m_elementsize(0) {
  // setCheckable(true);

  m_elementexpancion = new QPropertyAnimation(this, "elementexpancion");
  m_elementexpancion->setDuration(500);
  m_elementexpancion->setStartValue(0);
  m_elementexpancion->setEndValue(1);

  m_expancion = new QPropertyAnimation(this, "expancion");
  m_expancion->setDuration(300);
  m_expancion->setStartValue(1);
  m_expancion->setEndValue(3.3);

  m_zoomAnimation = new QPropertyAnimation(this, "zoom");
  m_zoomAnimation->setDuration(300);
  m_zoomAnimation->setStartValue(0);
  m_zoomAnimation->setEndValue(0.5);

  m_moving = new QPropertyAnimation(
      this, "moving");        // Animación sobre la propiedad 'pos'
  m_moving->setDuration(300); // Duración de 500ms
  m_moving->setStartValue(pos());
  m_moving->setEndValue(QPointF((radius * 3.5) / 2, (radius * 3.5) / 2));

  m_moving->setEasingCurve(QEasingCurve::InQuad);

  // Crea la animación
  m_posAnimation = new QPropertyAnimation(this, "pos");
  m_posAnimation->setDuration(200); // Duración de la animación en milisegundos
  m_posAnimation->setStartValue(0); // Posición inicial
  m_posAnimation->setEndValue(10);  // Levanta el ícono 10 píxeles hacia arriba
  m_posAnimation->setEasingCurve(QEasingCurve::OutBounce); // Efecto de rebote

  m_mogAnimation = new QPropertyAnimation(this, "mog");
  m_mogAnimation->setDuration(200); // Duración de la animación en milisegundos
  m_mogAnimation->setStartValue(0); // Posición inicial
  m_mogAnimation->setEndValue((radius * 3.5) /
                              2); // Levanta el ícono 10 píxeles hacia arriba
  m_mogAnimation->setEasingCurve(QEasingCurve::OutBounce); // Efecto de rebote

  setFixedSize(2 * radius, 2 * radius); // Set the fixed size of the widget

  placeholder = new QLabel(this);
  placeholder->setStyleSheet(
      "color: gray; font-size: 25pt;background: transparent;"); // Tamaño del
                                                                // placeholder
  placeholder->setWordWrap(true); // Permite el ajuste de texto
  placeholder->setText("Escribe lo que loquendo quieres que reprodusca");
  placeholder->setAlignment(Qt::AlignCenter);

  editline = new QTextEdit(this);
  editline->setEnabled(false);
  editline->setStyleSheet(
      "QTextEdit { "
      "color: white;"                  // Color del texto
      "font-weight: bold;"             // Texto en negrita (bold)
      "font-size: 25pt;"               // Tamaño de fuente 25 puntos
      "background-color: transparent;" // Fondo negro opcional
      "padding: 5px;" // Padding de 10 píxeles en todos los lados
      "}");
  editline->setAlignment(Qt::AlignCenter);
  editline->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editline->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editline->setCursorWidth(0);

  // Centrar el texto horizontalmente
  QTextCursor cursor(editline->textCursor());
  QTextBlockFormat blockFormat;
  blockFormat.setAlignment(Qt::AlignCenter);
  cursor.setBlockFormat(blockFormat);
  editline->setTextCursor(cursor);

  // Ajustar la altura del documento para centrar el texto verticalmente
  QTextDocument *doc = editline->document();
  QTextFrame *frame = doc->rootFrame();
  QTextFrameFormat frameFormat = frame->frameFormat();
  frameFormat.setTopMargin(editline->height() / 2);
  frameFormat.setBottomMargin(editline->height() / 2);
  frame->setFrameFormat(frameFormat);

  placeholder->show();
  editline->show();

  opacityEffect = new QGraphicsOpacityEffect(placeholder);

  opacityEffect->setOpacity(0.0);
  placeholder->setGraphicsEffect(opacityEffect);

  m_opacityEfect = new QPropertyAnimation(opacityEffect, "opacity");
  m_opacityEfect->setDuration(300);   // Duración de 1 segundo
  m_opacityEfect->setStartValue(0.0); // Inicia en opacidad 0 (oculto)
  m_opacityEfect->setEndValue(1.0);   // Termina en opacidad 1 (visible)

  // editline->show();
  // connect(this, &QPushButton::toggled, this,
  // &CirleButonEditConvert::updateIcon);

  connect(editline, &QTextEdit::textChanged, this,
          &CirleButonEditConvert::updatePlaceholder);

  connect(m_expancion, &QPropertyAnimation::finished, this,
          &CirleButonEditConvert::onZoomAnimationFinished);

  // Alinear los widgets en el centro del layout
  awesome = IconManager::instance().awesome();

  treants = new CircularButton("", QChar(static_cast<char16_t>(fa::fa_layer_group)), QChar(static_cast<char16_t>(fa::fa_list_check)),
                               radius / 2, this);
  clear = new CircularButton("", QChar(static_cast<char16_t>(fa::fa_minus)), QChar(static_cast<char16_t>(fa::fa_circle_check)),
                             radius / 2, this);
  clear->setIconColor(QColor(128, 128, 128));

  icon_place = new QLabel(this);
  QFont font = awesome->font(fa::fa_solid, 40);
  icon_place->setFont(font);

  // Cambiar el color del texto usando setStyleSheet
  icon_place->setStyleSheet("color: white;");
  icon_place->setText(icon); // Establecer el ícono
  icon_place->setGeometry(0, 0, radius * 2,
                          radius * 2);       // Definir la geometría del QLabel
  icon_place->setAlignment(Qt::AlignCenter); // Alinear al centro

  // treants->setGeometry(radius / 2, radius / 2, radius * 2, radius * 2);
  // treants->show();

  // CircularButton *clear = new CircularButton("", fa::fa_circle_check,
  // fa::fa_delete_left, 0, this); CircularButton *sendmessage = new
  // CircularButton("",fa::fa_circle_check,fa::fa_delete_left,0,this);

  // Crear un efecto de opacidad para el botón
  QGraphicsOpacityEffect *opacityEffectClear =
      new QGraphicsOpacityEffect(clear);
  opacityEffectClear->setOpacity(0.0);
  clear->setGraphicsEffect(opacityEffectClear);

  // Crear una animación para la opacidad
  m_opacityTreant = new QPropertyAnimation(opacityEffectClear, "opacity");
  m_opacityTreant->setDuration(200); // Duración de 1 segundo
  m_opacityTreant->setStartValue(
      0.0); // Comienza con 0% de opacidad (invisible)
  m_opacityTreant->setEndValue(
      1.0); // Termina con 100% de opacidad (totalmente visible)

  // Crear un efecto de opacidad para el botón
  QGraphicsOpacityEffect *opacityEffectTreant =
      new QGraphicsOpacityEffect(treants);
  opacityEffectTreant->setOpacity(0.0);
  treants->setGraphicsEffect(opacityEffectTreant);

  // Crear una animación para la opacidad
  m_opacityClear = new QPropertyAnimation(opacityEffectTreant, "opacity");
  m_opacityClear->setDuration(200);   // Duración de 1 segundo
  m_opacityClear->setStartValue(0.0); // Comienza con 0% de opacidad (invisible)
  m_opacityClear->setEndValue(
      1.0); // Termina con 100% de opacidad (totalmente visible)
}

void CirleButonEditConvert::updatePlaceholder() {
  m_posAnimation->start();
  editline->setCursorWidth(8);
  placeholder->setVisible(false);
}

void CirleButonEditConvert::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing);
  painter.save();
  // Dibujar el círculo
  painter.setBrush(m_backgroundColor);

  if (m_isHovered) {
    painter.setBrush(m_hoverBackgroundColor);
  }
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(0, 0, 2 * (radius * exp), 2 * (radius * exp));

  painter.restore();
}

void CirleButonEditConvert::animatePosition(const QPoint &startPos,
                                            const QPoint &endPos) {
  // Establecer la posición inicial
  move(startPos);
  // Configurar los valores de inicio y fin para la animación
  m_moving->setStartValue(startPos);
  m_moving->setEndValue(endPos);

  // Iniciar la animación
  m_moving->start();
}

void CirleButonEditConvert::toggleExpansion() {
  updateIcon(!m_check);
}

void CirleButonEditConvert::setSendStatus(SendStatus status) {
  switch (status) {
    case SendStatus::Neutral:
      clear->setCustomIcon(QChar(static_cast<char16_t>(fa::fa_minus)));
      clear->setIconColor(QColor(128, 128, 128));
      break;
    case SendStatus::Success:
      clear->setCustomIcon(QChar(static_cast<char16_t>(fa::fa_circle_check)));
      clear->setIconColor(Qt::white);
      break;
    case SendStatus::Failure:
      clear->setCustomIcon(QChar(static_cast<char16_t>(fa::fa_circle_xmark)));
      clear->setIconColor(Qt::red);
      break;
  }
}

void CirleButonEditConvert::updateIcon(bool checked) {
  m_check = checked;
  if (!checked) {
    m_icon = m_iconchecked;
    m_hoverBackgroundColor = QColor(31, 31, 31, 220);

    m_backgroundColor = Qt::black;

    m_moving->setDirection(QPropertyAnimation::Backward);
    m_moving->start();

    m_expancion->setDirection(QPropertyAnimation::Backward);
    m_expancion->start();

    m_opacityEfect->setDirection(QPropertyAnimation::Backward);
    m_opacityEfect->start();

    m_opacityClear->setDirection(QPropertyAnimation::Backward);
    m_opacityClear->start();

    m_opacityTreant->setDirection(QPropertyAnimation::Backward);
    m_opacityTreant->start();

    // m_elementexpancion->setDirection(QPropertyAnimation::Forward);
    // m_elementexpancion->start();
  } else {
    m_backgroundColor = QColor(31, 31, 31, 220);
    m_hoverBackgroundColor = Qt::black;
    m_icon = m_iconUnchecked;
    m_expancion->setDirection(QPropertyAnimation::Forward);
    m_expancion->start();

    m_moving->setDirection(QPropertyAnimation::Forward);
    m_moving->start();

    m_opacityEfect->setDirection(QPropertyAnimation::Forward);
    m_opacityEfect->start();
  }
  icon_place->setText(m_icon);
  update();
}

void CirleButonEditConvert::onZoomAnimationFinished() {
  if (m_check) {
    editline->setEnabled(true);
    editline->setFocus();
    m_mogAnimation->setDirection(QPropertyAnimation::Forward);
    m_mogAnimation->start();

    m_opacityClear->setDirection(QPropertyAnimation::Forward);
    m_opacityClear->start();

    m_opacityTreant->setDirection(QPropertyAnimation::Forward);
    m_opacityTreant->start();

  } else {
    // m_elementexpancion->setDirection(QPropertyAnimation::Backward);
    // m_elementexpancion->start();

    editline->setEnabled(false);
    editline->tabChangesFocus();
  }

  // Verificar si el mouse está sobre el widget después de la animación
  if (underMouse()) {
    enterEvent(nullptr); // Simular la entrada del mouse
  } else {
    leaveEvent(nullptr); // Simular la salida del mouse
  }
}

void CirleButonEditConvert::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  updatePlaceholderPosition(); // Reposicionar el QLabel cuando el tamaño cambie
}

void CirleButonEditConvert::updatePlaceholderPosition() {
  placeholder->setGeometry(0, 0, width(), height());
  // placeholder->setAlignment(Qt::AlignCenter);
  editline->setGeometry(0, radius * 2, width(), height() - (radius * 2 * 2));
  // editline->setAlignment(Qt::AlignCenter);
  if (m_check) {
    treants->setGeometry(width() / 2 - radius / 2,
                         height() / 2 - mov.x() - radius / 2, radius, radius);

    clear->setGeometry(width() / 2 - radius / 2,
                       height() / 2 - mov.x() - radius / 2, radius, radius);
    icon_place->setGeometry(width() / 2 - radius / 2,
                            height() / 2 - (mov.x() * 1.5) - radius / 2, radius,
                            radius);

  } else {
    treants->setGeometry(width() / 2 - mov.y() - radius / 2,
                         height() / 2 - mov.x() - radius / 2, radius, radius);

    clear->setGeometry(width() / 2 + mov.y() - radius / 2,
                       height() / 2 - mov.x() - radius / 2, radius, radius);

    icon_place->setGeometry(width() / 2 - radius / 2,
                            height() / 2 - (mov.x() * 1.5) - radius / 2, radius,
                            radius);
  }
}

void CirleButonEditConvert::updateMask() {
  QBitmap bitmap(size());
  bitmap.fill(Qt::color0);
  QPainter painter(&bitmap);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setBrush(Qt::color1);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(0, 0, 2 * (radius * exp), 2 * (radius * exp));
  setMask(bitmap);
}

QSize CirleButonEditConvert::scaleSize() const { return size(); }
void CirleButonEditConvert::setScaleSize(QSize &size) {
  size; // Cambiar el tamaño del widget
}

qreal CirleButonEditConvert::mog() const { return m_pos; }

void CirleButonEditConvert::setmog(qreal &mog) {
  treants->setGeometry(width() / 2 - mog - radius / 2,
                       height() / 2 - (mov.x()) - radius / 2, radius, radius);

  clear->setGeometry(width() / 2 + mog - radius / 2,
                     height() / 2 - mov.x() - radius / 2, radius, radius);
  update();
}
qreal CirleButonEditConvert::pos() const { return m_pos; }

void CirleButonEditConvert::setpos(qreal &pos) {
  icon_place->setGeometry(width() / 2 - radius / 2,
                          height() / 2 - (mov.x() * 1.5) + pos - radius / 2,
                          radius, radius);
  m_pos = pos;
  update();
}

QPointF CirleButonEditConvert::moving() { return mov; }

void CirleButonEditConvert::setMoving(QPointF &moving) {
  mov = moving;
  update();
}

qreal CirleButonEditConvert::opacity() const { return m_opacity; }

void CirleButonEditConvert::setopacity(qreal &opacity) {
  m_opacity = opacity;
  update();
}

qreal CirleButonEditConvert::zoom() const { return m_zoom; }

void CirleButonEditConvert::setzoom(const qreal &zom) {
  m_zoom = zom;
  update();
}

qreal CirleButonEditConvert::elementexpancion() { return m_elementsize; }
void CirleButonEditConvert::setElementexpancion(qreal number) {
  m_elementsize = number;
  // clear->resize(number, number);
  // treants->resize(number, number);
  update();
}

qreal CirleButonEditConvert::expancion() { return exp; }
void CirleButonEditConvert::setExpancion(qreal number) {
  exp = number;
  setFixedSize(2 * radius * exp,
               2 * radius * exp); // Ajustar el tamaño del widget dinámicamente
  updateMask();
  setGeometry(m_centerX - (radius * exp), m_centerY - (radius * exp),
              (radius * exp), (radius * exp));
  update();
}

void CirleButonEditConvert::setText(const QString &text) { m_text = text; }

QString CirleButonEditConvert::text() const { return m_text; }

void CirleButonEditConvert::focusOutEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  m_isHovered = true;

  if (!m_check) {
    m_zoomAnimation->setDirection(QPropertyAnimation::Forward);
    m_zoomAnimation->start();
  } else {
    m_zoomAnimation->setDirection(QPropertyAnimation::Backward);
    m_zoomAnimation->start();
  }
  update();
}

void CirleButonEditConvert::focusInEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  m_isHovered = false;
  if (!m_check) {
    m_zoomAnimation->setDirection(QPropertyAnimation::Backward);
    m_zoomAnimation->start();
  } else {
    m_zoomAnimation->setDirection(QPropertyAnimation::Forward);
    m_zoomAnimation->start();
  }
  update();
}
