#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QObject>
#include <QString>

class DataModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)

public:
    explicit DataModel(QObject *parent = nullptr);

    QString data() const;
    void setData(const QString &data);

signals:
    void dataChanged();

private:
    QString m_data;
};

#endif // DATAMODEL_H
