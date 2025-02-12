#ifndef POLYGONBUTTON_H
#define POLYGONBUTTON_H

#include <QPushButton>
#include <QPolygonF>
#include <QColor>
#include <QPropertyAnimation>

class PolygonButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal gradientProgress READ gradientProgress WRITE setGradientProgress)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius)
    Q_PROPERTY(QColor gradientColorStart READ gradientColorStart WRITE setGradientColorStart)
    Q_PROPERTY(QColor gradientColorMiddle READ gradientColorMiddle WRITE setGradientColorMiddle)
    Q_PROPERTY(QColor gradientColorEnd READ gradientColorEnd WRITE setGradientColorEnd)
    Q_PROPERTY(qreal size READ size WRITE setSize)

public:
    explicit PolygonButton(const QString &text, qreal centerX_a, qreal centerY_a,qreal radius,qreal angle,qreal nextangle, QWidget *parent = nullptr);

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

    qreal angle() const;
    void setAngle(qreal angle);

    qreal nextangle() const;
    void setnextAngle(qreal nextangle);

    void setBackgroundColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setHoverBackgroundColor(const QColor &color);
    void setBorderColor(const QColor &color);
    void setHoverBorderColor(const QColor &color);
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

    int calculateFontSize(const QString& text, const QRectF& inscribedRect, int maxFontSize);
    void updatePolygon();

protected:
    void paintEvent(QPaintEvent *event) override;
    //void enterEvent(QEnterEvent *event) override;
    //void leaveEvent(QEvent *event) override;s
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
signals:
    void hoverEntered(int x, int y);  // Signal emitted when hover is detected, with x and y positions

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
    double m_rotationAngle; // Agregar esta variable

    QString m_text;
};

#endif // POLYGONBUTTON_H
