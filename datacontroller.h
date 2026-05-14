#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include "datamodel.h"
#include "dataview.h"
#include <QObject>

class DataController : public QObject {
  Q_OBJECT

public:
  explicit DataController(DataModel *model, DataView *view,
                          QObject *parent = nullptr);

public slots:
  void setData(const QString &data);

private slots:
  void onDataChanged();

private:
  DataModel *m_model;
  DataView *m_view;
};

#endif // DATACONTROLLER_H
