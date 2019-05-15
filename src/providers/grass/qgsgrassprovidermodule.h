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

#ifdef HAVE_GUI
/* Qt does not allow inheritance from multiple QObjects, that is why we have QgsGrassItemActions
 * to keep common actions. QgsGrassItemActions must be children of data items, so that when a data item
 * is moved to to another thread, it moves also QgsGrassItemActions and signals work.
 * That is why each data item class keeps QgsGrassItemActions, instead of putting them to QgsGrassObjectItemBase,
 * because it would be ugly and dangerous to be parent of member's member. */
class QgsGrassItemActions : public QObject
{
    Q_OBJECT
  public:
    QgsGrassItemActions( const QgsGrassObject &grassObject, bool valid, QObject *parent );

    QList<QAction *> actions( QWidget *parent );

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
    void newLayer( const QString &type );
    QgsGrassObject mGrassObject;
    // Grass object is valid
    bool mValid;
};
#else
// just a forward declaration
class QgsGrassItemActions;
#endif

class QgsGrassObjectItemBase
{
  public:
    // actionsParent so that actions are moved to thread with item
    explicit QgsGrassObjectItemBase( const QgsGrassObject &grassObject );

    bool equal( const QgsDataItem *other );

  protected:
    QgsGrassObject mGrassObject;
};

class QgsGrassLocationItem : public QgsDirectoryItem, public QgsGrassObjectItemBase
{
    Q_OBJECT

  public:
    QgsGrassLocationItem( QgsDataItem *parent, QString dirPath, QString path );

    QIcon icon() override;

    QVector<QgsDataItem *> createChildren() override;
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override { return mActions->actions( parent ); }
#endif

  private:
    QgsGrassItemActions *mActions = nullptr;
};

class QgsGrassMapsetItem : public QgsDirectoryItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassMapsetItem( QgsDataItem *parent, QString dirPath, QString path );

    void setState( State state ) override;

    QIcon icon() override;

    QVector<QgsDataItem *> createChildren() override;
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override { return mActions->actions( parent ); }
#endif
    bool acceptDrop() override;
    bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

  public slots:
    void onImportFinished( QgsGrassImport *import );
    void onDirectoryChanged();
    void childrenCreated() override;

  private:
    bool objectInImports( const QgsGrassObject &grassObject );
    QgsGrassItemActions *mActions = nullptr;
    //void showImportError(const QString& error);
    QFileSystemWatcher *mMapsetFileSystemWatcher = nullptr;
    bool mRefreshLater;
    // running imports
    static QList<QgsGrassImport *> sImports;
};

class QgsGrassObjectItem : public QgsLayerItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassObjectItem( QgsDataItem *parent, QgsGrassObject grassObject,
                        QString name, QString path, QString uri,
                        LayerType layerType, QString providerKey );

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override { return mActions->actions( parent ); }
#endif
    bool equal( const QgsDataItem *other ) override;

  protected:
    QgsGrassItemActions *mActions = nullptr;

};

// Vector is collection of layers
class QgsGrassVectorItem : public QgsDataCollectionItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    // labelName - name to be displayed in tree if it should be different from grassObject.name() (e.g. invalid vector)
    QgsGrassVectorItem( QgsDataItem *parent, QgsGrassObject grassObject, QString path, QString labelName = QString(), bool valid = true );
    ~QgsGrassVectorItem() override;

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override { return mActions->actions( parent ); }
#endif
    bool equal( const QgsDataItem *other ) override;

  public slots:
    void onDirectoryChanged();

  private:
    bool mValid;
    QgsGrassItemActions *mActions = nullptr;
    QFileSystemWatcher *mWatcher = nullptr;
};

class QgsGrassVectorLayerItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassVectorLayerItem( QgsDataItem *parent, QgsGrassObject vector, QString layerName,
                             QString path, QString uri, LayerType layerType, bool singleLayer );

    QString layerName() const override;
    bool equal( const QgsDataItem *other ) override;

  private:
    // layer from single layer vector map (cannot have delete action)
    bool mSingleLayer;
};

class QgsGrassRasterItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassRasterItem( QgsDataItem *parent, QgsGrassObject grassObject,
                        QString path, QString uri, bool isExternal );

    QIcon icon() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    // is external created by r.external
    bool mExternal;
};

// Imagery group
class QgsGrassGroupItem : public QgsGrassObjectItem
{
    Q_OBJECT
  public:
    QgsGrassGroupItem( QgsDataItem *parent, QgsGrassObject grassObject,
                       QString path, QString uril );

    QIcon icon() override;

};

#ifdef HAVE_GUI
class QgsGrassImportItemWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsGrassImportItemWidget( QWidget *parent = nullptr );

    void setHtml( const QString &html );

  public slots:
    void onProgressChanged( const QString &recentHtml, const QString &allHtml, int min, int max, int value );

  private:
    QTextEdit *mTextEdit = nullptr;
    QProgressBar *mProgressBar = nullptr;
};
#endif

// item representing a layer being imported
class QgsGrassImportItem : public QgsDataItem, public QgsGrassObjectItemBase
{
    Q_OBJECT
  public:
    QgsGrassImportItem( QgsDataItem *parent, const QString &name, const QString &path, QgsGrassImport *import );
    ~QgsGrassImportItem() override;
    //void setState( State state ) override {
    //  QgsDataItem::setState(state);
    //} // do nothing to keep Populating
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
    QWidget *paramWidget() override;
#endif
    QIcon icon() override;

  public slots:
    void refresh() override {}
#ifdef HAVE_GUI
    void cancel();
#endif

  protected:
    // override refresh to keep Populating state
    void refresh( const QVector<QgsDataItem *> &children ) override { Q_UNUSED( children ) }
    //bool mDeleteAction;
    QgsGrassImport *mImport = nullptr;

  private:
    static QgsAnimatedIcon *sImportIcon;
};

#endif // QGSGRASSPROVIDERMODULE_H
