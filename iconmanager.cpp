#include "iconmanager.h"
#include "QObject"
#include "QApplication"

IconManager::IconManager(QObject *parent) : QObject(parent) {
    m_awesome = new fa::QtAwesome(qApp);
    m_awesome->initFontAwesome();
}
