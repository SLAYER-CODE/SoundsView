#ifndef CIRCULARTRIANGLEBUTTON_H
#define CIRCULARTRIANGLEBUTTON_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>

class CircularTriangleButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal radius READ radius WRITE setRadius)
  Q_PROPERTY(qreal zoom READ zoom WRITE setzoom)

public:
  explicit CircularTriangleButton(const QString &text, const QChar &icon,
                                  const QChar &iconCheck, qreal centerX_a,
                                  qreal centerY_a, qreal radius, qreal angle,
                                  qreal nextangle, qreal nextanglestep,
                                  QWidget *parent = nullptr);

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
  void setRotationAngle(double angle); // Agregar este método

  qreal zoom() const;
  void setzoom(const qreal &zo);

protected:
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

  void paintEvent(QPaintEvent *event) override;
  // void enterEvent(QEnterEvent *event) override;
  // void leaveEvent(QEvent *event) override;

private slots:
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

  qreal m_centerX;
  qreal m_centerY;
  qreal m_angle;
  qreal m_nextangle;

  QLabel *m_iconLabel; // QLabel para mostrar el ícono

  qreal m_zoom;
  QPolygonF m_polygon;

  QChar m_icon; // Icono de FontAwesome
  QChar m_iconchecked;
  QChar m_iconUnchecked;

  int m_iconSize;
  bool m_isHovered;
  double m_rotationAngle; // Agregar esta variable
};

#endif // CIRCULARTRIANGLEBUTTON_H
