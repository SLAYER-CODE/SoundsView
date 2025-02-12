#include "circularbutton.h"
#include <QPainter>
#include <QPushButton>
#include "iconmanager.h"

CircularButton::CircularButton(
    const QString &text, const QChar &icon, const QChar &iconCheck, qreal radius, QWidget *parent)
    : QPushButton(text, parent)
    , m_backgroundColor(Qt::black)
    , exp(1)
    , m_icon(icon)
    , m_iconUnchecked(iconCheck)
    , radius(radius)
    , m_zoom(1)
    , m_isHovered(false)
{
    setCheckable(true);
    m_expancion = new QPropertyAnimation(this, "expancion");
    m_expancion->setDuration(300);
    m_expancion->setStartValue(1);
    m_expancion->setEndValue(3.3);

    awesome = IconManager::instance().awesome();
}

void CircularButton::startAnimation()
{
    m_expancion->setDirection(QPropertyAnimation::Forward);
    m_expancion->start();
}

void CircularButton::resverseAnimation()
{
    m_expancion->setDirection(QPropertyAnimation::Backward);
    m_expancion->start();
}

void CircularButton::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void CircularButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    // Dibujar el círculo
    painter.setBrush(m_backgroundColor);

    if (m_isHovered) {
        painter.setBrush(m_hoverBackgroundColor);
    }

    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 2 * (radius * exp), 2 * (radius * exp));

    painter.restore();

    QPen pen(Qt::white, 1); // Ajusta el grosor del borde aquí
    painter.setPen(pen);

    painter.setBrush(Qt::NoBrush);

    QString iconText = m_icon; // Utilizar el ícono almacenado en la clase

    // Configurar el font para el ícono de FontAwesome
    QFont font = awesome->font(fa::fa_solid, 1 + radius); // Establecer el tamaño de fuente 16
    painter.setFont(font);
    // Calcular el rectángulo que encierra el círculo
    QRectF circleBoundingRect(0, 0, 2 * (radius * exp), 2 * (radius * exp));

    // Calcular el centro del círculo
    QPointF center = circleBoundingRect.center(); // Centro del círculo

    // Obtener el tamaño del ícono utilizando QFontMetrics
    QFontMetrics fm(font);
    QRectF iconBoundingRect = fm.boundingRect(iconText);

    // Calcular la posición superior izquierda para centrar el ícono
    QPointF iconTopLeft((center.x()) - iconBoundingRect.width() / 2,
                        (center.y()) - iconBoundingRect.height() / 2);
    // resize(0, 0);
    // Dibujar el ícono centrado dentro del círculo
    painter.drawText(QRectF(iconTopLeft, iconBoundingRect.size()), Qt::AlignCenter, iconText);
}

void CircularButton::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
}

void CircularButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
}

qreal CircularButton::expancion()
{
    return exp;
}
void CircularButton::setExpancion(qreal number)
{
    exp = number;
    setFixedSize(2 * radius * exp,
                 2 * radius * exp); // Ajustar el tamaño del widget dinámicamente
    updateMask();
    update();
}

void CircularButton::updateMask()
{
    QBitmap bitmap(size());
    bitmap.fill(Qt::color0);
    QPainter painter(&bitmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::color1);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 2 * (radius * exp), 2 * (radius * exp));
    setMask(bitmap);
}
