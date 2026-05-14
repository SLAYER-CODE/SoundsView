#include "datacontroller.h"

DataController::DataController(DataModel *model, DataView *view,
                               QObject *parent)
    : QObject(parent), m_model(model), m_view(view) {
  connect(m_model, &DataModel::dataChanged, this,
          &DataController::onDataChanged);
}

void DataController::setData(const QString &data) { m_model->setData(data); }

void DataController::onDataChanged() { m_view->updateView(m_model->data()); }
