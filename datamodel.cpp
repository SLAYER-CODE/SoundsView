#include "datamodel.h"

DataModel::DataModel(QObject *parent)
    : QObject(parent), m_data("Default Data") {}

QString DataModel::data() const { return m_data; }

void DataModel::setData(const QString &data) {
  if (m_data != data) {
    m_data = data;
    emit dataChanged();
  }
}
