/***************************************************************************
    qgsgrassprovidermodule.h  -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSPROVIDERMODULE_H
#define QGSGRASSPROVIDERMODULE_H

#include "qgsdataitem.h"
#include "qgsgrass.h"

class QgsGrassLocationItem : public QgsDirectoryItem
{
  public:
    QgsGrassLocationItem( QgsDataItem* parent, QString dirPath, QString path );

    QIcon icon() override { return QgsDataItem::icon(); }

    static bool isLocation( QString path );
    QVector<QgsDataItem*> createChildren() override;
};

class QgsGrassMapsetItem : public QgsDirectoryItem
{
  public:
    QgsGrassMapsetItem( QgsDataItem* parent, QString dirPath, QString path );

    QIcon icon() override { return QgsDataItem::icon(); }

    static bool isMapset( QString path );
    QVector<QgsDataItem*> createChildren() override;

    QString mLocation;
    QString mGisdbase;
};

class QgsGrassObjectItemBase
{
  public:
    QgsGrassObjectItemBase( QgsGrassObject grassObject );

  public:
    void deleteGrassObject( QgsDataItem* parent );

  protected:
    QgsGrassObject mGrassObject;
};

class QgsGrassObjectItem : public QgsLayerItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassObjectItem( QgsDataItem* parent, QgsGrassObject grassObject,
                        QString name, QString path, QString uri,
                        LayerType layerType, QString providerKey,
                        bool deleteAction = true );

    virtual QList<QAction*> actions() override;

  public slots:
    void deleteGrassObject();

  protected:
    //QgsGrassObject mGrassObject;
    bool mDeleteAction;
};

// Vector is collection of layers
class QgsGrassVectorItem : public QgsDataCollectionItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassVectorItem( QgsDataItem* parent, QgsGrassObject grassObject, QString path );
    ~QgsGrassVectorItem() {}

    virtual QList<QAction*> actions() override;

  public slots:
    void deleteGrassObject();

  private:
    QgsGrassObject mVector;
};

class QgsGrassVectorLayerItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassVectorLayerItem( QgsDataItem* parent, QgsGrassObject vector, QString layerName,
                             QString path, QString uri, LayerType layerType, bool singleLayer );

    QString layerName() const override;
    //virtual QList<QAction*> actions() override;

  public slots:
    //void deleteMap();

  private:
    // layer from single layer vector map (cannot have delete action)
    bool mSingleLayer;
};

class QgsGrassRasterItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassRasterItem( QgsDataItem* parent, QgsGrassObject grassObject,
                        QString path, QString uri );

    //virtual QList<QAction*> actions() override;

  public slots:
    //void deleteMap();
};

#endif // QGSGRASSPROVIDERMODULE_H
