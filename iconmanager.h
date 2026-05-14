#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include "QtAwesome.h"
#include <QObject>

class IconManager : public QObject {
  Q_OBJECT

public:
  static IconManager &instance() {
    static IconManager instance;
    return instance;
  }

  fa::QtAwesome *awesome() const { return m_awesome; }

private:
  IconManager(QObject *parent = nullptr);

  fa::QtAwesome *m_awesome;

  // Delete copy constructor and assignment operator to ensure singleton
  // property
  IconManager(const IconManager &) = delete;
  IconManager &operator=(const IconManager &) = delete;
};

#endif // ICONMANAGER_H
