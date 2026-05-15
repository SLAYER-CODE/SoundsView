#include "voiceroulette.h"
#include <QApplication>
#include <QDebug>
#include <QSocketNotifier>
#include <csignal>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int sigFd[2];

static void sigHandler(int) {
    char c = 1;
    write(sigFd[0], &c, 1);
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  VoiceRoulette voiceRoulette;
  voiceRoulette.hide();

  socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd);
  QSocketNotifier *notifier = new QSocketNotifier(sigFd[1], QSocketNotifier::Read, &app);
  QObject::connect(notifier, &QSocketNotifier::activated, [&]() {
      char c;
      read(sigFd[1], &c, 1);
      if (voiceRoulette.isHidden())
          voiceRoulette.show();
      else
          voiceRoulette.hide();
  });

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigHandler;
  sigaction(SIGUSR1, &sa, nullptr);

  qDebug() << "SoundsView listo. pkill -USR1 SoundsView para toggle";
  return app.exec();
}
