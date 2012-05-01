#ifndef QGSOWSDATAITEMS_H
#define QGSOWSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
class QgsOWSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOWSConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions();

  public slots:
    void editConnection();
    void deleteConnection();
};

class QgsOWSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOWSRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QList<QAction*> actions();

    virtual QWidget * paramWidget();

  public slots:
    void connectionsChanged();

    void newConnection();
};

#endif // QGSOWSDATAITEMS_H
