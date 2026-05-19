#ifndef VOICEROULETTE_H
#define VOICEROULETTE_H

#include "cirlebutoneditconvert.h"
#include "polygonbutton.h"
#include "soundmanager.h"
#include "structdata.h"
#include <QDialog>
#include <QWidget>
class VoiceRoulette : public QWidget {
  Q_OBJECT
  Q_PROPERTY(double windowOpacity READ windowOpacity WRITE setWindowOpacity
                 NOTIFY windowOpacityChanged)
  Q_PROPERTY(double radiusButons READ radiusButons WRITE setradiusButons NOTIFY
                 radiusButonsChanged)
  Q_PROPERTY(QPoint hoverPos READ hoverPos WRITE setHoverPos)
  Q_PROPERTY(qreal transition READ transition WRITE setTransition)

public:
  double windowOpacity() const;
  void setWindowOpacity(double opacity);

  QPoint hoverPos() const;
  void setHoverPos(const QPoint &pos);

  qreal radiusButons() const;
  void setradiusButons(const qreal &border);

  qreal transition() const { return m_transition; }
  void setTransition(qreal t);

  QPointF m_mouseRestore() const;
  void m_setMRestore(const QPointF &point);

  VoiceRoulette(QWidget *parent = nullptr);

  bool menuSelect();
  void menuSelect_change(const bool change);

protected:
  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void hideEvent(QHideEvent *event) override;

private:
  double m_opacity;

  void startFadeInAnimation();

  void setupButtonLoquendo();
  void setupButtonsFromSounds(const QList<SoundEntry> &sounds, bool animate = true);
  void setupButtonsMenu(QMap<QString, QPair<QChar, QChar>> list);
  void setupButtonsFromLists(const QStringList &lists);
  void clearButtons();
  void clearListButtons();
  void switchToListMode();
  void switchToSoundMode(const QString &folderName = QString());
  void saveProfile(const QString &folderName);
  QString loadProfile();
  QList<ButtonData *> m_buttons;
  QList<ButtonDataMenu *> m_buttonsMenu;
  QList<ButtonData *> m_listButtons;

  CirleButonEditConvert *m_buttonloquendo;
  QPropertyAnimation *m_animation;

  QPropertyAnimation *animation;        // Animation for the hover position
  QPropertyAnimation *animation_radius; // Animation for the hover position

  qreal m_radiusButons;

  QPointF m_restoreMouse;
  QPoint m_hoverPos; // Position of the hovered button's center
  bool isHovering;   // Flag to check if a button is being hovered
  bool m_menuSelect;
  bool m_altHeld = false;
  bool m_menuLarge = false;
  QWidget *m_focusedButton;

  void activateCurrentSector();
  void handleMenuAction(const QString &name);
  bool angleInRange(double angle, double start, double end);
  double angularDistance(double a, double b) const;
  double distanceToSectorEdge(double angle, double start, double end) const;
  static constexpr double kHysteresis = 8.0;
  void speakText(const QString &text);
  void stopSpeech();

  bool m_micMuted = false;
  bool m_headphoneMuted = false;
  bool m_listMode = false;
  QList<SoundEntry> m_originalSounds;
  QString m_profileName;
  qreal m_transition = 0.0;
  QPropertyAnimation *m_transitionAnim = nullptr;

signals:
  void windowOpacityChanged(double opacity);
  void radiusButonsChanged(qreal radius);

private slots:
  void startAnimations();
  void onButtonClickedWithName();
  void onButtonClicked();
  void onButtonClickedMenu();
  void handleHover(
      int x,
      int y); // Slot to handle the hover signal from CircularTriangleButton
  void
  handleLeave(); // Slot to handle the hover signal from CircularTriangleButton
};

#endif // VOICEROULETTE_H
