#ifndef CIRCULARTRIANGLEBUTTON_H
#define CIRCULARTRIANGLEBUTTON_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>

class CircularTriangleButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal radius READ radius WRITE setRadius)
  Q_PROPERTY(qreal zoom READ zoom WRITE setzoom)
  Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
  explicit CircularTriangleButton(const QString &text, const QChar &icon,
                                  const QChar &iconCheck, qreal centerX_a,
                                  qreal centerY_a, qreal radius, qreal angle,
                                  qreal nextangle, qreal nextanglestep,
                                  QWidget *parent = nullptr);

  void setText(const QString &text) { m_text = text; }
  QString text() const { return m_text; }

  void setPolygon(const QPolygonF &polygon);
  QPolygonF polygon() const;

  qreal radius() const;
  void setRadius(qreal radius);

  qreal centerX() const;
  void setCenterX(qreal centerX);

  qreal centerY() const;
  void setCenterY(qreal centerY);

  qreal angle() const;
  void setAngle(qreal angle);

  qreal nextangle() const;
  void setnextAngle(qreal nextangle);

  // colores y forma
  void setBackgroundColor(const QColor &color);
  QColor backgroundColor() const;

  void setTextColor(const QColor &color);
  void setHoverBackgroundColor(const QColor &color);
  void setBorderColor(const QColor &color);
  void setHoverBorderColor(const QColor &color);
  void setRotationAngle(double angle);

  qreal zoom() const;
  void setzoom(const qreal &zo);

  bool isChecked() const { return m_isChecked; }
  void setChecked(bool checked);
  void toggle() { setChecked(!m_isChecked); }
  void setVisualHighlight(bool highlighted);
  void setGrayedOut(bool grayed);
  void animateScale(qreal target);
  qreal scale() const { return m_scale; }
  void setScale(qreal s);

protected:
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

public slots:
  void updateIcon(bool checked);
signals:
  void hoverEntered(
      int x,
      int y); // Signal emitted when hover is detected, with x and y positions
  void hoverLeave();

private:
  QFont m_font;
  QColor m_backgroundColor;
  QColor m_hoverBackgroundColor;
  QColor m_textColor;
  QColor m_borderColor;
  QColor m_hoverBorderColor;

  qreal m_radius;
  QPropertyAnimation *m_zoomAnimation;
  QPropertyAnimation *m_radiusAnimation;
  QPropertyAnimation *m_scaleAnim;

  qreal m_centerX;
  qreal m_centerY;
  qreal m_angle;
  qreal m_nextangle;

  QLabel *m_iconLabel;

  qreal m_zoom;
  qreal m_scale = 1.0;
  QPolygonF m_polygon;
  QPolygonF m_originalPolygon;

  bool m_grayedOut = false;
  QColor m_savedBackgroundColor;
  QColor m_savedHoverBackgroundColor;

  QString m_text;
  QChar m_icon;
  QChar m_iconchecked;
  QChar m_iconUnchecked;

  int m_iconSize;
  bool m_isHovered;
  bool m_visualHighlighted = false;
  bool m_isChecked = false;
  double m_rotationAngle; // Agregar esta variable
};

#endif // CIRCULARTRIANGLEBUTTON_H
