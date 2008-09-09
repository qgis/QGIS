/***************************************************************************
                              qgsgrasstree.h
                             -------------------
    begin                : February, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSMODEL_H
#define QGSGRASSMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

class QgsGrassModelItem;


/*! \class QgsGrassModel
 *  \brief Model representing GRASS location structure.
 */
class QgsGrassModel: public QAbstractItemModel
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassModel( QObject * parent = 0 );
    //! Destructor
    ~QgsGrassModel();

    //! Item types
    enum ItemType { None, Gisbase, Location, Mapset, Rasters, Vectors, Raster,
                    Vector, VectorLayer, Regions, Region
                };

    //! Set GISBASE and LOCATION_NAME
    void setLocation( const QString &gisbase, const QString &location );

    // Refresh populated node
    void refresh();

    // Refresh item
    void refreshItem( QgsGrassModelItem *item ) ;

    // Remove items missing in the list
    void removeItems( QgsGrassModelItem *item, QStringList list ) ;

    // Add items missing in children
    void addItems( QgsGrassModelItem *item, QStringList list, int type ) ;

    //! Item type
    int itemType( const QModelIndex &index ) const;

    //! Item URI if it is a map
    QString uri( const QModelIndex &index ) const;

    // Index
    QModelIndex index( QgsGrassModelItem *item ) ;

    // Name
    QString itemName( const QModelIndex &index );

    // Item mapset (raster and vector)
    QString itemMapset( const QModelIndex &index );

    // Item map (raster and vector)
    QString itemMap( const QModelIndex &index );

    // Get info in HTML format
    QString itemInfo( const QModelIndex &index );

    // Reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column,
                       const QModelIndex & parent = QModelIndex() ) const;

    QModelIndex parent( const QModelIndex & index ) const;

    int rowCount( const QModelIndex & parent ) const;

    int columnCount( const QModelIndex & parent ) const;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;

  private:
    //! Current GISBASE
    QString mGisbase;

    //! Current LOCATION_NAME
    QString mLocation;

    //! Root node for current location
    QgsGrassModelItem *mRoot;

    //! Icons
    QIcon mIconDirectory;
    QIcon mIconFile;
    QIcon mIconRasterLayer;
    QIcon mIconVectorLayer;
    QIcon mIconPointLayer;
    QIcon mIconLineLayer;
    QIcon mIconPolygonLayer;
};

#endif // QGSGRASSMODEL_H
