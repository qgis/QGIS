#ifndef QGSGDALDATAITEMS_H
#define QGSGDALDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
//#include "qgsowsconnection.h"
#include "qgswcscapabilities.h"

class QgsGdalLayerItem : public QgsLayerItem
{
  public:
    QgsGdalLayerItem( QgsDataItem* parent,
                      QString name, QString path, QString uri );
    ~QgsGdalLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs );
    Capability capabilities();
};

class QgsWCSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWCSConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWCSConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions();

    QgsWcsCapabilities mCapabilities;
    QVector<QgsWcsCoverageSummary> mLayerProperties;

  public slots:
    void editConnection();
    void deleteConnection();
};

// WCS Layers may be nested, so that they may be both QgsDataCollectionItem and QgsLayerItem
// We have to use QgsDataCollectionItem and support layer methods if necessary
class QgsWCSLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsWCSLayerItem( QgsDataItem* parent, QString name, QString path,
                     QgsWcsCapabilitiesProperty capabilitiesProperty, QgsDataSourceURI dataSourceUri, QgsWcsCoverageSummary coverageSummary );
    ~QgsWCSLayerItem();

    QString createUri();

    QgsWcsCapabilitiesProperty mCapabilities;
    QgsDataSourceURI mDataSourceUri;
    QgsWcsCoverageSummary mCoverageSummary;
};

class QgsWCSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWCSRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWCSRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QList<QAction*> actions();

    virtual QWidget * paramWidget();

  public slots:
    void connectionsChanged();

    void newConnection();
};

#endif // QGSGDALDATAITEMS_H
