/***************************************************************************
                 qgsdataitem.h  - Items representing data
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATAITEM_H
#define QGSDATAITEM_H

#include <QIcon>
#include <QLibrary>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QTreeWidget>

#include "qgsmaplayer.h"
#include "qgscoordinatereferencesystem.h"

class QgsDataProvider;
class QgsDataItem;

// TODO: bad place, where to put this? qgsproviderregistry.h?
typedef int dataCapabilities_t();
typedef QgsDataItem * dataItem_t( QString, QgsDataItem* );


/** base class for all items in the model */
class CORE_EXPORT QgsDataItem : public QObject
{
    Q_OBJECT
  public:
    enum Type
    {
      Collection,
      Directory,
      Layer,
    };

    QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path );
    virtual ~QgsDataItem() {}

    bool hasChildren();

    int rowCount();

    //

    virtual void refresh();

    // Create vector of children
    virtual QVector<QgsDataItem*> createChildren();

    // Populate children using children vector created by createChildren()
    virtual void populate();

    // Insert new child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals
    // refresh - refresh populated item, emit signals to model
    virtual void addChildItem( QgsDataItem * child, bool refresh = false );

    // remove and delete child item, signals to browser are emited
    virtual void deleteChildItem( QgsDataItem * child );

    virtual bool equal( const QgsDataItem *other );

    virtual QWidget * paramWidget() { return 0; }

    //

    enum Capability
    {
      NoCapabilities =          0,
      SetCrs         =          1 //Can set CRS on layer or group of layers
    };

    // This will _write_ selected crs in data source
    virtual bool setCrs( QgsCoordinateReferenceSystem crs )
    { Q_UNUSED( crs ); return false; }

    virtual Capability capabilities() { return NoCapabilities; }

    // static methods

    static QPixmap getThemePixmap( const QString theName );

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsDataItem*> items, QgsDataItem * item );

    // members

    Type type() const { return mType; }
    QgsDataItem* parent() const { return mParent; }
    QVector<QgsDataItem*> children() const { return mChildren; }
    QIcon icon() const { return mIcon; }
    QString name() const { return mName; }
    QString path() const { return mPath; }

  protected:

    Type mType;
    QgsDataItem* mParent;
    QVector<QgsDataItem*> mChildren; // easier to have it always
    bool mPopulated;
    QString mName;
    QString mPath; // it is also used to identify item in tree
    QIcon mIcon;

  public slots:
    void emitBeginInsertItems( QgsDataItem* parent, int first, int last );
    void emitEndInsertItems();
    void emitBeginRemoveItems( QgsDataItem* parent, int first, int last );
    void emitEndRemoveItems();

  signals:
    void beginInsertItems( QgsDataItem* parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem* parent, int first, int last );
    void endRemoveItems();
};

/** Item that represents a layer that can be opened with one of the providers */
class CORE_EXPORT QgsLayerItem : public QgsDataItem
{
    Q_OBJECT
  public:
    enum LayerType
    {
      NoType,
      Vector,
      Raster,
      Point,
      Line,
      Polygon,
      TableLayer,
      Database,
      Table
    };

    QgsLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType, QString providerKey );

    // --- reimplemented from QgsDataItem ---

    virtual bool equal( const QgsDataItem *other );

    // --- New virtual methods for layer item derived classes ---

    // Returns QgsMapLayer::LayerType
    QgsMapLayer::LayerType mapLayerType();

    // Returns layer uri or empty string if layer cannot be created
    QString uri() { return mUri; }

    // Returns provider key
    QString providerKey() { return mProviderKey; }

  protected:

    QString mProviderKey;
    QString mUri;
    LayerType mLayerType;

    static const QIcon &iconPoint();
    static const QIcon &iconLine();
    static const QIcon &iconPolygon();
    static const QIcon &iconTable();
    static const QIcon &iconDefault();
};


/** A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source? */
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
    Q_OBJECT
  public:
    QgsDataCollectionItem( QgsDataItem* parent, QString name, QString path = QString::null );
    ~QgsDataCollectionItem();

    void setPopulated() { mPopulated = true; }
    void addChild( QgsDataItem *item ) { mChildren.append( item ); }

    static const QIcon &iconDir(); // shared icon: open/closed directory
};

/** A directory: contains subdirectories and layers */
class CORE_EXPORT QgsDirectoryItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    enum Column
    {
      Name,
      Size,
      Date,
      Permissions,
      Owner,
      Group,
      Type
    };
    QgsDirectoryItem( QgsDataItem* parent, QString name, QString path );
    ~QgsDirectoryItem();

    QVector<QgsDataItem*> createChildren();

    virtual bool equal( const QgsDataItem *other );

    virtual QWidget * paramWidget();

    static QVector<QgsDataProvider*> mProviders;
    static QVector<QLibrary*> mLibraries;
};

class QgsDirectoryParamWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    QgsDirectoryParamWidget( QString path, QWidget* parent = NULL );

  protected:
    void mousePressEvent( QMouseEvent* event );

  public slots:
    void showHideColumn();
};

#endif // QGSDATAITEM_H

