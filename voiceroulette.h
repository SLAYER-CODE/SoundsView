#ifndef VOICEROULETTE_H
#define VOICEROULETTE_H

#include "cirlebutoneditconvert.h"
#include "polygonbutton.h"
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

public:
  double windowOpacity() const;
  void setWindowOpacity(double opacity);

  QPoint hoverPos() const;
  void setHoverPos(const QPoint &pos);

  qreal radiusButons() const;
  void setradiusButons(const qreal &border);

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
  void hideEvent(QHideEvent *event) override;

private:
  double m_opacity;

  void startFadeInAnimation();

  void setupButtonLoquendo();
  void setupButtons(QStringList list);
  void setupButtonsMenu(QMap<QString, QPair<QChar, QChar>> list);
  QList<ButtonData *> m_buttons;
  QList<ButtonDataMenu *> m_buttonsMenu;

  CirleButonEditConvert *m_buttonloquendo;
  QPropertyAnimation *m_animation;

  QPropertyAnimation *animation;        // Animation for the hover position
  QPropertyAnimation *animation_radius; // Animation for the hover position

  qreal m_radiusButons;

  QPointF m_restoreMouse;
  QPoint m_hoverPos; // Position of the hovered button's center
  bool isHovering;   // Flag to check if a button is being hovered
  bool m_menuSelect;

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
