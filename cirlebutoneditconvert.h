#ifndef CIRLEBUTONEDITCONVERT_H
#define CIRLEBUTONEDITCONVERT_H
#include "circularbutton.h"
#include "iconmanager.h"
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTextEdit>

class CirleButonEditConvert : public QWidget {
  Q_OBJECT
  Q_PROPERTY(
      qreal elementexpancion READ elementexpancion WRITE setElementexpancion)
  Q_PROPERTY(qreal expancion READ expancion WRITE setExpancion NOTIFY expancion)
  Q_PROPERTY(QPointF moving READ moving WRITE setMoving NOTIFY moving)
  Q_PROPERTY(qreal zoom READ zoom WRITE setzoom)

  Q_PROPERTY(qreal opacity READ opacity WRITE setopacity)

  Q_PROPERTY(qreal pos READ pos WRITE setpos NOTIFY pos)
  Q_PROPERTY(qreal mog READ mog WRITE setmog NOTIFY mog)
  Q_PROPERTY(
      QSize scaleSize READ scaleSize WRITE setScaleSize NOTIFY scaleSizeChanged)

public:
  explicit CirleButonEditConvert(const QString &text, qreal raidus,
                                 qreal centerx, qreal centery,
                                 const QChar &icon, const QChar &iconCheck,
                                 QWidget *parent = nullptr);
  void setText(const QString &text);
  QString text() const;

  QSize scaleSize() const;
  void setScaleSize(QSize &size);

  qreal mog() const;
  void setmog(qreal &mog);

  qreal pos() const;
  void setpos(qreal &pos);

  qreal opacity() const;
  void setopacity(qreal &zoom);

  qreal expancion();
  void setExpancion(qreal number);

  qreal elementexpancion();
  void setElementexpancion(qreal number);

  QPointF moving();
  void setMoving(QPointF &zoom);

  qreal centerX() const;
  void setCenterX(qreal centerX);

  qreal centerY() const;
  void setCenterY(qreal centerY);

  qreal zoom() const;
  void setzoom(const qreal &zo);
  void onZoomAnimationFinished();
  void updatePlaceholderPosition();
  void updatePlaceholder();
  bool isExpanded() const { return m_check; }
  void toggleExpansion();
  void collapse() { updateIcon(false); }

protected:
  void paintEvent(QPaintEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void updateMask();

private slots:
  void updateIcon(bool checked);

private:
  QChar m_icon; // Icono de FontAwesome
  QChar m_iconUnchecked;
  QChar m_iconchecked;

  QColor m_backgroundColor;
  int m_iconSize;
  qreal m_centerX;
  qreal m_centerY;
  qreal m_elementsize;
  int radius;
  qreal exp;
  QPointF pointMessage;

  bool m_isHovered;
  QColor m_hoverBackgroundColor;
  qreal m_zoom;

  bool m_check;

  qreal m_opacity;

  qreal m_pos;
  QLabel *icon_place;
  QLabel *placeholder;
  QTextEdit *editline;

  QPointF mov;

  QSize m_size;

  QPropertyAnimation *m_zoomAnimation;
  QPropertyAnimation *m_expancion;
  QPropertyAnimation *m_elementexpancion;

  QPropertyAnimation *m_transform;
  QPropertyAnimation *m_moving;

  QPropertyAnimation *m_opacityTreant;
  QPropertyAnimation *m_opacityClear;
  QPropertyAnimation *m_opacityEfect;

  QGraphicsOpacityEffect *opacityEffect;

  QPropertyAnimation *m_posAnimation;

  QPropertyAnimation *m_mogAnimation;

  fa::QtAwesome *awesome;

  CircularButton *treants;
  CircularButton *clear;
  QString m_text;

signals:
  void scaleSizeChanged(QSize point);
  void expancion(double number);
  void moving(double number);
  void pos(double number);

private slots:
  void animatePosition(const QPoint &startPos, const QPoint &endPos);
};

#endif // CIRLEBUTONEDITCONVERT_H
