#include "dataview.h"
#include <QPushButton>
#include <QVBoxLayout>

DataView::DataView(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  m_label = new QLabel("No data", this);
  m_button = new QPushButton("Change Data", this);

  layout->addWidget(m_label);
  layout->addWidget(m_button);

  connect(m_button, &QPushButton::clicked, this, &DataView::changeDataClicked);
}

void DataView::updateView(const QString &data) { m_label->setText(data); }
