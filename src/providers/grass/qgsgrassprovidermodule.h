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
#include "qgsgrassimport.h"

/** Class to share actions (we cannot inherit from multiple QObjects */
class QgsGrassItemActions : public QObject
{
    Q_OBJECT
  public:
    QgsGrassItemActions() {};

    QList<QAction*> actions();

    static QgsGrassItemActions* instance();

  public slots:
    void openOptions();
};

class QgsGrassImportItem;

class QgsGrassLocationItem : public QgsDirectoryItem
{
  public:
    QgsGrassLocationItem( QgsDataItem* parent, QString dirPath, QString path );

    QIcon icon() override { return QgsDataItem::icon(); }

    QVector<QgsDataItem*> createChildren() override;
    virtual QList<QAction*> actions() override;
};

class QgsGrassMapsetItem : public QgsDirectoryItem
{
    Q_OBJECT
  public:
    QgsGrassMapsetItem( QgsDataItem* parent, QString dirPath, QString path );

    QIcon icon() override { return QgsDataItem::icon(); }

    QVector<QgsDataItem*> createChildren() override;
    virtual QList<QAction*> actions() override;
    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) override;

  public slots:
    void onImportFinished( QgsGrassImport* import );

  private:
    //void showImportError(const QString& error);
    QString mLocation;
    QString mGisdbase;
    // running imports
    static QList<QgsGrassImport*> mImports;
};

class QgsGrassObjectItemBase
{
  public:
    QgsGrassObjectItemBase( QgsGrassObject grassObject );

  public:
    void renameGrassObject( QgsDataItem* parent );
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
                        bool showObjectActions = true );

    virtual QList<QAction*> actions() override;
    virtual bool equal( const QgsDataItem *other ) override;

  public slots:
    void renameGrassObject();
    void deleteGrassObject();

  protected:
    //QgsGrassObject mGrassObject;
    // indicates if it is really GRASS object like raster or vector map,
    // for example
    bool mShowObjectActions;
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
    void renameGrassObject();
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
    virtual bool equal( const QgsDataItem *other ) override;

  private:
    // layer from single layer vector map (cannot have delete action)
    bool mSingleLayer;
};

class QgsGrassRasterItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassRasterItem( QgsDataItem* parent, QgsGrassObject grassObject,
                        QString path, QString uri, bool isExternal );

    virtual QIcon icon() override;
    virtual bool equal( const QgsDataItem *other ) override;

  private:
    // is external created by r.external
    bool mExternal;
};

// Imagery group
class QgsGrassGroupItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassGroupItem( QgsDataItem* parent, QgsGrassObject grassObject,
                       QString path, QString uril );

    virtual QIcon icon() override;

};

// item representing a layer being imported
class QgsGrassImportItem : public QgsDataItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassImportItem( QgsDataItem* parent, const QString& name, const QString& path, QgsGrassImport* import );
    ~QgsGrassImportItem();
    //virtual void setState( State state ) override {
    //  QgsDataItem::setState(state);
    //} // do nothing to keep Populating
    virtual QList<QAction*> actions() override;
    virtual QIcon icon() override;
    // Init animated icon, to be called from main UI thread
    static void initIcon();

  public slots:
    virtual void refresh() override {}
    void cancel();

  protected:
    // override refresh to keep Populating state
    virtual void refresh( QVector<QgsDataItem*> children ) override { Q_UNUSED( children )};
    //bool mDeleteAction;
    QgsGrassImport* mImport;

  private:
    static QgsAnimatedIcon *mImportIcon;
};

#endif // QGSGRASSPROVIDERMODULE_H
