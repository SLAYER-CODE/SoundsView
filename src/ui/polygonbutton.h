#ifndef POLYGONBUTTON_H
#define POLYGONBUTTON_H

#include <QColor>
#include <QPainterPath>
#include <QPolygonF>
#include <QPropertyAnimation>
#include <QPushButton>

class PolygonButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(
      qreal gradientProgress READ gradientProgress WRITE setGradientProgress)
  Q_PROPERTY(qreal radius READ radius WRITE setRadius)
  Q_PROPERTY(QColor gradientColorStart READ gradientColorStart WRITE
                 setGradientColorStart)
  Q_PROPERTY(QColor gradientColorMiddle READ gradientColorMiddle WRITE
                 setGradientColorMiddle)
  Q_PROPERTY(
      QColor gradientColorEnd READ gradientColorEnd WRITE setGradientColorEnd)
  Q_PROPERTY(qreal size READ size WRITE setSize)
  Q_PROPERTY(qreal startAngle READ angle WRITE setAngle)
  Q_PROPERTY(qreal endAngle READ nextangle WRITE setnextAngle)

public:
  explicit PolygonButton(const QString &text, qreal centerX_a, qreal centerY_a,
                         qreal radius, qreal angle, qreal nextangle,
                         QWidget *parent = nullptr,
                         const QChar &icon = QChar());
  void setIcon(const QChar &icon) { m_icon = icon; }

  void setText(const QString &text);
  QString text() const;

  void setPolygon(const QPolygonF &polygon);
  QPolygonF polygon() const;

  qreal radius() const;
  void setRadius(qreal radius);

  qreal centerX() const;
  void setCenterX(qreal centerX);

  qreal centerY() const;
  void setCenterY(qreal centerY);

  qreal angle() const { return m_angle; }
  void setAngle(qreal angle);

  qreal nextangle() const { return m_nextangle; }
  void setnextAngle(qreal nextangle);

  void setBackgroundColor(const QColor &color);
  void setTextColor(const QColor &color);
  void setHoverBackgroundColor(const QColor &color);
  void setBorderColor(const QColor &color);
  void setHoverBorderColor(const QColor &color);
  void setHighlightColor(const QColor &color) { m_highlightColor = color; }
  void setFocusColor(const QColor &c) { m_focusColor = c; }
  void setNonFocusColor(const QColor &c) { m_nonFocusColor = c; }
  void setInnerRadiusRatio(qreal r) { m_innerRadiusRatio = r; }
  void setFillOverride(const QColor &c) { m_fillOverride = c; }
  void setBluntCorners(bool b) { m_bluntCorners = b; }
  void setIconLeftTextRight(bool b) { m_iconLeftTextRight = b; }
  void setRotateWithAngle(bool b) { m_rotateWithAngle = b; }
  void setCustomFont(const QFont &f) { m_customFont = f; }
  void setRotationAngle(double angle); // Agregar este método

  qreal gradientProgress() const;
  void setGradientProgress(qreal progress);

  QColor gradientColorStart() const;
  void setGradientColorStart(const QColor &color);

  QColor gradientColorMiddle() const;
  void setGradientColorMiddle(const QColor &color);

  QColor gradientColorEnd() const;
  void setGradientColorEnd(const QColor &color);

  qreal size() const;
  void setSize(qreal size);
  void startSizeAnimation();
  void animateSizeTo(qreal target);
  void setVisualHighlight(bool highlighted);

  int calculateFontSize(const QString &text, const QRectF &inscribedRect,
                        int maxFontSize);
  void updatePolygon();

protected:
  void paintEvent(QPaintEvent *event) override;
  // void enterEvent(QEnterEvent *event) override;
  // void leaveEvent(QEvent *event) override;s
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
signals:
  void hoverEntered(
      int x,
      int y); // Signal emitted when hover is detected, with x and y positions

private:
  QFont m_font;
  QColor m_backgroundColor;
  QColor m_hoverBackgroundColor;
  QColor m_textColor;
  QColor m_borderColor;
  QColor m_hoverBorderColor;

  QColor m_gradientColorStart;
  QColor m_gradientColorMiddle;
  QColor m_gradientColorEnd;
  qreal m_gradientProgress;

  QPolygonF m_polygon;
  QPolygonF m_realpolygon;
  QPainterPath m_arcPath;
  QPainterPath m_innerArcPath;

  qreal rad;
  qreal m_radius;
  qreal m_centerX;
  qreal m_centerY;
  qreal m_angle;
  qreal m_nextangle;
  qreal m_size;

  QPropertyAnimation *m_sizeAnimation;
  QPropertyAnimation *m_animation;
  QPropertyAnimation *gradientAnimation;
  QPropertyAnimation *m_radiusAnimation;
  QPropertyAnimation *m_centerXAnimation;
  QPropertyAnimation *m_centerYAnimation;

  QRectF inscribedRect;
  int fontSize;

  bool m_isHovered;
  bool m_visualHighlighted = false;
  double m_rotationAngle;
  QColor m_highlightColor{Qt::white};
  QColor m_focusColor{QColor(180, 230, 255)};
  QColor m_nonFocusColor{QColor(200, 200, 200)};
  qreal m_innerRadiusRatio = 0.44;
  QColor m_fillOverride{};
  bool m_bluntCorners = false;
  bool m_iconLeftTextRight = false;
  bool m_rotateWithAngle = false;
  QFont m_customFont{};

  QString m_text;
  QChar m_icon;
};

#endif // POLYGONBUTTON_H
