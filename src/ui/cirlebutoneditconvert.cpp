#include "cirlebutoneditconvert.h"
#include "circularbutton.h"
#include "iconmanager.h"
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
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
  m_expancion->setEndValue(3.5);

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
      "color: #999999; font-size: 16pt; background: transparent;");
  placeholder->setAlignment(Qt::AlignCenter);
  placeholder->setText("Escribir texto...");

  editline = new QTextEdit(this);
  editline->setEnabled(false);
  editline->setStyleSheet(
      "QTextEdit {"
      "color: #f0f0f0;"
      "font-size: 16pt;"
      "background: transparent;"
      "padding: 0px;"
      "border: none;"
      "}");
  editline->setAlignment(Qt::AlignCenter);
  editline->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editline->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editline->viewport()->setCursor(Qt::BlankCursor);
  editline->setCursorWidth(0);

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

  connect(treants, &QPushButton::clicked, this,
          &CirleButonEditConvert::onSendClicked);
  connect(clear, &QPushButton::clicked, this,
          &CirleButonEditConvert::onClearClicked);

  setMouseTracking(true);
  editline->installEventFilter(this);
  treants->installEventFilter(this);
  clear->installEventFilter(this);

  m_progressAnim = new QPropertyAnimation(this, "progress");
  m_progressAnim->setDuration(300);
  m_progressAnim->setEasingCurve(QEasingCurve::Linear);
}

void CirleButonEditConvert::setInitialExpanded() {
  m_check = true;
  m_icon = m_iconUnchecked;
  icon_place->setText(m_icon);
  m_backgroundColor = Qt::black;
  m_hoverBackgroundColor = Qt::black;

  qreal v = (radius * 3.5) / 2;
  m_pos = v;
  mov = QPointF(v, v);
  setExpancion(3.5);
  setmog(m_pos);
  updatePlaceholderPosition();
  m_mogAnimation->setStartValue(m_pos);
  m_mogAnimation->setEndValue(m_pos);

  editline->setEnabled(true);
  editline->setFocus();
  editline->setCursor(Qt::BlankCursor);
  editline->viewport()->setCursor(Qt::BlankCursor);
  treants->setCursor(Qt::BlankCursor);
  clear->setCursor(Qt::BlankCursor);

  treants->setIconColor(QColor(128, 128, 128));
  clear->setIconColor(QColor(128, 128, 128));

  placeholder->setVisible(false);
  opacityEffect->setOpacity(1.0);
  qobject_cast<QGraphicsOpacityEffect*>(m_opacityTreant->targetObject())->setOpacity(1.0);
  qobject_cast<QGraphicsOpacityEffect*>(m_opacityClear->targetObject())->setOpacity(1.0);

  update();
}

void CirleButonEditConvert::updatePlaceholder() {
  m_posAnimation->start();
  editline->setCursorWidth(8);
  placeholder->setVisible(false);
}

void CirleButonEditConvert::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  editline->viewport()->setCursor(Qt::BlankCursor);
  if (m_firstShow) {
    m_firstShow = false;
    if (m_check) {
      qreal v = (radius * 3.5) / 2;
      m_pos = v;
      mov = QPointF(v, v);
      setmog(m_pos);
      updatePlaceholderPosition();
    }
  }
}

static QPixmap makeNoiseTexture() {
  QPixmap tex(64, 64);
  tex.fill(Qt::transparent);
  QPainter p(&tex);
  p.setRenderHint(QPainter::Antialiasing);
  for (int i = 0; i < 600; ++i) {
    int x = QRandomGenerator::global()->bounded(64);
    int y = QRandomGenerator::global()->bounded(64);
    int a = QRandomGenerator::global()->bounded(8, 25);
    p.setPen(QPen(QColor(255, 255, 255, a), 1));
    p.drawPoint(x, y);
  }
  p.end();
  return tex;
}

void CirleButonEditConvert::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.save();

  QRectF circleRect(0, 0, 2 * (radius * exp), 2 * (radius * exp));

  painter.setBrush(m_locked ? QColor(40, 40, 40) : Qt::black);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(circleRect);

  QRectF arcRect = circleRect.adjusted(3, 3, -3, -3);
  QPen trackPen(m_locked ? QColor(80, 80, 80, 60) : QColor(255, 255, 255, 30), 4);
  trackPen.setCapStyle(Qt::RoundCap);
  painter.setPen(trackPen);
  painter.setBrush(Qt::NoBrush);
  painter.drawEllipse(arcRect);

  if (m_progress > 0.0) {
    QConicalGradient cg(arcRect.center(), 90);
    cg.setColorAt(0.0, m_progressBarColor.lighter(140));
    cg.setColorAt(0.5, m_progressBarColor);
    cg.setColorAt(1.0, m_progressBarColor.darker(150));
    QPen arcPen(QBrush(cg), 4);
    arcPen.setCapStyle(Qt::RoundCap);
    painter.setPen(arcPen);
    painter.drawArc(arcRect, 90 * 16, -m_progress * 360 * 16);
  }

  // Recording overlay text + icon
  if (m_locked && !m_recordingText.isEmpty()) {
    QColor darkGrey(80, 80, 80);
    qreal w = width(), h = height();

    QFont iconFont = awesome->font(fa::fa_solid, 24);
    painter.setFont(iconFont);
    painter.setPen(darkGrey);
    painter.drawText(QRectF(w / 2 - 30, h / 2 - 50, 60, 40), Qt::AlignCenter, m_recordingIcon);

    QFont textFont;
    textFont.setPointSize(10);
    painter.setFont(textFont);
    painter.setPen(darkGrey);
    painter.drawText(QRectF(w / 2 - 120, h / 2, 240, 30), Qt::AlignCenter, m_recordingText);
  }

  painter.restore();
}

void CirleButonEditConvert::updateEmojiHighlight(int mouseX) {
  int cx = width() / 2;
  int dx = mouseX - cx;
  bool toRight = false, toLeft = false;
  if (m_highlightedSide == Left)
    toRight = dx > kHysteresisPx;
  else if (m_highlightedSide == Right)
    toLeft = dx < -kHysteresisPx;
  else {
    toRight = dx > 0;
    toLeft = dx < 0;
  }
  if (toRight) { highlightLeft(false); highlightRight(true); }
  else if (toLeft) { highlightLeft(true); highlightRight(false); }
}

void CirleButonEditConvert::mouseMoveEvent(QMouseEvent *event) {
  if (m_locked || !m_check || !m_altActive) {
    event->ignore();
    return;
  }
  updateEmojiHighlight(event->pos().x());
}

void CirleButonEditConvert::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  highlightLeft(false);
  highlightRight(false);
}

bool CirleButonEditConvert::eventFilter(QObject *obj, QEvent *event) {
  if (m_locked) return true;
  if (m_check && m_altActive && event->type() == QEvent::MouseMove) {
    QMouseEvent *me = static_cast<QMouseEvent *>(event);
    QPoint local = mapFromGlobal(me->globalPosition().toPoint());
    updateEmojiHighlight(local.x());
    if (obj == editline) return true;
    return false;
  }
  if (m_check && m_altActive && obj == editline &&
      (event->type() == QEvent::MouseButtonPress ||
       event->type() == QEvent::MouseButtonRelease ||
       event->type() == QEvent::MouseButtonDblClick)) {
    return true;
  }
  return QWidget::eventFilter(obj, event);
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
  m_posAnimation->stop();
  m_pos = 0;
  if (!checked) {
    highlightLeft(false);
    highlightRight(false);
    m_icon = m_iconchecked;
    m_hoverBackgroundColor = QColor(31, 31, 31, 220);

    m_backgroundColor = Qt::black;
    m_treantsActive = false;
    m_clearActive = false;
    treants->setIconColor(QColor(128, 128, 128));
    clear->setIconColor(QColor(128, 128, 128));

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

    m_mogAnimation->setStartValue(0);
    m_mogAnimation->setEndValue((radius * 3.5) / 2);

    // m_elementexpancion->setDirection(QPropertyAnimation::Forward);
    // m_elementexpancion->start();
  } else {
    m_backgroundColor = Qt::black;
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
    editline->setCursor(Qt::BlankCursor);
    editline->viewport()->setCursor(Qt::BlankCursor);
    treants->setCursor(Qt::BlankCursor);
    clear->setCursor(Qt::BlankCursor);
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
  qreal R = radius * exp;
  qreal cx = width() / 2.0;
  qreal cy = height() / 2.0;

  placeholder->setGeometry(0, 0, width(), height());

  bool expanded = m_check;
  placeholder->setVisible(expanded);
  editline->setVisible(expanded);

  if (expanded) {
    qreal textW = R * 1.2;
    qreal textH = R * 0.5;
    editline->setGeometry(static_cast<int>(cx - textW / 2),
                          static_cast<int>(cy - textH / 2),
                          static_cast<int>(textW), static_cast<int>(textH));

    QTextDocument *doc = editline->document();
    QTextFrame *frame = doc->rootFrame();
    QTextFrameFormat fmt = frame->frameFormat();
    fmt.setTopMargin(0);
    fmt.setBottomMargin(0);
    frame->setFrameFormat(fmt);
  }

  if (m_check) {
    treants->setGeometry(static_cast<int>(cx - mog() - radius / 2.0),
                         static_cast<int>(cy - mov.x() - radius / 2.0),
                         radius, radius);
    clear->setGeometry(static_cast<int>(cx + mog() - radius / 2.0),
                       static_cast<int>(cy - mov.x() - radius / 2.0),
                       radius, radius);
    icon_place->setGeometry(static_cast<int>(cx - radius / 2.0),
                            static_cast<int>(cy - (mov.x() * 1.5) - radius / 2.0),
                            radius, radius);
  } else {
    treants->setGeometry(static_cast<int>(cx - mov.y() - radius / 2.0),
                         static_cast<int>(cy - mov.x() - radius / 2.0),
                         radius, radius);
    clear->setGeometry(static_cast<int>(cx + mov.y() - radius / 2.0),
                       static_cast<int>(cy - mov.x() - radius / 2.0),
                       radius, radius);
    icon_place->setGeometry(static_cast<int>(cx - radius / 2.0),
                            static_cast<int>(cy - (mov.x() * 1.5) - radius / 2.0),
                            radius, radius);
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

void CirleButonEditConvert::triggerHighlighted() {
  if (!m_check || m_locked) return;
  CircularButton *btn = nullptr;
  if (m_highlightedSide == Left)
    btn = treants;
  else if (m_highlightedSide == Right)
    btn = clear;
  if (btn) {
    btn->click();
    QPoint globalCenter = btn->mapToGlobal(btn->rect().center());
    QCursor::setPos(globalCenter);
  }
}

void CirleButonEditConvert::onSendClicked() {
  if (!m_check || m_locked) return;
  QString text = editline->toPlainText().trimmed();
  if (!text.isEmpty())
    emit sendRequested(text);
  m_treantsActive = !m_treantsActive;
  treants->setIconColor(m_treantsActive ? QColor(76, 175, 80) : QColor(128, 128, 128));
}

void CirleButonEditConvert::onClearClicked() {
  if (!m_check || m_locked) return;
  editline->clear();
  placeholder->setVisible(true);
  editline->setCursorWidth(0);
  setNeutral();
  m_clearActive = !m_clearActive;
  clear->setIconColor(m_clearActive ? QColor(76, 175, 80) : QColor(128, 128, 128));
}

void CirleButonEditConvert::setLocked(bool locked) {
  m_locked = locked;
  treants->setVisible(!locked);
  clear->setVisible(!locked);
  editline->setVisible(!locked);
  placeholder->setVisible(!locked);
  icon_place->setVisible(!locked || !m_check);
  update();
}

void CirleButonEditConvert::setProgress(qreal p) {
  m_progress = qBound(0.0, p, 1.0);
  update();
}

void CirleButonEditConvert::animateProgress(qreal target) {
  if (m_progressAnim->state() == QPropertyAnimation::Running)
    m_progressAnim->stop();
  m_progressAnim->setStartValue(m_progress);
  m_progressAnim->setEndValue(target);
  m_progressAnim->start();
}
