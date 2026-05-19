#include "voiceroulette.h"
#include "audiomanager.h"
#include "circulartrianglebutton.h"
#include "cirlebutoneditconvert.h"
#include "iconmanager.h"
#include "polygonbutton.h"
#include "soundmanager.h"
#include <QApplication>
#include <QCursor>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
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
  m_originalSounds = sounds;

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
  for (ButtonDataMenu *data : m_buttonsMenu)
    data->button->setScale(0.3);
  setupButtonLoquendo();
  m_buttonloquendo->setInitialExpanded();

  m_transitionAnim = new QPropertyAnimation(this, "transition");
  m_transitionAnim->setDuration(400);
  m_transitionAnim->setEasingCurve(QEasingCurve::InOutQuad);

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

  QString saved = loadProfile();
  m_profileName = saved;
  if (!saved.isEmpty()) {
    QList<SoundEntry> profileSounds = SoundManager::instance().scanSoundsInFolder(saved);
    if (!profileSounds.isEmpty()) {
      setupButtonsFromSounds(profileSounds);
    }
  }
}

void VoiceRoulette::setupButtonsFromSounds(const QList<SoundEntry> &sounds, bool animate) {
  clearButtons();
  AudioManager &audio = AudioManager::instance();

  if (sounds.isEmpty()) {
    PolygonButton *btn = new PolygonButton("no hay sonidos", width() / 2, height() / 2, 400, 0, 360, this);
    btn->setGeometry(0, 0, 2 * 400 + width() / 2, 2 * 400 + height() / 2);
    btn->setVisualHighlight(false);
    ButtonData *data = new ButtonData(btn, 0, 360);
    data->button->setEnabled(false);
    m_buttons.append(data);
    btn->show();
    if (animate)
      QTimer::singleShot(0, this, &VoiceRoulette::startAnimations);
    return;
  }

  double gap = 2.0;
  double angleStep = 360.0 / sounds.size();
  int radius = 400;
  int centerX = width() / 2;
  int centerY = height() / 2;

  for (int i = 0; i < sounds.size(); ++i) {
    double angle, nextAngle;
    if (sounds.size() == 1) {
      angle = 0;
      nextAngle = 360;
    } else {
      angle = i * angleStep + gap / 2.0;
      nextAngle = (i + 1) * angleStep - gap / 2.0;
    }
    PolygonButton *btn = new PolygonButton(sounds[i].name, centerX, centerY, radius, angle, nextAngle, this);
    btn->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);
    audio.setSoundPath(sounds[i].name, sounds[i].filePath);
    ButtonData *data = new ButtonData(btn, angle, nextAngle);
    m_buttons.append(data);
    btn->show();
  }
  if (animate)
    QTimer::singleShot(0, this, &VoiceRoulette::startAnimations);
}

void VoiceRoulette::clearButtons() {
  for (ButtonData *data : m_buttons) {
    data->button->hide();
    data->button->deleteLater();
    delete data;
  }
  m_buttons.clear();
}

void VoiceRoulette::clearListButtons() {
  for (ButtonData *data : m_listButtons) {
    data->button->hide();
    data->button->deleteLater();
    delete data;
  }
  m_listButtons.clear();
}

void VoiceRoulette::setupButtonsFromLists(const QStringList &lists) {
  clearListButtons();

  QStringList items;
  for (const QString &l : lists)
    items << l;
  items << "Favoritos" << "Acceso Rapido" << "Agregar";

  double gap = 2.0;
  double angleStep = 360.0 / items.size();
  int radius = 400;
  int centerX = width() / 2;
  int centerY = height() / 2;

  for (int i = 0; i < items.size(); ++i) {
    double angle, nextAngle;
    if (items.size() == 1) {
      angle = 0;
      nextAngle = 360;
    } else {
      angle = i * angleStep + gap / 2.0;
      nextAngle = (i + 1) * angleStep - gap / 2.0;
    }

    QChar icon;
    if (items[i] == "Favoritos")
      icon = QChar(static_cast<char16_t>(fa::fa_heart));
    else if (items[i] == "Acceso Rapido")
      icon = QChar(static_cast<char16_t>(fa::fa_arrow_right));
    else if (items[i] == "Agregar")
      icon = QChar('+');
    else
      icon = QChar(static_cast<char16_t>(fa::fa_folder));

    PolygonButton *btn = new PolygonButton(items[i], centerX, centerY, radius, angle, nextAngle, this, icon);
    btn->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);
    btn->setEnabled(true);
    if (items[i] == "Favoritos") {
      btn->setHighlightColor(Qt::red);
      btn->setFocusColor(Qt::red);
      btn->setNonFocusColor(QColor(180, 60, 60));
    } else if (items[i] == "Acceso Rapido") {
      btn->setHighlightColor(QColor(255, 200, 0));
      btn->setFocusColor(QColor(255, 200, 0));
      btn->setNonFocusColor(QColor(180, 140, 0));
    } else if (items[i] == "Agregar") {
      btn->setHighlightColor(QColor(60, 140, 255));
      btn->setFocusColor(QColor(60, 140, 255));
      btn->setNonFocusColor(QColor(40, 80, 180));
    }
    ButtonData *data = new ButtonData(btn, angle, nextAngle);
    m_listButtons.append(data);
  }
  for (ButtonData *data : m_listButtons)
    data->button->show();

  QTimer::singleShot(0, this, [this]() {
    QList<int> indices;
    for (int i = 0; i < m_listButtons.size(); ++i)
      indices.append(i);
    auto rng = std::default_random_engine{QRandomGenerator::global()->generate()};
    std::shuffle(indices.begin(), indices.end(), rng);
    for (int i = 0; i < indices.size(); ++i) {
      int delay = QRandomGenerator::global()->bounded(0, 50);
      QTimer::singleShot(i * delay, this, [=]() {
        m_listButtons[indices[i]]->button->startSizeAnimation();
      });
    }
  });
}

struct AnimSector {
  PolygonButton *btn;
  qreal startAngle, endAngle;     // at transition=0
  qreal targetStart, targetEnd;   // at transition=1
};

static QList<AnimSector> g_animSectors;
static qreal g_entryAngle = 0;

void VoiceRoulette::setTransition(qreal t) {
  m_transition = t;
  for (const AnimSector &as : g_animSectors) {
    qreal a0 = as.startAngle + (as.targetStart - as.startAngle) * t;
    qreal a1 = as.endAngle + (as.targetEnd - as.endAngle) * t;
    as.btn->setAngle(a0);
    as.btn->setnextAngle(a1);
  }
}

void VoiceRoulette::switchToListMode() {
  m_listMode = true;
  if (m_buttons.isEmpty()) {
    return;
  }

  // Pick entry point: between last and first sector
  qreal entry = m_buttons.last()->endAngle;
  g_entryAngle = entry;

  // Build new list buttons
  QStringList lists = SoundManager::instance().scanSoundLists();
  clearListButtons();
  QStringList items;
  for (const QString &l : lists) items << l;
  items << "Favoritos" << "Acceso Rapido" << "Agregar";

  double gap = 2.0;
  double totalSpan = 360.0 - gap * items.size();
  int radius = 400;
  int centerX = width() / 2;
  int centerY = height() / 2;

  g_animSectors.clear();

  // Old sound sectors: start with current angles, target 0 at entry point
  for (ButtonData *bd : m_buttons) {
    qreal s = bd->startAngle;
    qreal e = bd->endAngle;
    g_animSectors.append({bd->button, s, e, entry, entry});
  }

  // New list sectors: start at entry point (0 span), target their real angles
  qreal cur = entry;
  for (int i = 0; i < items.size(); ++i) {
    qreal span = (items.size() == 1) ? 360.0 : (360.0 / items.size()) - gap;
    qreal s = cur;
    qreal e = cur + span;
    cur = e + gap;

    QChar icon;
    if (items[i] == "Favoritos") icon = QChar(static_cast<char16_t>(fa::fa_heart));
    else if (items[i] == "Acceso Rapido") icon = QChar(static_cast<char16_t>(fa::fa_arrow_right));
    else if (items[i] == "Agregar") icon = QChar('+');
    else icon = QChar(static_cast<char16_t>(fa::fa_folder));

    PolygonButton *btn = new PolygonButton(items[i], centerX, centerY, radius, s, e, this, icon);
    btn->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);
    btn->setEnabled(true);
    btn->setSize(1.0);
    if (items[i] == "Favoritos") {
      btn->setHighlightColor(Qt::red);
      btn->setFocusColor(Qt::red);
      btn->setNonFocusColor(QColor(180, 60, 60));
    } else if (items[i] == "Acceso Rapido") {
      btn->setHighlightColor(QColor(255, 200, 0));
      btn->setFocusColor(QColor(255, 200, 0));
      btn->setNonFocusColor(QColor(180, 140, 0));
    } else if (items[i] == "Agregar") {
      btn->setHighlightColor(QColor(60, 140, 255));
      btn->setFocusColor(QColor(60, 140, 255));
      btn->setNonFocusColor(QColor(40, 80, 180));
    }
    btn->show();

    ButtonData *d = new ButtonData(btn, s, e);
    m_listButtons.append(d);
    g_animSectors.append({btn, entry, entry, s, e});
  }

  // Animate
  m_transitionAnim->stop();
  m_transitionAnim->setStartValue(0.0);
  m_transitionAnim->setEndValue(1.0);
  m_transitionAnim->start();

  connect(m_transitionAnim, &QPropertyAnimation::finished, this, [this]() {
    for (ButtonData *bd : m_buttons) {
      bd->button->hide();
      bd->button->deleteLater();
      delete bd;
    }
    m_buttons.clear();
  }, Qt::SingleShotConnection);
}

void VoiceRoulette::switchToSoundMode(const QString &folderName) {
  m_listMode = false;

  for (ButtonDataMenu *data : m_buttonsMenu) {
    if (data->button->text() == "Process Audio" && data->button->isChecked())
      data->button->toggle();
  }

  // Build new sound buttons (they will animate in)
  if (!folderName.isEmpty())
    m_profileName = folderName;

  if (m_profileName.isEmpty()) {
    setupButtonsFromSounds(m_originalSounds, false);
  } else {
    QList<SoundEntry> sounds = SoundManager::instance().scanSoundsInFolder(m_profileName);
    if (sounds.isEmpty())
      setupButtonsFromSounds(m_originalSounds, false);
    else
      setupButtonsFromSounds(sounds, false);
  }
  for (ButtonData *bd : m_buttons) {
    bd->button->setSize(1.0);
    bd->button->show();
  }

  // Animate: list sectors shrink to entry, sound sectors grow from entry
  qreal entry = g_entryAngle;
  g_animSectors.clear();

  for (ButtonData *bd : m_listButtons) {
    qreal s = bd->startAngle;
    qreal e = bd->endAngle;
    g_animSectors.append({bd->button, s, e, entry, entry});
  }

  qreal cur = entry;
  for (ButtonData *bd : m_buttons) {
    qreal s = bd->startAngle;
    qreal e = bd->endAngle;
    g_animSectors.append({bd->button, entry, entry, s, e});
  }

  m_transitionAnim->stop();
  m_transitionAnim->setStartValue(0.0);
  m_transitionAnim->setEndValue(1.0);
  m_transitionAnim->start();

  connect(m_transitionAnim, &QPropertyAnimation::finished, this, [this]() {
    for (ButtonData *bd : m_listButtons) {
      bd->button->hide();
      bd->button->deleteLater();
      delete bd;
    }
    m_listButtons.clear();
  }, Qt::SingleShotConnection);
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

  if (m_listMode) {
    for (ButtonData *data : m_buttons) data->button->hide();
    for (ButtonData *data : m_listButtons) {
      data->button->setSize(1.0);
      data->button->show();
    }
    m_buttonloquendo->collapse();
  } else {
    for (ButtonData *data : m_buttons) data->button->show();
    for (ButtonData *data : m_listButtons) data->button->hide();
    if (m_menuLarge) {
      m_buttonloquendo->collapse();
    } else if (!m_buttonloquendo->isExpanded()) {
      m_buttonloquendo->setInitialExpanded();
    }
  }

  for (ButtonDataMenu *data : m_buttonsMenu) {
    data->button->setScale(m_menuLarge ? 1.0 : 0.3);
  }

  for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
  for (const ButtonData *data : m_listButtons) data->button->setVisualHighlight(false);
  for (const ButtonDataMenu *data : m_buttonsMenu) data->button->setVisualHighlight(false);
  m_focusedButton = nullptr;
  raise();
  activateWindow();
}

void VoiceRoulette::hideEvent(QHideEvent *event) {
  menuSelect_change(false);
  for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
  for (const ButtonData *data : m_listButtons) data->button->setVisualHighlight(false);
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
    if (m_listMode) {
      switchToSoundMode();
    } else {
      menuSelect_change(false);
      for (const ButtonData *data : m_buttons) data->button->setVisualHighlight(false);
      for (const ButtonData *data : m_listButtons) data->button->setVisualHighlight(false);
      for (const ButtonDataMenu *data : m_buttonsMenu) data->button->setVisualHighlight(false);
      m_focusedButton = nullptr;
      hide();
    }
    event->accept();
  } else if (event->key() == Qt::Key_Control) {
    m_menuLarge = !m_menuLarge;
    if (m_menuLarge) {
      for (ButtonDataMenu *data : m_buttonsMenu)
        data->button->animateScale(1.0);
      m_buttonloquendo->collapse();
    } else {
      for (ButtonDataMenu *data : m_buttonsMenu)
        data->button->animateScale(0.3);
      m_buttonloquendo->toggleExpansion();
    }
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
  a = fmod(a, 360.0);
  b = fmod(b, 360.0);
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
  } else if (m_listMode) {
    ButtonData *rawSector = nullptr;
    for (ButtonData *data : m_listButtons) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        rawSector = data;
        break;
      }
    }
    QWidget *target = rawSector ? rawSector->button : nullptr;
    if (target && m_focusedButton && target != m_focusedButton) {
      ButtonData *current = nullptr;
      for (ButtonData *data : m_listButtons) {
        if (data->button == m_focusedButton) {
          current = data;
          break;
        }
      }
      if (current && distanceToSectorEdge(angle, current->startAngle,
                                           current->endAngle) < kHysteresis)
        target = current->button;
    }
    for (const ButtonData *data : m_listButtons)
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
  start = fmod(start, 360.0);
  end = fmod(end, 360.0);
  if (qFuzzyCompare(start, end))
    return angle >= 0 && angle < 360;
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
  } else if (m_listMode) {
    for (const ButtonData *data : m_listButtons) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        QString name = data->button->text().trimmed();
        if (name == "Favoritos")
          break;
        if (name == "Acceso Rapido") {
          m_profileName.clear();
          saveProfile(QString());
          switchToSoundMode();
          break;
        }
        if (name == "Agregar")
          break;
        saveProfile(name);
        switchToSoundMode(name);
        break;
      }
    }
  } else {
    for (const ButtonData *data : m_buttons) {
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        QString name = data->button->text().trimmed();
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
    if (m_listMode) {
      switchToSoundMode();
    } else {
      switchToListMode();
    }
  }
}



void VoiceRoulette::speakText(const QString &text) {
  QProcess::startDetached("spd-say", QStringList() << "-e" << "-r" << "-50" << text);
  m_buttonloquendo->animateProgress(1.0);
}

void VoiceRoulette::saveProfile(const QString &folderName) {
  QFile file(QCoreApplication::applicationDirPath() + "/perfil.json");
  if (file.open(QIODevice::WriteOnly)) {
    QJsonObject obj;
    obj["profile"] = folderName;
    file.write(QJsonDocument(obj).toJson());
    file.close();
  }
}

QString VoiceRoulette::loadProfile() {
  QFile file(QCoreApplication::applicationDirPath() + "/perfil.json");
  if (file.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isObject())
      return doc.object()["profile"].toString();
  }
  return {};
}

void VoiceRoulette::stopSpeech() {
  QProcess::startDetached("spd-say", QStringList() << "-C");
  m_buttonloquendo->animateProgress(0.0);
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
