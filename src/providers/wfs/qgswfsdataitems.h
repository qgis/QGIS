#ifndef QGSWFSDATAITEMS_H
#define QGSWFSDATAITEMS_H

#include "qgsdataitem.h"

class QgsWFSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWFSRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWFSRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QWidget * paramWidget();

  public slots:
    void connectionsChanged();
};

class QgsWFSConnection;

class QgsWFSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWFSConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWFSConnectionItem();

    QVector<QgsDataItem*> createChildren();
    //virtual bool equal( const QgsDataItem *other );

  private slots:
    void gotCapabilities();

  private:
    QString mName;

    QgsWFSConnection* mConn;
    bool mGotCapabilities;
};


class QgsWFSLayerItem : public QgsLayerItem
{
  public:
    QgsWFSLayerItem( QgsDataItem* parent, QString connName, QString name, QString title );
    ~QgsWFSLayerItem();

};


#endif // QGSWFSDATAITEMS_H
