#include "soundmanager.h"

SoundManager &SoundManager::instance() {
  static SoundManager inst;
  return inst;
}

SoundManager::SoundManager(QObject *parent) : QObject(parent) {}

QStringList SoundManager::supportedExtensions() const {
  return {"wav", "mp3", "ogg", "flac", "aac", "m4a", "wma"};
}

QList<SoundEntry> SoundManager::scanSounds() const {
  QDir dir("/home/Tiopaz/SoundsView/src/sounds");
  QList<SoundEntry> entries;
  if (!dir.exists()) return entries;

  QStringList filters;
  for (const QString &ext : supportedExtensions())
    filters << QString("*.%1").arg(ext);

  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
  for (const QFileInfo &fi : files) {
    SoundEntry e;
    e.name = fi.completeBaseName();
    e.filePath = fi.absoluteFilePath();
    entries.append(e);
  }
  return entries;
}
