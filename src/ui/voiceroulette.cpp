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
  // Wayland: use overlay layer (above fullscreen windows)
  setProperty("_qt_wayland_shell_layer", "overlay");
  setMouseTracking(true);
  QScreen *primaryScreen = QGuiApplication::primaryScreen();
  setGeometry(primaryScreen->geometry());
  m_crosshairPos = QPointF(width() / 2.0, height() / 2.0);

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

  const qreal gap = 2.0;
  const qreal kAddButtonSpan = 10.0;
  int radius = 400;
  int centerX = width() / 2;
  int centerY = height() / 2;

  // Reserve space for the + Agregar button (fixed span + gap)
  qreal availableForSounds = 360.0 - kAddButtonSpan - gap;
  qreal angleStep = sounds.isEmpty() ? 0 : availableForSounds / sounds.size();
  qreal cur = 0;

  for (int i = 0; i < sounds.size(); ++i) {
    qreal start = cur + gap / 2.0;
    qreal end = cur + angleStep - gap / 2.0;
    PolygonButton *btn = new PolygonButton(sounds[i].name, centerX, centerY,
                                           radius, start, end, this);
    btn->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);
    audio.setSoundPath(sounds[i].name, sounds[i].filePath);
    ButtonData *data = new ButtonData(btn, start, end);
    m_buttons.append(data);
    btn->show();
    cur += angleStep;
  }

  // + Agregar fixed button at the end
  {
    qreal addStart = cur + gap / 2.0;
    qreal addEnd = addStart + kAddButtonSpan;
    PolygonButton *btn = new PolygonButton("Agregar", centerX, centerY,
                                           radius + 60, addStart, addEnd,
                                           this, QChar('+'));
    btn->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);
    btn->setInnerRadiusRatio(0.44 * radius / (radius + 60));
    btn->setHighlightColor(Qt::white);
    btn->setFocusColor(Qt::white);
    btn->setNonFocusColor(Qt::white);
    btn->setFillOverride(QColor(60, 5, 5));
    btn->setBluntCorners(true);
    btn->setIconLeftTextRight(true);
    btn->setRotateWithAngle(true);
    QFont addFont;
    addFont.setBold(true);
    addFont.setFamily("CaskaydiaCove Nerd Font");
    addFont.setPointSize(11);
    btn->setCustomFont(addFont);
    btn->setEnabled(true);
    btn->setHighlightDisabled(true);
    m_addButtonData = new ButtonData(btn, addStart, addEnd);
    m_buttons.append(m_addButtonData);
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
  m_addButtonData = nullptr;
  m_grabarBtn = nullptr;
  m_escucharBtn = nullptr;
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

void VoiceRoulette::switchToAddMode() {
  if (!m_addButtonData) return;
  // Toggle: if already in add mode, exit
  if (m_addMode) {
    exitAddMode();
    return;
  }
  if (m_grabarBtn) return;

  const qreal gap = 2.0;
  int radius = 400;
  int centerX = width() / 2;
  int centerY = height() / 2;

  qreal entry = (m_addButtonData->startAngle + m_addButtonData->endAngle) / 2.0;

  // Keep the add button visible while we show Grabar/Escuchar radially.
  // m_addButtonData->button->hide();
  m_addMode = true;

  // Collect current sound buttons (excluding add)
  QList<ButtonData*> sounds;
  for (ButtonData *bd : m_buttons)
    if (bd != m_addButtonData) sounds.append(bd);
  int N = sounds.size();
  if (N == 0) return;

  // Create Grabar at entry (0 span)
   const int extraOut = 20;
   PolygonButton *grab = new PolygonButton("Grabar", centerX, centerY,
                                           radius + extraOut, entry, entry,
                                           this, QChar(static_cast<char16_t>(fa::fa_microphone)));
   grab->setGeometry(0, 0, 2 * (radius + extraOut) + centerX, 2 * (radius + extraOut) + centerY);
    grab->setInnerRadiusRatio(0.55);
  grab->setHighlightColor(QColor(180, 80, 0));
  grab->setFocusColor(QColor(255, 150, 50));
  grab->setNonFocusColor(QColor(255, 150, 50));
  grab->setFillOverride(QColor(100, 50, 10));
   grab->setBluntCorners(false);
   grab->setEnabled(true);
   grab->setHighlightDisabled(true);
   grab->setSize(0.95);
   grab->setRotateWithAngle(true);
  m_grabarBtn = new ButtonData(grab, entry, entry);

  // Create Escuchar at entry (0 span)
   PolygonButton *escu = new PolygonButton("Escuchar", centerX, centerY,
                                            radius + extraOut, entry, entry,
                                            this, QChar(static_cast<char16_t>(fa::fa_headphones)));
   escu->setGeometry(0, 0, 2 * (radius + extraOut) + centerX, 2 * (radius + extraOut) + centerY);
    escu->setInnerRadiusRatio(0.55);
  escu->setHighlightColor(QColor(160, 130, 0));
  escu->setFocusColor(QColor(255, 230, 50));
  escu->setNonFocusColor(QColor(255, 230, 50));
  escu->setFillOverride(QColor(90, 75, 10));
   escu->setBluntCorners(false);
   escu->setEnabled(true);
   escu->setHighlightDisabled(true);
   escu->setSize(0.95);
   escu->setRotateWithAngle(true);
  escu->setIconLeftTextRight(true);
  escu->setIconOnRight(true);
  escu->setIconFontScale(0.9);
  m_escucharBtn = new ButtonData(escu, entry, entry);

  grab->setIconLeftTextRight(true);
  grab->setIconOnRight(true);
  grab->setIconFontScale(0.9);
  // Both new buttons are shown radially around the same angular span
  // as the Add button. Keep Add visible underneath.
  grab->show();
  escu->show();

  // We'll make the two new buttons come out from the Add sector edges.
  // Grabar will appear directly CCW adjacent to Add.start, Escuchar CW adjacent to Add.end.
  const qreal kNewSpan = 6.0;   // desired span per new button (smaller)
  const qreal kMinSpan = 6.0;    // minimum allowed span for existing sectors

  qreal addStartOrig = m_addButtonData->startAngle;
  qreal addEndOrig = m_addButtonData->endAngle;
  qreal addSpan = addEndOrig - addStartOrig;
  if (addSpan < 0) addSpan += 360.0;

  // Save originals before animating so we can restore later
  m_savedAngles.clear();
  for (ButtonData *bd : m_buttons) {
    if (bd == m_addButtonData) continue;
    m_savedAngles.append(std::make_tuple(bd, bd->startAngle, bd->endAngle));
  }
  if (m_addButtonData) { m_addOrigStart = m_addButtonData->startAngle; m_addOrigEnd = m_addButtonData->endAngle; }
  // Save visuals of Add button
  if (m_addButtonData) {
    m_addOldText = m_addButtonData->button->text();
    m_addOldIcon = m_addButtonData->button->icon(); // placeholder, will replace next
    // Use getter since PolygonButton stores icon internally
    m_addOldIcon = m_addButtonData->button->icon();
    m_addOldFill = m_addButtonData->button->fillOverride();
    m_addOldFocusColor = m_addButtonData->button->focusColor();
    m_addOldNonFocusColor = m_addButtonData->button->nonFocusColor();
    m_addOldGradStart = m_addButtonData->button->gradientColorStart();
    m_addOldGradMid = m_addButtonData->button->gradientColorMiddle();
    m_addOldGradEnd = m_addButtonData->button->gradientColorEnd();
    m_addOldTextColor = m_addButtonData->button->textColor();
    // Change Add to toggle visual: light red gradient, '-' and text 'Cerrar'
    m_addButtonData->button->setText("Cerrar");
    m_addButtonData->button->setIcon(QChar('-'));
    m_addButtonData->button->setTextColor(Qt::white);
    m_addButtonData->button->setFocusColor(Qt::white);
    m_addButtonData->button->setNonFocusColor(Qt::white);
    // Toggle visual: transparent light red gradient + enlarged
    m_addButtonData->button->setFillOverride(QColor()); // clear to show gradient
    m_addButtonData->button->setGradientColorStart(QColor(220, 60, 60, 60));
    m_addButtonData->button->setGradientColorMiddle(QColor(200, 40, 40, 130));
    m_addButtonData->button->setGradientColorEnd(QColor(160, 20, 20, 200));
    m_addButtonData->button->animateSizeTo(1.06);
    // Disable highlight animations while toggled, but don't draw inner highlight
    m_addButtonData->button->setVisualHighlight(false);
    m_addButtonData->button->setHighlightDisabled(true);
    m_addButtonData->button->setBluntCorners(false);
    m_addButtonData->button->raise();
  }

  qreal needLeft = kNewSpan;   // need this much on the left side
  qreal needRight = kNewSpan;  // need this much on the right side

  // Find index of m_addButtonData
  int addIndex = -1;
  for (int i = 0; i < m_buttons.size(); ++i) if (m_buttons[i] == m_addButtonData) { addIndex = i; break; }
  if (addIndex == -1) addIndex = m_buttons.size() - 1;

  // Collect compression on left (previous sectors) to free needLeft
  QList<QPair<int, qreal>> compressLeft;
  int lidx = addIndex - 1; if (lidx < 0) lidx = m_buttons.size() - 1;
  qreal remL = needLeft;
  while (remL > 0 && lidx != addIndex) {
    ButtonData *bd = m_buttons[lidx];
    qreal span = bd->endAngle - bd->startAngle; if (span < 0) span += 360.0;
    qreal avail = span - kMinSpan;
    if (avail > 0) {
      qreal take = qMin(avail, remL);
      compressLeft.append(qMakePair(lidx, take));
      remL -= take;
    }
    lidx--; if (lidx < 0) lidx = m_buttons.size() - 1;
    if (compressLeft.size() > m_buttons.size()) break;
  }

  // Collect compression on right (next sectors) to free needRight
  QList<QPair<int, qreal>> compressRight;
  int ridx = addIndex + 1; if (ridx >= m_buttons.size()) ridx = 0;
  qreal remR = needRight;
  while (remR > 0 && ridx != addIndex) {
    ButtonData *bd = m_buttons[ridx];
    qreal span = bd->endAngle - bd->startAngle; if (span < 0) span += 360.0;
    qreal avail = span - kMinSpan;
    if (avail > 0) {
      qreal take = qMin(avail, remR);
      compressRight.append(qMakePair(ridx, take));
      remR -= take;
    }
    ridx++; if (ridx >= m_buttons.size()) ridx = 0;
    if (compressRight.size() > m_buttons.size()) break;
  }

  // If couldn't free enough on one side, try to borrow from the other side
  qreal freedL = 0; for (const auto &p : compressLeft) freedL += p.second;
  qreal freedR = 0; for (const auto &p : compressRight) freedR += p.second;
  qreal needMoreL = qMax<qreal>(0.0, needLeft - freedL);
  qreal needMoreR = qMax<qreal>(0.0, needRight - freedR);
  if (needMoreL > 0) {
    // try to get more from right side neighbors
    int scan = ridx;
    while (needMoreL > 0 && scan != addIndex) {
      ButtonData *bd = m_buttons[scan]; qreal span = bd->endAngle - bd->startAngle; if (span < 0) span += 360.0;
      qreal avail = span - kMinSpan; for (const auto &p: compressRight) if (p.first == scan) avail -= p.second;
      if (avail > 0) {
        qreal take = qMin(avail, needMoreL);
        compressRight.append(qMakePair(scan, take)); freedR += take; needMoreL -= take;
      }
      scan++; if (scan >= m_buttons.size()) scan = 0;
    }
  }
  if (needMoreR > 0) {
    int scan = lidx;
    while (needMoreR > 0 && scan != addIndex) {
      ButtonData *bd = m_buttons[scan]; qreal span = bd->endAngle - bd->startAngle; if (span < 0) span += 360.0;
      qreal avail = span - kMinSpan; for (const auto &p: compressLeft) if (p.first == scan) avail -= p.second;
      if (avail > 0) {
        qreal take = qMin(avail, needMoreR);
        compressLeft.append(qMakePair(scan, take)); freedL += take; needMoreR -= take;
      }
      scan--; if (scan < 0) scan = m_buttons.size() - 1;
    }
  }

  // Targets: grab directly before addStart, esc directly after addEnd
  qreal grabStart = addStartOrig - kNewSpan;
  while (grabStart < 0) grabStart += 360.0;
  qreal grabEnd = addStartOrig;
  qreal escStart = addEndOrig;
  qreal escEnd = addEndOrig + kNewSpan;

  g_animSectors.clear();

  // Combine left and right compression per button index so a sector
  // compressed from both sides gets a single entry with both targets.
  QMap<int, std::tuple<PolygonButton*, qreal, qreal, qreal, qreal>> merged;
  for (const auto &p : compressLeft) {
    int i = p.first; qreal red = p.second; ButtonData *bd = m_buttons[i];
    merged[i] = {bd->button, bd->startAngle, bd->endAngle, bd->startAngle, bd->endAngle - red};
  }
  for (const auto &p : compressRight) {
    int i = p.first; qreal red = p.second; ButtonData *bd = m_buttons[i];
    auto it = merged.find(i);
    if (it != merged.end()) {
      std::get<3>(it.value()) = bd->startAngle + red;
    } else {
      merged[i] = {bd->button, bd->startAngle, bd->endAngle, bd->startAngle + red, bd->endAngle};
    }
  }
  for (const auto &m : merged)
    g_animSectors.append({std::get<0>(m), std::get<1>(m), std::get<2>(m), std::get<3>(m), std::get<4>(m)});

  // New grab and escu occupy space adjacent to Add (not overlapping Add)
  // Shrink Add so it sits exactly between grab and escu with no gap
  qreal centerGapStart = grabEnd;
  qreal centerGapEnd = escStart;
  // Ensure normalized
  while (centerGapStart < 0) centerGapStart += 360.0;
  while (centerGapEnd < 0) centerGapEnd += 360.0;
  // Animate grab/escu from the Add edges so they appear to come out from Add
  g_animSectors.append({grab, addStartOrig, addStartOrig, grabStart, grabEnd});
  g_animSectors.append({m_addButtonData->button, m_addButtonData->startAngle, m_addButtonData->endAngle, centerGapStart, centerGapEnd});
  g_animSectors.append({escu, addEndOrig, addEndOrig, escStart, escEnd});

  m_transitionAnim->stop();
  m_transitionAnim->setStartValue(0.0);
  m_transitionAnim->setEndValue(1.0);
  m_transitionAnim->start();

  connect(m_transitionAnim, &QPropertyAnimation::finished, this, [this, compressLeft, compressRight, grabStart, grabEnd, escStart, escEnd]() {
    // Apply final angles to compressed neighbors and append grab/escu buttons to m_buttons
    // (previous 'compress' loop removed — we now have compressLeft/compressRight)
    // Update compressed neighbors stored angles
    for (const auto &p : compressLeft) {
      int iidx = p.first;
      ButtonData *bd = m_buttons[iidx];
      bd->startAngle = bd->button->angle();
      bd->endAngle = bd->button->nextangle();
    }
    for (const auto &p : compressRight) {
      int iidx = p.first;
      ButtonData *bd = m_buttons[iidx];
      bd->startAngle = bd->button->angle();
      bd->endAngle = bd->button->nextangle();
    }

    // Update Add stored angles to its new centered position (center gap)
    if (m_addButtonData) {
      // centerGap calculated earlier during animation setup; recover from button
      m_addButtonData->startAngle = m_addButtonData->button->angle();
      m_addButtonData->endAngle = m_addButtonData->button->nextangle();
    }

    if (m_grabarBtn) {
      m_grabarBtn->startAngle = grabStart;
      m_grabarBtn->endAngle = grabEnd;
      // register as active button so it participates in hover/focus
      m_buttons.append(m_grabarBtn);
    }
    if (m_escucharBtn) {
      m_escucharBtn->startAngle = escStart;
      m_escucharBtn->endAngle = escEnd;
      m_buttons.append(m_escucharBtn);
    }
    // Ensure Add is rendered on top so it remains visible in the center gap
    if (m_addButtonData && m_addButtonData->button) m_addButtonData->button->raise();
    m_addMode = true;
  }, Qt::SingleShotConnection);
}

void VoiceRoulette::exitAddMode() {
  if (!m_addMode) return;
  // Animate Grabar/Escuchar back to entry (zero span) and restore compressed neighbors
  if (!m_grabarBtn && !m_escucharBtn) {
    m_addMode = false;
    return;
  }

  qreal entry = (m_addButtonData->startAngle + m_addButtonData->endAngle) / 2.0;
  g_animSectors.clear();

  // compressed neighbors: we will restore them by reading current button angles
  // Use saved originals if available
  if (!m_savedAngles.isEmpty()) {
    for (const auto &t : m_savedAngles) {
      ButtonData *bd; qreal s,e; std::tie(bd,s,e) = t;
      g_animSectors.append({bd->button, bd->button->angle(), bd->button->nextangle(), s, e});
    }
  } else {
    for (ButtonData *bd : m_buttons) {
      if (bd == m_addButtonData) continue;
      g_animSectors.append({bd->button, bd->button->angle(), bd->button->nextangle(), bd->startAngle, bd->endAngle});
    }
  }

  // shrink Grabar/Escuchar to entry
  if (m_grabarBtn) g_animSectors.append({m_grabarBtn->button, m_grabarBtn->button->angle(), m_grabarBtn->button->nextangle(), entry, entry});
  if (m_escucharBtn) g_animSectors.append({m_escucharBtn->button, m_escucharBtn->button->angle(), m_escucharBtn->button->nextangle(), entry, entry});

  m_transitionAnim->stop();
  m_transitionAnim->setStartValue(0.0);
  m_transitionAnim->setEndValue(1.0);
  m_transitionAnim->start();

  connect(m_transitionAnim, &QPropertyAnimation::finished, this, [this]() {
    // hide and delete grab/escu and restore Add as visible
    for (auto it = m_buttons.begin(); it != m_buttons.end();) {
      if (*it == m_grabarBtn || *it == m_escucharBtn)
        it = m_buttons.erase(it);
      else
        ++it;
    }
    if (m_grabarBtn) {
      m_grabarBtn->button->hide();
      m_grabarBtn->button->deleteLater();
      delete m_grabarBtn;
      m_grabarBtn = nullptr;
    }
    if (m_escucharBtn) {
      m_escucharBtn->button->hide();
      m_escucharBtn->button->deleteLater();
      delete m_escucharBtn;
      m_escucharBtn = nullptr;
    }
    // Restore stored angles on m_buttons from saved list
    if (!m_savedAngles.isEmpty()) {
      for (const auto &t : m_savedAngles) {
        ButtonData *bd; qreal s,e; std::tie(bd,s,e) = t;
        bd->button->setAngle(s);
        bd->button->setnextAngle(e);
        bd->button->show();
        bd->startAngle = s; bd->endAngle = e;
      }
    } else {
      for (ButtonData *bd : m_buttons) {
        bd->button->setAngle(bd->startAngle);
        bd->button->setnextAngle(bd->endAngle);
        bd->button->show();
      }
    }
    if (m_addButtonData) {
      m_addButtonData->startAngle = m_addOrigStart;
      m_addButtonData->endAngle = m_addOrigEnd;
      m_addButtonData->button->setAngle(m_addOrigStart);
      m_addButtonData->button->setnextAngle(m_addOrigEnd);
      // restore visuals
      m_addButtonData->button->setText(m_addOldText);
      m_addButtonData->button->setIcon(m_addOldIcon);
      m_addButtonData->button->setFillOverride(m_addOldFill);
      m_addButtonData->button->setTextColor(m_addOldTextColor);
      m_addButtonData->button->setGradientColorStart(m_addOldGradStart);
      m_addButtonData->button->setGradientColorMiddle(m_addOldGradMid);
      m_addButtonData->button->setGradientColorEnd(m_addOldGradEnd);
    m_addButtonData->button->animateSizeTo(1.0);
      m_addButtonData->button->setTextColor(m_addButtonData->button->nonFocusColor());
      m_addButtonData->button->setBluntCorners(false);
      m_addButtonData->button->setVisualHighlight(false);
      m_addButtonData->button->setHighlightDisabled(true); // stays static on hover
      m_addButtonData->button->setFocusColor(m_addOldFocusColor);
      m_addButtonData->button->setNonFocusColor(m_addOldNonFocusColor);
      m_addButtonData->button->show();
    }
    m_grabarActive = false;
    m_escucharActive = false;
    setSoundButtonsDisabled(false);
    m_savedAngles.clear();
    m_addMode = false;
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

  // Crosshair
  painter.setRenderHint(QPainter::Antialiasing);
  double cx = m_crosshairPos.x();
  double cy = m_crosshairPos.y();
  double len = 10.0;
  QPen crossPen(Qt::white, 2.0);
  painter.setPen(crossPen);
  painter.drawLine(QPointF(cx - len, cy), QPointF(cx + len, cy));
  painter.drawLine(QPointF(cx, cy - len), QPointF(cx, cy + len));
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
    if (m_addMode && m_addButtonData)
      m_addButtonData->button->hide();
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
    if (m_addMode) {
      exitAddMode();
      event->accept();
      return;
    }
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
    if (m_grabarActive || m_escucharActive) { event->accept(); return; }
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
    if (m_grabarActive || m_escucharActive) { event->accept(); return; }
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
  if (event->key() == Qt::Key_Alt && !(m_grabarActive || m_escucharActive)) {
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

  // Compute constrained crosshair position
  QPoint center(width() / 2, height() / 2);
  double dx = pos.x() - center.x();
  double dy = pos.y() - center.y();
  double dist = std::sqrt(dx * dx + dy * dy);
  double maxR = m_altHeld ? 160.0 : 400.0;
  if (dist > maxR) {
    dx = dx / dist * maxR;
    dy = dy / dist * maxR;
  }
  m_crosshairPos = QPointF(center.x() + dx, center.y() + dy);
  update();

  if (m_buttonloquendo->isExpanded() && m_altHeld) {
    m_buttonloquendo->updateEmojiHighlight(pos.x());
    return;
  }

  int dx2 = pos.x() - center.x();
  int dy2 = pos.y() - center.y();

  double angle = qAtan2(dy2, dx2) * (180.0 / M_PI);
  if (angle < 0) angle += 360;

  if (m_menuSelect) {
    ButtonDataMenu *rawSector = nullptr;
    for (ButtonDataMenu *data : m_buttonsMenu) {
      if (!data->button->isEnabled()) continue;
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
      if (current) {
          double span = current->endAngle - current->startAngle;
          if (span < 0) span += 360.0;
          if (distanceToSectorEdge(angle, current->startAngle,
                                   current->endAngle) < qMin(kHysteresis, span * 0.25))
            target = current->button;
      }
    }
    for (const ButtonDataMenu *data : m_buttonsMenu)
      data->button->setVisualHighlight(data->button == target && data->button->isEnabled());
    m_focusedButton = (target && target->isEnabled()) ? target : nullptr;
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
      if (current) {
          double span = current->endAngle - current->startAngle;
          if (span < 0) span += 360.0;
          if (distanceToSectorEdge(angle, current->startAngle,
                                   current->endAngle) < qMin(kHysteresis, span * 0.25))
            target = current->button;
      }
    }
    for (const ButtonData *data : m_listButtons)
      data->button->setVisualHighlight(data->button == target);
    m_focusedButton = target;
  } else {
    ButtonData *rawSector = nullptr;
    for (ButtonData *data : m_buttons) {
      if (!data->button->isEnabled()) continue;
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
      if (current) {
          double span = current->endAngle - current->startAngle;
          if (span < 0) span += 360.0;
          if (distanceToSectorEdge(angle, current->startAngle,
                                   current->endAngle) < qMin(kHysteresis, span * 0.25))
            target = current->button;
      }
    }
    for (const ButtonData *data : m_buttons)
      data->button->setVisualHighlight(data->button == target && data->button->isEnabled());
    m_focusedButton = (target && target->isEnabled()) ? target : nullptr;
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
      if (!data->button->isEnabled()) continue;
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
      if (!data->button->isEnabled()) continue;
      if (angleInRange(angle, data->startAngle, data->endAngle)) {
        QString name = data->button->text().trimmed();
        if (name == "Agregar" || name == "Cerrar") {
          switchToAddMode();
          break;
        }
        if (name == "Grabar") {
          m_grabarActive = !m_grabarActive;
          setSoundButtonsDisabled(m_grabarActive || m_escucharActive);
          if (m_grabarBtn) {
            PolygonButton *g = m_grabarBtn->button;
            if (m_grabarActive) {
              m_grabarOldGradStart = g->gradientColorStart();
              m_grabarOldGradMid = g->gradientColorMiddle();
              m_grabarOldGradEnd = g->gradientColorEnd();
              m_grabarOldTextColor = g->textColor();
              m_grabarOldFillOverride = g->fillOverride();
              g->setFillOverride(QColor());
              g->setGradientColorStart(QColor(130, 15, 15, 120));
              g->setGradientColorMiddle(QColor(180, 25, 25, 250));
              g->setGradientColorEnd(QColor(90, 8, 8, 206));
              g->setTextColor(Qt::white);
            } else {
              g->setFillOverride(m_grabarOldFillOverride);
              g->setGradientColorStart(m_grabarOldGradStart);
              g->setGradientColorMiddle(m_grabarOldGradMid);
              g->setGradientColorEnd(m_grabarOldGradEnd);
              g->setTextColor(m_grabarOldTextColor);
            }
            g->animateSizeTo(m_grabarActive ? 1.04 : 1.0);
          }
          break;
        }
        if (name == "Escuchar") {
          m_escucharActive = !m_escucharActive;
          setSoundButtonsDisabled(m_grabarActive || m_escucharActive);
          if (m_escucharBtn) {
            PolygonButton *e = m_escucharBtn->button;
            if (m_escucharActive) {
              m_escucharOldGradStart = e->gradientColorStart();
              m_escucharOldGradMid = e->gradientColorMiddle();
              m_escucharOldGradEnd = e->gradientColorEnd();
              m_escucharOldTextColor = e->textColor();
              m_escucharOldFillOverride = e->fillOverride();
              e->setFillOverride(QColor());
              e->setGradientColorStart(QColor(130, 15, 15, 120));
              e->setGradientColorMiddle(QColor(180, 25, 25, 250));
              e->setGradientColorEnd(QColor(90, 8, 8, 206));
              e->setTextColor(Qt::white);
            } else {
              e->setFillOverride(m_escucharOldFillOverride);
              e->setGradientColorStart(m_escucharOldGradStart);
              e->setGradientColorMiddle(m_escucharOldGradMid);
              e->setGradientColorEnd(m_escucharOldGradEnd);
              e->setTextColor(m_escucharOldTextColor);
            }
            e->animateSizeTo(m_escucharActive ? 1.04 : 1.0);
          }
          break;
        }
        if (m_grabarActive || m_escucharActive) break;
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



void VoiceRoulette::setSoundButtonsDisabled(bool disabled) {
  for (ButtonData *bd : m_buttons) {
    if (bd == m_addButtonData || bd == m_grabarBtn || bd == m_escucharBtn) {
      if (disabled) {
        bool isThisActive = (bd == m_grabarBtn && m_grabarActive) ||
                            (bd == m_escucharBtn && m_escucharActive);
        bd->button->setGrayedOut(!isThisActive);
        bd->button->setEnabled(isThisActive);
      } else {
        bd->button->setGrayedOut(false);
        bd->button->setEnabled(true);
      }
      continue;
    }
    bd->button->setEnabled(!disabled);
    bd->button->setGrayedOut(disabled);
  }
  for (ButtonDataMenu *md : m_buttonsMenu) {
    md->button->setEnabled(!disabled);
    md->button->setGrayedOut(disabled);
  }
  m_buttonloquendo->setLocked(disabled);
  if (disabled) {
    QChar recIcon = m_grabarActive ? QChar(static_cast<char16_t>(fa::fa_microphone))
                                   : QChar(static_cast<char16_t>(fa::fa_headphones));
    QString recText = m_grabarActive ? "Esperando tu hermoza voz"
                                     : "Grabando sonido del equipo";
    if (m_buttonloquendo->isExpanded()) {
      m_buttonloquendo->setRecordingOverlay(recIcon, recText);
    } else {
      m_buttonloquendo->setRecordingOverlay(QChar(), QString());
      m_buttonloquendo->setMessageIcon(recIcon);
    }
  } else {
    m_buttonloquendo->setRecordingOverlay(QChar(), QString());
    m_buttonloquendo->resetMessageIcon();
  }
  if (m_grabarActive) {
    m_buttonloquendo->setProgressBarColor(QColor(200, 30, 30));
    m_buttonloquendo->animateProgress(1.0);
  } else if (m_escucharActive) {
    m_buttonloquendo->setProgressBarColor(QColor(200, 30, 30));
    m_buttonloquendo->animateProgress(1.0);
  } else {
    m_buttonloquendo->setProgressBarColor(QColor(255, 160, 0));
    m_buttonloquendo->animateProgress(0.0);
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
