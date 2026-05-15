#include "audiomanager.h"
#include <QDebug>
#include <QFileInfo>

AudioManager::AudioManager(QObject *parent) : QObject(parent) {}

AudioManager &AudioManager::instance() {
  static AudioManager inst;
  return inst;
}

QString AudioManager::audioTool() const {
  if (QFileInfo::exists("/usr/bin/paplay")) return "paplay";
  if (QFileInfo::exists("/usr/bin/pw-play")) return "pw-play";
  if (QFileInfo::exists("/usr/bin/ffplay"))
    return "ffplay -nodisp -autoexit";
  if (QFileInfo::exists("/usr/bin/aplay")) return "aplay";
  return QString();
}

void AudioManager::setSoundPath(const QString &name, const QString &path) {
  m_soundPaths[normalized(name)] = path;
}

void AudioManager::playSound(const QString &name) {
  QString key = normalized(name);
  if (!m_soundPaths.contains(key)) {
    qDebug() << "AudioManager: no path for" << name;
    return;
  }
  QString path = m_soundPaths[key];
  if (!QFileInfo::exists(path)) {
    qDebug() << "AudioManager: file not found" << path;
    return;
  }
  QString tool = audioTool();
  if (tool.isEmpty()) {
    qDebug() << "AudioManager: no audio tool found";
    return;
  }
  QProcess *proc = new QProcess(this);
  connect(proc, &QProcess::finished, this,
          [this, proc](int, QProcess::ExitStatus) {
            m_activeProcesses.removeOne(proc);
            proc->deleteLater();
          });
  QStringList args;
  if (tool.startsWith("ffplay")) {
    args = tool.split(' ') << path;
    proc->start(args.takeFirst(), args);
  } else {
    proc->start(tool, {path});
  }
  m_activeProcesses.append(proc);
}

void AudioManager::stopSound(const QString &name) {
  QString key = normalized(name);
  if (!m_soundPaths.contains(key)) return;
  for (auto it = m_activeProcesses.begin(); it != m_activeProcesses.end();) {
    QProcess *proc = *it;
    if (proc->program().contains("paplay") ||
        proc->arguments().contains(m_soundPaths[key])) {
      proc->kill();
      proc->waitForFinished(500);
      m_activeProcesses.erase(it);
      proc->deleteLater();
    } else {
      ++it;
    }
  }
}

void AudioManager::stopAll() {
  for (QProcess *proc : m_activeProcesses) {
    proc->kill();
    proc->waitForFinished(500);
    proc->deleteLater();
  }
  m_activeProcesses.clear();
}
