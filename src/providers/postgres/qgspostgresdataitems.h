#ifndef QGSPOSTGRESDATAITEMS_H
#define QGSPOSTGRESDATAITEMS_H

#include "qgsdataitem.h"

#include "qgspostgresconn.h"
#include "qgspgsourceselect.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerimport.h"

class QgsPGRootItem;
class QgsPGConnectionItem;
class QgsPGSchemaItem;
class QgsPGLayerItem;

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

class QgsPGConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );
    virtual QList<QAction*> actions();

    virtual bool acceptDrop() { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action );

    QgsPostgresConn *connection() const { return mConn; }

  signals:
    void addGeometryColumn( QgsPostgresLayerProperty );

  public slots:
    void editConnection();
    void deleteConnection();

    void setLayerType( QgsPostgresLayerProperty layerProperty );

  private:
    QgsPostgresConn *mConn;
    QMap<QString, QgsPGSchemaItem * > mSchemaMap;
};

class QgsPGSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGSchemaItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGSchemaItem();

    QVector<QgsDataItem*> createChildren();

    void addLayer( QgsPostgresLayerProperty layerProperty );
};

class QgsPGLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsPGLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperties );
    ~QgsPGLayerItem();

    QString createUri();

    virtual QList<QAction*> actions();

  public slots:
    void deleteLayer();

  private:
    QgsPostgresLayerProperty mLayerProperty;
};

#endif // QGSPOSTGRESDATAITEMS_H
