#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QStringList>

struct SoundEntry {
  QString name;
  QString filePath;
};

class SoundManager : public QObject {
  Q_OBJECT
public:
  static SoundManager &instance();

  QList<SoundEntry> scanSounds() const;
  QStringList scanSoundLists() const;
  QList<SoundEntry> scanSoundsInFolder(const QString &folderName) const;
  QStringList supportedExtensions() const;

private:
  SoundManager(QObject *parent = nullptr);
};

#endif // SOUNDMANAGER_H
