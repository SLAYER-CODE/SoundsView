#ifndef CIRCULARBUTTON_H
#define CIRCULARBUTTON_H
#include "iconmanager.h"
#include <QColor>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>

class CircularButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(qreal expancion READ expancion WRITE setExpancion)

public:
  CircularButton(const QString &text, const QChar &icon, const QChar &iconCheck,
                 qreal radius, QWidget *parent = nullptr);
  qreal pos() const;
  void setpos(qreal &pos);

  qreal opacity() const;
  void setopacity(qreal &zoom);

  qreal expancion();
  void setExpancion(qreal number);

  QPointF moving();
  void setMoving(QPointF &zoom);

  void startAnimation();
  void resverseAnimation();

  void setCustomIcon(const QChar &icon) { m_icon = icon; update(); }
  void setIconColor(const QColor &color) { m_iconColor = color; update(); }
  void setHighlight(bool h) { m_highlight = h; update(); }

protected:
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

  void updateMask();

private slots:

private:
  QChar m_icon; // Icono de FontAwesome
  QChar m_iconUnchecked;
  QChar m_iconchecked;

  QColor m_backgroundColor;
  int m_iconSize;
  qreal m_centerX;
  qreal m_centerY;
  int radius;
  qreal exp;
  QPointF pointMessage;

  bool m_isHovered;
  QColor m_hoverBackgroundColor;
  qreal m_zoom;

  bool m_check;

  qreal m_opacity;
  qreal m_pos;
  QLabel *placeholder;

  QPropertyAnimation *m_expancion;
  fa::QtAwesome *awesome;

  QColor m_iconColor;
  bool m_highlight = false;
};

#endif // CIRCULARBUTTON_H
