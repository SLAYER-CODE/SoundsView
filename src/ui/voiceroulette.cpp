#include "voiceroulette.h"
#include "audiomanager.h"
#include "circulartrianglebutton.h"
#include "cirlebutoneditconvert.h"
#include "iconmanager.h"
#include "polygonbutton.h"
#include "soundmanager.h"
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScreen>
#include <QProcess>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtAwesome.h>
#include <algorithm>
#include <cmath>

#include "structdata.h"

VoiceRoulette::VoiceRoulette(QWidget *parent)
    : QWidget(parent), m_opacity(1.0), m_hoverPos(0, 0),
      m_restoreMouse(QCursor::pos()), m_menuSelect(false),
      m_focusedButton(nullptr) {
  setWindowTitle("Sounds Of Hears");
  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint |
                 Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint |
                 Qt::CustomizeWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setMouseTracking(true);
  QScreen *primaryScreen = QGuiApplication::primaryScreen();
  setGeometry(primaryScreen->geometry());

  fa::QtAwesome *awesome = IconManager::instance().awesome();

  QPalette pal = palette();
  pal.setColor(QPalette::Window, Qt::transparent);
  setPalette(pal);
  setStyleSheet("background: transparent;");

  QMap<QString, QPair<QChar, QChar>> iconMap = {
      {"Config", qMakePair(QChar(static_cast<char16_t>(fa::fa_gear)), QChar(static_cast<char16_t>(fa::fa_gears)))},
      {"Process Audio", qMakePair(QChar(static_cast<char16_t>(fa::fa_bars)), QChar(static_cast<char16_t>(fa::fa_link)))},
      {"Microphone",
       qMakePair(QChar(static_cast<char16_t>(fa::fa_microphone)), QChar(static_cast<char16_t>(fa::fa_microphone_lines_slash)))},
      {"Sound", qMakePair(QChar(static_cast<char16_t>(fa::fa_headphones_simple)), QChar(static_cast<char16_t>(fa::fa_volume_xmark)))}};

  QList<SoundEntry> sounds = SoundManager::instance().scanSounds();

  m_animation = new QPropertyAnimation(this, "windowOpacity");
  m_animation->setDuration(300);
  m_animation->setStartValue(0.0);
  m_animation->setEndValue(1.0);
  m_animation->start(QAbstractAnimation::DeleteWhenStopped);

  animation = new QPropertyAnimation(this, "hoverPos");
  animation->setDuration(200);
  animation->setEasingCurve(QEasingCurve::OutCubic);

  animation_radius = new QPropertyAnimation(this, "radiusButons");
  animation_radius->setDuration(200);
  animation_radius->setStartValue(0);
  animation_radius->setEndValue(200);

  setupButtonsFromSounds(sounds);
  setupButtonsMenu(iconMap);
  setupButtonLoquendo();
  m_buttonloquendo->setInitialExpanded();

  connect(m_buttonloquendo, &CirleButonEditConvert::sendRequested, this,
          [this](const QString &text) {
            if (text.isEmpty()) return;
            speakText(text);
            m_buttonloquendo->markSent();
          });
  connect(m_buttonloquendo, &CirleButonEditConvert::clearRequested, this,
          [this]() {
            stopSpeech();
            m_buttonloquendo->setNeutral();
          });
}

void VoiceRoulette::setupButtonsFromSounds(const QList<SoundEntry> &sounds) {
  AudioManager &audio = AudioManager::instance();

  if (sounds.isEmpty()) {
    PolygonButton *btn = new PolygonButton("no hay sonidos", width() / 2, height() / 2, 400, 0, 360, this);
    btn->setGeometry(0, 0, 2 * 400 + width() / 2, 2 * 400 + height() / 2);
    btn->setVisualHighlight(false);
    ButtonData *data = new ButtonData(btn, 0, 360);
    data->button->setEnabled(false);
    m_buttons.append(data);
    btn->show();
    QTimer::singleShot(0, this, &VoiceRoulette::startAnimations);
    return;
  }

  double angleStep = 360.0 / sounds.size();
  int radius = 400;
  int centerX = width() / 2;
  int centerY = height() / 2;

  for (int i = 0; i < sounds.size(); ++i) {
    double angle = i * angleStep;
    double nextAngle = angle + angleStep;
    PolygonButton *btn = new PolygonButton(sounds[i].name, centerX, centerY, radius, angle, nextAngle, this);
    btn->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);
    audio.setSoundPath(sounds[i].name, sounds[i].filePath);
    ButtonData *data = new ButtonData(btn, angle, nextAngle);
    m_buttons.append(data);
    btn->show();
  }
  QTimer::singleShot(0, this, &VoiceRoulette::startAnimations);
}

void VoiceRoulette::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);

  QPainter painter(this);
  QRadialGradient gradient(QPointF(width() / 2, height() / 2),
                           50 + m_radiusButons);
  gradient.setColorAt(0, Qt::white);
  gradient.setColorAt(0.5, Qt::black);
  gradient.setColorAt(1, Qt::transparent);
  painter.setBrush(gradient);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect());
}

void VoiceRoulette::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  m_restoreMouse = QCursor::pos();
  setCursor(Qt::BlankCursor);
  if (!m_buttonloquendo->isExpanded())
    m_buttonloquendo->setInitialExpanded();
  for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
  for (const ButtonDataMenu *data : m_buttonsMenu) data->button->setVisualHighlight(false);
  m_focusedButton = nullptr;
  raise();
  activateWindow();
}

void VoiceRoulette::hideEvent(QHideEvent *event) {
  menuSelect_change(false);
  for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
  for (const ButtonDataMenu *data : m_buttonsMenu) data->button->setVisualHighlight(false);
  m_focusedButton = nullptr;
  m_buttonloquendo->collapse();
  stopSpeech();
  unsetCursor();
  QWidget::hideEvent(event);
}

double VoiceRoulette::windowOpacity() const { return m_opacity; }

void VoiceRoulette::setWindowOpacity(double opacity) {
  if (m_opacity != opacity) {
    m_opacity = opacity;
    if (QGuiApplication::platformName() != "wayland")
        QWidget::setWindowOpacity(opacity);
    emit windowOpacityChanged(opacity);
  }
}

void VoiceRoulette::setupButtonLoquendo() {
  fa::QtAwesome *awesome = IconManager::instance().awesome();
  int centerX = width() / 2;
  int centerY = height() / 2;
  int radiusMin = (qMin(800, 800)) / 17;
  m_buttonloquendo =
      new CirleButonEditConvert("Loquendo Message", radiusMin, centerX, centerY,
                                QChar(static_cast<char16_t>(fa::fa_comment_dots)), QChar(static_cast<char16_t>(fa::fa_keyboard)), this);
  m_buttonloquendo->setToolTip("Habla atravez de texto con voz de loquendo");
  m_buttonloquendo->setStyleSheet(
      "QToolTip {"
      "background-color: #00000055;"
      "color: white;"
      "border: 5px solid black;"
      "}");
  m_buttonloquendo->setGeometry(centerX - radiusMin, centerY - radiusMin,
                                radiusMin, radiusMin);
  m_buttonloquendo->show();
}

void VoiceRoulette::setupButtonsMenu(QMap<QString, QPair<QChar, QChar>> list) {
  fa::QtAwesome *awesome = IconManager::instance().awesome();

  double angleStepMenu = 360.0 / list.size();
  int radiusMin = (qMin(800, 800)) / 5;
  double borderOffset = 10;
  double adjustedRadius = radiusMin - borderOffset;
  int centerX = width() / 2;
  int centerY = height() / 2;
  int radius = (width() / 2);

  int i = 0;
  for (auto it = list.begin(); it != list.end(); ++it, ++i) {
    double angle = i * angleStepMenu;
    double nextAngleStep = angle + angleStepMenu / 2;
    double nextAngle = angle + angleStepMenu;
    double radians = qDegreesToRadians(angle);
    double nextRadians = qDegreesToRadians(nextAngle);
    double nextRadiansstep = qDegreesToRadians(nextAngleStep);

    QPointF center(centerX, centerY);
    QPointF p1(centerX + radiusMin * cos(radians),
               centerY + radiusMin * sin(radians));
    QPointF p2(centerX + radiusMin * cos(nextRadians),
               centerY + radiusMin * sin(nextRadians));
    QPointF p3(centerX + radiusMin * cos(nextRadiansstep),
               centerY + radiusMin * sin(nextRadiansstep));

    QPolygonF polygon;
    polygon << p1 << p3 << p2 << center;

    CircularTriangleButton *button = new CircularTriangleButton(
        it.key(), it->first, it->second, centerX, centerY, adjustedRadius,
        angle, nextAngle, nextRadiansstep, this);
    button->setPolygon(polygon);
    button->setGeometry(0, 0, 2 * radius, 2 * radius);
    button->setToolTip(it.key());
    button->setStyleSheet("QToolTip {"
                          "background-color: #00000055;"
                          "color: white;"
                          "border: 5px solid black;"
                          "}");

    ButtonDataMenu *data = new ButtonDataMenu(button, angle, nextAngle);
    m_buttonsMenu.append(data);
    button->show();
  }
}

void VoiceRoulette::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    menuSelect_change(false);
    for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
    for (const ButtonDataMenu *data : m_buttonsMenu) data->button->setVisualHighlight(false);
    m_focusedButton = nullptr;
    hide();
    event->accept();
  } else if (event->key() == Qt::Key_Control) {
    m_buttonloquendo->toggleExpansion();
    event->accept();
  } else if (event->key() == Qt::Key_Alt) {
    m_altHeld = true;
    m_buttonloquendo->setAltActive(true);
    menuSelect_change(true);
    for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
    event->accept();
  } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
    activateCurrentSector();
    event->accept();
  } else {
    QWidget::keyPressEvent(event);
  }
}

void VoiceRoulette::keyReleaseEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Alt) {
    m_altHeld = false;
    m_buttonloquendo->setAltActive(false);
    menuSelect_change(false);
    for (const ButtonDataMenu *data : m_buttonsMenu) data->button->setVisualHighlight(false);
    event->accept();
  } else {
    QWidget::keyReleaseEvent(event);
  }
}

double VoiceRoulette::angularDistance(double a, double b) const {
  double diff = fmod(a - b + 360, 360);
  if (diff > 180) diff = 360 - diff;
  return diff;
}

double VoiceRoulette::distanceToSectorEdge(double angle, double start,
                                           double end) const {
  return qMin(angularDistance(angle, start), angularDistance(angle, end));
}

void VoiceRoulette::mouseMoveEvent(QMouseEvent *event) {
  QPoint pos = mapFromGlobal(event->globalPosition().toPoint());

  if (m_buttonloquendo->isExpanded() && m_altHeld) {
    m_buttonloquendo->updateEmojiHighlight(pos.x());
    return;
  }

  QPoint center(width() / 2, height() / 2);

  int dx = pos.x() - center.x();
  int dy = pos.y() - center.y();

  double angle = qAtan2(dy, dx) * (180.0 / M_PI);
  if (angle < 0) angle += 360;

  if (m_menuSelect) {
    ButtonDataMenu *rawSector = nullptr;
    for (ButtonDataMenu *data : m_buttonsMenu) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        rawSector = data;
        break;
      }
    }
    QWidget *target = rawSector ? rawSector->button : nullptr;
    if (target && m_focusedButton && target != m_focusedButton) {
      ButtonDataMenu *current = nullptr;
      for (ButtonDataMenu *data : m_buttonsMenu) {
        if (data->button == m_focusedButton) {
          current = data;
          break;
        }
      }
      if (current && distanceToSectorEdge(angle, current->startAngle,
                                          current->endAngle) < kHysteresis)
        target = current->button;
    }
    for (const ButtonDataMenu *data : m_buttonsMenu)
      data->button->setVisualHighlight(data->button == target);
    m_focusedButton = target;
  } else {
    ButtonData *rawSector = nullptr;
    for (ButtonData *data : m_buttons) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        rawSector = data;
        break;
      }
    }
    QWidget *target = rawSector ? rawSector->button : nullptr;
    if (target && m_focusedButton && target != m_focusedButton) {
      ButtonData *current = nullptr;
      for (ButtonData *data : m_buttons) {
        if (data->button == m_focusedButton) {
          current = data;
          break;
        }
      }
      if (current && distanceToSectorEdge(angle, current->startAngle,
                                          current->endAngle) < kHysteresis)
        target = current->button;
    }
    for (const ButtonData *data : m_buttons)
      data->button->setVisualHighlight(data->button == target);
    m_focusedButton = target;
  }
}

void VoiceRoulette::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton) return;
  if (m_buttonloquendo->isExpanded() && m_altHeld) {
    m_buttonloquendo->triggerHighlighted();
    return;
  }
  activateCurrentSector();
}

bool VoiceRoulette::angleInRange(double angle, double start, double end) {
  if (start <= end)
    return angle >= start && angle < end;
  else
    return (angle >= start && angle < 360) || (angle >= 0 && angle < end);
}

void VoiceRoulette::activateCurrentSector() {
  QPoint center(width() / 2, height() / 2);
  QPoint cursorPos = QCursor::pos();
  int dx = cursorPos.x() - center.x();
  int dy = cursorPos.y() - center.y();
  double angle = qAtan2(dy, dx) * (180.0 / M_PI);
  if (angle < 0) angle += 360;

  if (m_menuSelect) {
    for (const ButtonDataMenu *data : m_buttonsMenu) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        data->button->toggle();
        handleMenuAction(data->button->text());
        break;
      }
    }
  } else {
    for (const ButtonData *data : m_buttons) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        QString name = data->button->text().trimmed();
        qDebug() << "Playing sound:" << name;
        AudioManager::instance().playSound(name);
        break;
      }
    }
  }
}

void VoiceRoulette::handleMenuAction(const QString &name) {
  qDebug() << "Menu action:" << name;
  if (name == "Microphone") {
    m_micMuted = !m_micMuted;
    qDebug() << "Microphone" << (m_micMuted ? "muted" : "unmuted");
  } else if (name == "Sound") {
    m_headphoneMuted = !m_headphoneMuted;
    qDebug() << "Headphone monitoring" << (m_headphoneMuted ? "off" : "on");
  } else if (name == "Config") {
    qDebug() << "Open config";
  } else if (name == "Process Audio") {
    qDebug() << "Process audio";
  }
}

void VoiceRoulette::speakText(const QString &text) {
  QProcess::startDetached("spd-say", QStringList() << "-e" << "-r" << "-50" << text);
}

void VoiceRoulette::stopSpeech() {
  QProcess::startDetached("spd-say", QStringList() << "-C");
}

bool VoiceRoulette::menuSelect() { return m_menuSelect; }
void VoiceRoulette::menuSelect_change(const bool change) {
  m_menuSelect = change;
}

QPoint VoiceRoulette::hoverPos() const { return m_hoverPos; }

void VoiceRoulette::setHoverPos(const QPoint &pos) {
  m_hoverPos = pos;
  update();
}

qreal VoiceRoulette::radiusButons() const { return m_radiusButons; }

void VoiceRoulette::setradiusButons(const qreal &radius) {
  m_radiusButons = radius;
  update();
}

QPointF VoiceRoulette::m_mouseRestore() const { return m_restoreMouse; }

void VoiceRoulette::m_setMRestore(const QPointF &point) {
  m_restoreMouse = point;
}

void VoiceRoulette::handleHover(int x, int y) {
  isHovering = true;
  animation_radius->setDirection(QPropertyAnimation::Forward);
  animation_radius->start();
}

void VoiceRoulette::handleLeave() {
  isHovering = false;
  animation_radius->setDirection(QPropertyAnimation::Backward);
  animation_radius->start();
}

void VoiceRoulette::startAnimations() {
  QList<int> indices;
  for (int i = 0; i < m_buttons.size(); ++i) {
    indices.append(i);
  }

  auto rng = std::default_random_engine{QRandomGenerator::global()->generate()};
  std::shuffle(indices.begin(), indices.end(), rng);

  for (int i = 0; i < indices.size(); ++i) {
    int delay = QRandomGenerator::global()->bounded(0, 50);
    QTimer::singleShot(i * delay, this, [=]() {
      m_buttons[indices[i]]->button->startSizeAnimation();
    });
  }
}

void VoiceRoulette::onButtonClickedMenu() {
  CircularTriangleButton *button =
      qobject_cast<CircularTriangleButton *>(sender());
  if (button) {}
}

void VoiceRoulette::onButtonClicked() {
  PolygonButton *button = qobject_cast<PolygonButton *>(sender());
  if (button) {
    QString name = button->text();
    qDebug() << "Button clicked:" << name;
  }
}

void VoiceRoulette::onButtonClickedWithName() {
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button) {
    QString name = button->text();
    qDebug() << "Button clicked:" << name;
  }
}
