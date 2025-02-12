#include "voiceroulette.h"
#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtAwesome.h>
#include "circulartrianglebutton.h"
#include "cirlebutoneditconvert.h"
#include "iconmanager.h"
#include "polygonbutton.h"

#include <QRandomGenerator> // Incluir QRandomGenerator al principio
#include <algorithm> // Incluir algoritmo para std::shuffle
#include <cmath> // Incluir cmath para qDegreesToRadians

#include <QApplication>
#include <QCursor>
#include <QScreen>
#include "structdata.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

VoiceRoulette::VoiceRoulette(QWidget *parent)
    : QWidget(parent)
    , m_opacity(1.0)
    , m_hoverPos(0, 0)
    , m_restoreMouse(QCursor::pos())
    , m_menuSelect(false)
{
    //window()->showMaximized();
    //setFixedSize(800, 800);
    setMouseTracking(true); // Activar el seguimiento del mouse
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();
    this->setGeometry(screenGeometry);

    showFullScreen();
    setWindowTitle("Sounds Of Hears");
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
                   | Qt::WindowMaximizeButtonHint | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    //setAttribute(Qt::WA_NoSystemBackground, true);

    fa::QtAwesome *awesome = IconManager::instance().awesome();

    // Asegúrate de que el fondo del widget sea transparente
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    setPalette(pal);
    setStyleSheet("background: transparent;");

    QMap<QString, QPair<QChar, QChar>> iconMap
        = {{"Config", qMakePair(fa::fa_gear, fa::fa_gears)},
           {"Process Audio", qMakePair(fa::fa_bars, fa::fa_link)},
           {"Microphone", qMakePair(fa::fa_microphone, fa::fa_microphone_lines_slash)},
           {"Sound", qMakePair(fa::fa_headphones_simple, fa::fa_volume_xmark)}};

    QStringList Musics= {"python", "1", "Culebra python 1 ", "Culebra python 1 ", "Culebra python ", "Culebra python 1 ", "Culebra python 1 ", "Culebra pythinscribedRectinscribedRectinscribedRectpython 1Culebra python 11"};

    QMap<QString, QChar> configMap = {
        {"Profiles init on", fa::fa_youtube},
        {"Edit Youtube mix", fa::fa_youtube},
        {"Edit music and shorts", fa::fa_music},
        {"Edit Profiles", fa::fa_user_gear},
        {"Select Theme", fa::fa_palette},
        {"Select and configure Microphone", fa::fa_microphone_lines},
        {"Select output sound", fa::fa_play},
    };

    m_animation = new QPropertyAnimation(this, "windowOpacity");
    m_animation->setDuration(300); // Duración de la animación en milisegundos
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->start(QAbstractAnimation::DeleteWhenStopped);

    // Initialize the animation
    animation = new QPropertyAnimation(this, "hoverPos");
    animation->setDuration(200);  // Animation duration in milliseconds
    animation->setEasingCurve(QEasingCurve::OutCubic);

    animation_radius = new QPropertyAnimation(this, "radiusButons");
    animation_radius->setDuration(200); // Duración de la animación en milisegundos
    animation_radius->setStartValue(0);
    animation_radius->setEndValue(200);

    setupButtons(Musics);
    setupButtonsMenu(iconMap);
    setupButtonLoquendo();
}

void VoiceRoulette::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);

    // Draw a radial gradient centered at m_hoverPos
    QRadialGradient gradient(QPointF(width() / 2, height() / 2),
                             50 + m_radiusButons); // Radius of 50 pixels
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(0.5, Qt::black);
    gradient.setColorAt(1, Qt::transparent);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

void VoiceRoulette::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Obtener la pantalla en la que se encuentra la ventana
    QScreen *screen = QApplication::primaryScreen();

    QRect windowRect = this->geometry();
    QRect screenRect = screen->geometry();

    int x = (screenRect.width() - windowRect.width()) / 2;
    int y = (screenRect.height() - windowRect.height()) / 2;

    int centerX = x + windowRect.width() / 2;
    int centerY = y + windowRect.height() / 2;
    //Configure Mouse center

    //QCursor::setPos(centerX, centerY);
    //QCursor::pos();

    // Mover la ventana a la posición calculada
}

double VoiceRoulette::windowOpacity() const
{
    return m_opacity;
}

void VoiceRoulette::setWindowOpacity(double opacity)
{
    if (m_opacity != opacity) {
        m_opacity = opacity;
        QWidget::setWindowOpacity(opacity);
        emit windowOpacityChanged(opacity);
    }
}
void VoiceRoulette::setupButtonLoquendo()
{
    fa::QtAwesome *awesome = IconManager::instance().awesome();
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radiusMin = (qMin(800, 800)) / 17;
    m_buttonloquendo = new CirleButonEditConvert("Loquendo Message",
                                                 radiusMin,
                                                 centerX,
                                                 centerY,
                                                 fa::fa_comment_dots,
                                                 fa::fa_keyboard,
                                                 this);
    m_buttonloquendo->setToolTip("Habla atravez de texto con voz de loquendo");
    m_buttonloquendo->setStyleSheet("QToolTip {"
                                    "background-color: #00000055;" // Color de fondo
                                    "color: white;"                // Color del texto
                                    "border: 5px solid black;"     // Borde del tooltip
                                    "}");
    m_buttonloquendo->setGeometry(centerX - radiusMin, centerY - radiusMin, radiusMin, radiusMin);
    m_buttonloquendo->show();
}

void VoiceRoulette::setupButtons(QStringList list)
{
    double angleStep = 360.0 / list.size();
    int radius = 400;
    int centerX = width() / 2;

    int centerY = height() / 2;
    for (int i = 0; i < list.size(); ++i) {
        double angle = i * angleStep;
        double nextAngle = angle + angleStep;
        PolygonButton *button
            = new PolygonButton(list[i], centerX, centerY, radius, angle, nextAngle, this);
        button->setGeometry(0, 0, 2 * radius + centerX, 2 * radius + centerY);

        //connect(button, &PolygonButton::clicked, this, &VoiceRoulette::onButtonClicked);

        ButtonData *data = new ButtonData(button, angle, nextAngle);
        m_buttons.append(data);
        button->show();
    }
    QTimer::singleShot(0, this, &VoiceRoulette::startAnimations);
}

void VoiceRoulette::setupButtonsMenu(QMap<QString, QPair<QChar, QChar>> list)
{
    fa::QtAwesome* awesome = IconManager::instance().awesome();

    double angleStepMenu = 360.0 / list.size();
    int radiusMin = (qMin(800, 800)) / 5;
    double borderOffset = 10; // Ajusta este valor para cambiar el tamaño del borde
    double adjustedRadius = radiusMin - borderOffset;
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = (width() / 2);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int i = 0;
    for (auto it = list.begin(); it != list.end(); ++it, ++i) {
        double angle = i * angleStepMenu;
        double nextAngleStep = angle + angleStepMenu/2;
        double nextAngle = angle + angleStepMenu;
        double radians = qDegreesToRadians(angle);
        double nextRadians = qDegreesToRadians(nextAngle);

        double nextRadiansstep = qDegreesToRadians(nextAngleStep);

        QPointF center(centerX,centerY);
        QPointF p1(centerX + radiusMin * cos(radians), centerY + radiusMin * sin(radians));
        QPointF p2(centerX + radiusMin * cos(nextRadians), centerY + radiusMin* sin(nextRadians));
        QPointF p3(centerX + radiusMin * cos(nextRadiansstep ), centerY + radiusMin* sin(nextRadiansstep ));

        QPolygonF polygon;
        polygon << p1 << p3 << p2 << center ;

        CircularTriangleButton *button = new CircularTriangleButton(it.key(),it->first,it->second,centerX,centerY,adjustedRadius,angle,nextAngle,nextRadiansstep,this);
        button->setPolygon(polygon);
        button->setGeometry(0, 0, 2 * radius, 2 * radius);
        button->setToolTip(it.key());
        button->setStyleSheet(
            "QToolTip {"
            "background-color: #00000055;" // Color de fondo
            "color: white;"              // Color del texto
            "border: 5px solid black;"   // Borde del tooltip
            "}"
            );
        QSize iconSize(50, 50);  // Desired icon size

        ButtonDataMenu *data = new ButtonDataMenu(button, angle, nextAngle);
        m_buttonsMenu.append(data);

        //connect(button, &CircularTriangleButton::clicked, this, &VoiceRoulette::onButtonClicked);
        //connect(button, &CircularTriangleButton::hoverEntered, this, &VoiceRoulette::handleHover);
        //connect(button, &CircularTriangleButton::hoverLeave, this, &VoiceRoulette::handleLeave);

        button->show();
    }
}

void VoiceRoulette::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->hide(); // Ocultar el diálogo cuando se presiona Escape
        event->accept();
        this->menuSelect_change(false);
    } else {
        QWidget::keyPressEvent(event); // Manejar otros eventos de teclas normalmente
    }
}

void VoiceRoulette::mouseMoveEvent(QMouseEvent *event)
{
    QPoint center(width() / 2, height() / 2);

    // Calcula la distancia desde el centro al cursor
    QPoint cursorPos = event->pos();
    int dx = cursorPos.x() - m_restoreMouse.rx();
    int dy = cursorPos.y() - m_restoreMouse.ry();
    int distance = qSqrt(dx * dx + dy * dy);

    // Radio del círculo de restricción
    int radius = 100;

    // Si el cursor se mueve fuera del círculo, reposiciónalo en el borde del círculo
    if (distance >= radius) {
        double angleSelf = qAtan2(dy, dx);
        int newX = m_restoreMouse.rx() + int(radius * qCos(angleSelf));
        int newY = m_restoreMouse.ry() + int(radius * qSin(angleSelf));

        Display *display = XOpenDisplay(NULL);
        if (display) {
            Window root = DefaultRootWindow(display);
            XWarpPointer(display, None, root, 0, 0, 0, 0, newX, newY);
            XFlush(display);
            XCloseDisplay(display);
        }

        // Calcula las diferencias en los ejes X e Y
        int deltaX = cursorPos.x() - m_restoreMouse.x();
        int deltaY = cursorPos.y() - m_restoreMouse.y();

        double angle = qAtan2(deltaY, deltaX) * (180.0 / M_PI);
        if (angle < 0) {
            angle += 360; // Asegurarse de que el ángulo esté entre 0 y 360 grados
        }

        const double margin = 5.0; // Margen de error del 5%

        //qDebug() << m_menuSelect;
        if (m_menuSelect) {
            for (const ButtonDataMenu *data : m_buttonsMenu) {
                double startAngleWithMargin = data->startAngle + margin;
                double endAngleWithMargin = data->endAngle - margin;

                if (data->startAngle <= data->endAngle) {
                    if (angle >= startAngleWithMargin && angle <= endAngleWithMargin) {
                        data->button->setFocus();
                    } else if (angle < data->startAngle || angle > data->endAngle) {
                        data->button->clearFocus();
                    }
                } else { // Caso en que el ángulo cruza el 0/360 grados
                    if ((angle >= startAngleWithMargin && angle <= 360)
                        || (angle >= 0 && angle <= endAngleWithMargin)) {
                        data->button->setFocus();
                    } else if (angle < data->startAngle && angle > data->endAngle) {
                        data->button->clearFocus();
                    }
                }
            }
        } else {
            for (const ButtonData *data : m_buttons) {
                double startAngleWithMargin = data->startAngle + margin;
                double endAngleWithMargin = data->endAngle - margin;

                if (data->startAngle <= data->endAngle) {
                    if (angle >= startAngleWithMargin && angle <= endAngleWithMargin) {
                        data->button->setFocus();
                    } else if (angle < data->startAngle || angle > data->endAngle) {
                        data->button->clearFocus();
                    }
                } else { // Caso en que el ángulo cruza el 0/360 grados
                    if ((angle >= startAngleWithMargin && angle <= 360)
                        || (angle >= 0 && angle <= endAngleWithMargin)) {
                        data->button->setFocus();
                    } else if (angle < data->startAngle && angle > data->endAngle) {
                        data->button->clearFocus();
                    }
                }
            }
        }
    }

    // Ahora 'angleDegrees' contiene el ángulo en grados en relación con el centro del círculo
}

bool VoiceRoulette::menuSelect()
{
    return m_menuSelect;
}
void VoiceRoulette::menuSelect_change(const bool change)
{
    m_menuSelect = change;
}

QPoint VoiceRoulette::hoverPos() const
{
    return m_hoverPos;
}

void VoiceRoulette::setHoverPos(const QPoint &pos)
{
    m_hoverPos = pos;
    update();  // Trigger a repaint to reflect hover effect
}

qreal VoiceRoulette::radiusButons() const
{
    return m_radiusButons;
}

void VoiceRoulette::setradiusButons(const qreal &radius)
{
    m_radiusButons = radius;
    update(); // Trigger a repaint to reflect hover effect
}

QPointF VoiceRoulette::m_mouseRestore() const
{
    return m_restoreMouse;
}

void VoiceRoulette::m_setMRestore(const QPointF &point)
{
    m_restoreMouse = point;
}

void VoiceRoulette::handleHover(int x, int y)
{
    isHovering=true;
    //animation->stop();
    //animation->setStartValue(m_hoverPos);
    //animation->setEndValue(QPoint(x, y));
    //animation->start();

    animation_radius->setDirection(QPropertyAnimation::Forward);
    animation_radius->start();
}

void VoiceRoulette::handleLeave()
{
    isHovering = false;
    animation_radius->setDirection(QPropertyAnimation::Backward);
    animation_radius->start();
}

void VoiceRoulette::startAnimations() {
    QList<int> indices;
    for (int i = 0; i < m_buttons.size(); ++i) {
        indices.append(i);
    }

    auto rng = std::default_random_engine{QRandomGenerator::global()->generate()};
    std::shuffle(indices.begin(), indices.end(), rng);

    for (int i = 0; i < indices.size(); ++i) {
        int delay = QRandomGenerator::global()->bounded(0, 50); // Genera un retraso aleatorio entre 50 y 1000 milisegundos
        QTimer::singleShot(i * delay, this, [=]() {
            m_buttons[indices[i]]->button->startSizeAnimation();
        });
    }
}


void VoiceRoulette::onButtonClickedMenu() {
    CircularTriangleButton *button = qobject_cast<CircularTriangleButton *>(sender());
    if (button) {
    }
}

void VoiceRoulette::onButtonClicked() {
    PolygonButton *button = qobject_cast<PolygonButton *>(sender());
    if (button) {
        QString name = button->text();
        qDebug() << "Button clicked:" << name;
    }
}


void VoiceRoulette::onButtonClickedWithName() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        QString name = button->text();
        qDebug() << "Button clicked:" << name;
    }
}
