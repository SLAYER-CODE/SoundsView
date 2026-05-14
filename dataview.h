#ifndef DATAVIEW_H
#define DATAVIEW_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class DataView : public QWidget {
  Q_OBJECT

public:
  explicit DataView(QWidget *parent = nullptr);

  void updateView(const QString &data);

signals:
  void changeDataClicked();

private:
  QLabel *m_label;
  QPushButton *m_button;
};

#endif // DATAVIEW_H
