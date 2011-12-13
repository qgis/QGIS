#ifndef QGSPOSTGRESDATAITEMS_H
#define QGSPOSTGRESDATAITEMS_H

#include "qgsdataitem.h"

#include "qgspostgresprovider.h"

class QgsPGConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions();

    QString mConnInfo;
    QMap<QString, QVector<QgsPostgresLayerProperty> > mSchemasMap;

  public slots:
    void editConnection();
    void deleteConnection();
};

// WMS Layers may be nested, so that they may be both QgsDataCollectionItem and QgsLayerItem
// We have to use QgsDataCollectionItem and support layer methods if necessary
class QgsPGLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsPGLayerItem( QgsDataItem* parent, QString name, QString path,
                    QString connInfo, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperties );
    ~QgsPGLayerItem();

    QString createUri();

    QString mConnInfo;
    QgsPostgresLayerProperty mLayerProperty;
};

class QgsPGSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGSchemaItem( QgsDataItem* parent, QString name, QString path,
                     QString connInfo );
    ~QgsPGSchemaItem();

    QVector<QgsDataItem*> createChildren();

  protected:
    QString mConnInfo;
};

class QgsPGRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QWidget * paramWidget();

    virtual QList<QAction*> actions();

  public slots:
    void connectionsChanged();
    void newConnection();
};


#endif // QGSPOSTGRESDATAITEMS_H
