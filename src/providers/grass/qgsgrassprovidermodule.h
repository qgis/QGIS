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

class QTextEdit;
class QProgressBar;

class QgsGrassImportItem;

/* Qt does not allow inheritance from multiple QObjects, that is why we have QgsGrassItemActions
 * to keep common actions. QgsGrassItemActions must be childern of data items, so that when a data item
 * is moved to to another thread, it moves also QgsGrassItemActions and signals work.
 * That is why each data item class keeps QgsGrassItemActions, instead of putting them to QgsGrassObjectItemBase,
 * because it would be ugly and dangerous to be parent of member's member. */
class QgsGrassItemActions : public QObject
{
    Q_OBJECT
  public:
    QgsGrassItemActions( QgsGrassObject grassObject, bool valid, QObject *parent );

    QList<QAction*> actions();

  public slots:
    void newMapset();
    void openMapset();
    void addMapsetToSearchPath();
    void removeMapsetFromSearchPath();
    void renameGrassObject();
    void deleteGrassObject();
    void newPointLayer();
    void newLineLayer();
    void newPolygonLayer();

  private:
    // returns name of new vector map or empty string
    QString newVectorMap();
    void newLayer( QString type );
    QgsGrassObject mGrassObject;
    // Grass object is valid
    bool mValid;
};

class QgsGrassObjectItemBase
{
  public:
    // actionsParent so that actions are moved to thread with item
    explicit QgsGrassObjectItemBase( QgsGrassObject grassObject );

    bool equal( const QgsDataItem *other );

  protected:
    QgsGrassObject mGrassObject;
};

class QgsGrassLocationItem : public QgsDirectoryItem, public QgsGrassObjectItemBase
{
  public:
    QgsGrassLocationItem( QgsDataItem* parent, QString dirPath, QString path );

    QIcon icon() override { return QgsDataItem::icon(); }

    QVector<QgsDataItem*> createChildren() override;
    virtual QList<QAction*> actions() override { return mActions->actions(); }

  private:
    QgsGrassItemActions *mActions;
};

class QgsGrassMapsetItem : public QgsDirectoryItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassMapsetItem( QgsDataItem* parent, QString dirPath, QString path );

    virtual void setState( State state ) override;

    QIcon icon() override;

    QVector<QgsDataItem*> createChildren() override;
    virtual QList<QAction*> actions() override { return mActions->actions(); }
    virtual bool acceptDrop() override;
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) override;

  public slots:
    void onImportFinished( QgsGrassImport* import );
    void onDirectoryChanged();
    virtual void childrenCreated() override;

  private:
    bool objectInImports( QgsGrassObject grassObject );
    QgsGrassItemActions *mActions;
    //void showImportError(const QString& error);
    QFileSystemWatcher *mMapsetFileSystemWatcher;
    bool mRefreshLater;
    // running imports
    static QList<QgsGrassImport*> mImports;
};

class QgsGrassObjectItem : public QgsLayerItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassObjectItem( QgsDataItem* parent, QgsGrassObject grassObject,
                        QString name, QString path, QString uri,
                        LayerType layerType, QString providerKey );

    virtual QList<QAction*> actions() override { return mActions->actions(); }
    virtual bool equal( const QgsDataItem *other ) override;

  protected:
    QgsGrassItemActions *mActions;

};

// Vector is collection of layers
class QgsGrassVectorItem : public QgsDataCollectionItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    // labelName - name to be displayed in tree if it should be different from grassObject.name() (e.g. invalid vector)
    QgsGrassVectorItem( QgsDataItem* parent, QgsGrassObject grassObject, QString path, QString labelName = QString::null, bool valid = true );
    ~QgsGrassVectorItem();

    virtual QList<QAction*> actions() override { return mActions->actions(); }
    virtual bool equal( const QgsDataItem *other ) override;

  public slots:
    void onDirectoryChanged();

  private:
    bool mValid;
    QgsGrassItemActions *mActions;
    QFileSystemWatcher *mWatcher;
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

class QgsGrassImportItemWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsGrassImportItemWidget( QWidget* parent = 0 );

    void setHtml( const QString & html );

  public slots:
    void onProgressChanged( const QString &recentHtml, const QString &allHtml, int min, int max, int value );

  private:
    QTextEdit *mTextEdit;
    QProgressBar *mProgressBar;
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
    virtual QWidget *paramWidget() override;

  public slots:
    virtual void refresh() override {}
    void cancel();

  protected:
    // override refresh to keep Populating state
    virtual void refresh( QVector<QgsDataItem*> children ) override { Q_UNUSED( children ); }
    //bool mDeleteAction;
    QgsGrassImport* mImport;

  private:
    static QgsAnimatedIcon *mImportIcon;
};

#endif // QGSGRASSPROVIDERMODULE_H
