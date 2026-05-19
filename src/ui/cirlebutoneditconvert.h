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
  Q_PROPERTY(qreal progress READ progress WRITE setProgress)

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
  enum Side { None, Left, Right };
  void highlightLeft(bool h) { treants->setHighlight(h); if (h) m_highlightedSide = Left; else if (m_highlightedSide == Left) m_highlightedSide = None; }
  void highlightRight(bool h) { clear->setHighlight(h); if (h) m_highlightedSide = Right; else if (m_highlightedSide == Right) m_highlightedSide = None; }
  Side highlightedSide() const { return m_highlightedSide; }
  void triggerHighlighted();
  void setAltActive(bool active) { m_altActive = active; if (!active) { highlightLeft(false); highlightRight(false); } }
  void setInitialExpanded();
  void updateEmojiHighlight(int mouseX);

  static constexpr int kHysteresisPx = 15;
  enum class SendStatus { Neutral, Success, Failure };
  void setSendStatus(SendStatus status);
  void setNeutral() { setSendStatus(SendStatus::Neutral); }
  void markSent() { setSendStatus(SendStatus::Success); }
  void markFailed() { setSendStatus(SendStatus::Failure); }

  qreal progress() const { return m_progress; }
  void setProgress(qreal p);
  void animateProgress(qreal target);

protected:
  void paintEvent(QPaintEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

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

  QPropertyAnimation *m_progressAnim;
  qreal m_progress = 0.0;

  fa::QtAwesome *awesome;

  CircularButton *treants;
  CircularButton *clear;
  QString m_text;
  bool m_treantsActive = false;
  bool m_clearActive = false;
  bool m_altActive = false;
  Side m_highlightedSide = None;
  bool m_firstShow = true;

signals:
  void scaleSizeChanged(QSize point);
  void expancion(double number);
  void moving(double number);
  void pos(double number);
  void sendRequested(const QString &text);
  void clearRequested();

private slots:
  void animatePosition(const QPoint &startPos, const QPoint &endPos);
  void onSendClicked();
  void onClearClicked();
};

#endif // CIRLEBUTONEDITCONVERT_H
