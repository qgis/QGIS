#ifndef QGSWFSCONNECTION_H
#define QGSWFSCONNECTION_H

#include <QObject>

class QgsWFSConnection : public QObject
{
    Q_OBJECT
public:
    explicit QgsWFSConnection(QObject *parent = 0);

  static QStringList connectionList();

  static void deleteConnection( QString name );

  static QString selectedConnection();
  static void setSelectedConnection( QString name );

signals:

public slots:

};

#endif // QGSWFSCONNECTION_H
