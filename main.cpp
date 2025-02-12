#include <QApplication>
#include <QEnterEvent>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QWidget>
#include <QtMath>
#include "voiceroulette.h"

#include <QApplication>
#include <QKeySequence>
#include <QShortcut>
#include <qhotkey.h>

#include <QtGui/private/qtx11extras_p.h>

#include <QTextToSpeech>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <thread>

using namespace Qt;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VoiceRoulette voiceRoulette;
    //voiceRoulette.showMaximized();

    // Verificar si el sistema soporta íconos en la bandeja
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "System tray is not available!";
        return -1;
    }

    // Crear el ícono de la bandeja del sistema
    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(QIcon(":/icons/icon.png"));
    trayIcon.setToolTip("Mi aplicación en segundo plano");

    // Crear un menú para el ícono de la bandeja
    QMenu trayMenu;
    QAction quitAction("Salir", &app);
    QObject::connect(&quitAction, &QAction::triggered, &app, &QApplication::quit);
    trayMenu.addAction(&quitAction);

    // Asignar el menú al ícono de la bandeja
    trayIcon.setContextMenu(&trayMenu);

    // Conectar la señal activada del icono a una función que muestre el dialogo
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [&]() {
        if (voiceRoulette.isHidden()) {
            voiceRoulette.show(); // Mostrar el QDialog si está oculto
            voiceRoulette.m_setMRestore(QCursor::pos());
        } else {
            voiceRoulette.raise(); // Traer el QDialog al frente si ya está visible
            voiceRoulette.activateWindow();
        }
    });

    // Mostrar el ícono en la bandeja
    trayIcon.show();

    QHotkey manager(ModifierKey::Non, XK_F8);
    QObject::connect(&manager, &QHotkey::pressed, [&voiceRoulette]() {
        voiceRoulette.menuSelect_change(true);
        if (!voiceRoulette.isVisible()) {
            voiceRoulette.show(); // Mostrar el diálogo cuando se presiona la combinación de teclas
            voiceRoulette.m_setMRestore(QCursor::pos());

            QTextToSpeech *speech = new QTextToSpeech();
            speech->say("hola como estan todos soy vegeta setecientos noveitaiciete y estamos en "
                        "un nuevo video sabor ochi");
        } else {
            voiceRoulette.activateWindow(); // Traerlo al frente si ya está visible
        }
    });

    QHotkey managerMenu(ModifierKey::Control, XK_F8);
    QObject::connect(&managerMenu, &QHotkey::pressed, [&voiceRoulette]() {
        voiceRoulette.menuSelect_change(false);
        if (!voiceRoulette.isVisible()) {
            voiceRoulette.show(); // Mostrar el diálogo cuando se presiona la combinación de teclas
            voiceRoulette.m_setMRestore(QCursor::pos());
        } else {
            voiceRoulette.activateWindow(); // Traerlo al frente si ya está visible
        }
    });

    return app.exec();
}

#include "main.moc"
