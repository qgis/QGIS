//
// Created by myarjunar on 15/03/17.
//

#ifndef QGSGEONODEDATAITEMS_H
#define QGSGEONODEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsdatasourceuri.h"

#include <QMainWindow>

class QgsGeoNodeConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
//    ~QgsGeoNodeConnectionItem();
//
//    QVector<QgsDataItem *> createChildren() override;
//    virtual bool equal( const QgsDataItem *other ) override;
//
//    virtual QList<QAction *> actions() override;
//
//  public slots:
//    void editConnection();
//    void deleteConnection();
//    virtual void deleteLater() override;

  private:
    QString mUri;
};

class QgsGeoNodeRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoNodeRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    virtual QList<QAction *> actions() override;

  private slots:
    void newConnection();
};

//! Provider for Geonode root data item
class QgsGeoNodeDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "GeoNode" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override
    {
      if ( path.isEmpty() )
        return new QgsGeoNodeRootItem( parentItem, QStringLiteral( "GeoNode" ), QStringLiteral( "geonode:" ) );
      return nullptr;
    }
};

#endif //QGSGEONODEDATAITEMS_H