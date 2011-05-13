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
/* $Id$ */
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
      Vector,
      Raster,
      Point,
      Line,
      Polygon,
      TableLayer,
      Database,
      Table
    };
    enum Capability
    {
      NoCapabilities =          0,
      SetCrs         =          1 //Can set CRS on layer or group of layers
    };

    QgsDataItem(QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path);
    virtual ~QgsDataItem() {}

    virtual bool hasChildren();

    virtual int rowCount();

    virtual QIcon icon ();

    void setParent ( QgsDataItem *parent ) { mParent = parent; }

    QPixmap getThemePixmap( const QString theName );

    // Sets info about layer which can be created for this item
    // returns true if layer can be created
    virtual bool layerInfo ( QgsMapLayer::LayerType & type, 
      QString & providerKey, QString & uri ) { return false; } 

    virtual void refresh();

    // This will _write_ selected crs in data source
    virtual bool setCrs ( QgsCoordinateReferenceSystem crs ) { return false; }

    virtual Capability capabilities() { return NoCapabilities; }

    // Create vector of children
    virtual QVector<QgsDataItem*> createChildren();

    // Populate children using children vector created by createChildren()
    virtual void populate();

    // Insert new child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals
    // refresh - refresh populated item, emit signals to model
    virtual void addChildItem ( QgsDataItem * child, bool refresh = false );

    // remove and delete child item, signals to browser are emited
    virtual void deleteChildItem ( QgsDataItem * child );

    // Find child index in vector of items using '==' operator
    int findItem ( QVector<QgsDataItem*> items, QgsDataItem * item );

    Type mType;
    QgsDataItem* mParent;
    QVector<QgsDataItem*> mChildren; // easier to have it always
    bool mPopulated;
    QString mName;
    // Are the data shared? Cannot be static because each child has different.
    QIcon mDefaultIcon; 
    QIcon mIcon;
    QString mPath; // it is also used to identify item in tree

    virtual bool equal(const QgsDataItem *other)  { return false; }

    virtual QWidget * paramWidget() { return 0; }

  public slots:
    virtual void doubleClick();
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
public:
  QgsLayerItem(QgsDataItem* parent, QString name, QString path);
  QgsLayerItem(QgsDataItem* parent, QgsDataItem::Type type, QString name, QString path, QString uri);

  virtual QIcon icon ();

  virtual bool equal(const QgsDataItem *other);

  QString mProvider;
  QString mUri;
};


/** A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source? */
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
public:
  QgsDataCollectionItem( QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path =0);
  ~QgsDataCollectionItem();

  void setPopulated() { mPopulated = true; }
  void addChild( QgsDataItem *item ) { mChildren.append(item); }
};

/** A directory: contains subdirectories and layers */
class CORE_EXPORT QgsDirectoryItem : public QgsDataCollectionItem
{
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
  QgsDirectoryItem(QgsDataItem* parent, QString name, QString path);
  ~QgsDirectoryItem();

  QVector<QgsDataItem*> createChildren();

  virtual bool equal(const QgsDataItem *other);

  virtual QWidget * paramWidget();

  static QVector<QgsDataProvider*> mProviders;
  static QVector<QLibrary*> mLibraries;
};

class QgsDirectoryParamWidget : public QTreeWidget
{
  Q_OBJECT

public:
  QgsDirectoryParamWidget(QString path, QWidget* parent = NULL);

protected:
  void mousePressEvent(QMouseEvent* event);

public slots:
  void showHideColumn();
};

#endif // QGSDATAITEM_H

