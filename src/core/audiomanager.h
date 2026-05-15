#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QHash>
#include <QObject>
#include <QProcess>
#include <QStringList>

class AudioManager : public QObject {
  Q_OBJECT
public:
  static AudioManager &instance();

  void playSound(const QString &name);
  void setSoundPath(const QString &name, const QString &path);
  QString normalized(const QString &name) const { return name.trimmed(); }
  void stopSound(const QString &name);
  void stopAll();

private:
  AudioManager(QObject *parent = nullptr);
  QString audioTool() const;
  QHash<QString, QString> m_soundPaths;
  QList<QProcess *> m_activeProcesses;
};

#endif // AUDIOMANAGER_H
