/*******************************************************************
                              qgsgrasstree.cpp
                             -------------------
    begin                : February, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
********************************************************************/
/********************************************************************
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
*******************************************************************/

#include "qgsgrassmodel.h"
#include "qgsgrassselect.h"
#include "qgsgrassprovider.h"
#include "qgsgrass.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsgrassplugin.h"

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/gis.h>
#include <grass/vector.h>
#include <grass/raster.h>
#define G_get_cellhd(name,mapset,cellhd) (Rast_get_cellhd(name,mapset,cellhd),0)
#define G_raster_map_type Rast_map_type
#define G_read_fp_range Rast_read_fp_range
#define G_get_fp_range_min_max Rast_get_fp_range_min_max
#define G_read_history Rast_read_history
#define G_read_cats Rast_read_cats
#define G_free_cats Rast_free_cats
#define datsrc_1 fields[HIST_DATSRC_1]
#define datsrc_2 fields[HIST_DATSRC_2]
#define keywrd fields[HIST_KEYWRD]
#define edlinecnt nlines
#define edhist lines
#endif
}


/*
 * Internal data structure starts (at present) with LOCATION
 * as top level element, that is root but it does not appear
 * in the tree view. First elements shown in the view are mapsets
 */

class QgsGrassModelItem
{
  public:
    QgsGrassModelItem( QgsGrassModelItem *parent, int row, QString name, QString path, int type );
    QgsGrassModelItem();
    ~QgsGrassModelItem();

    // Copy mGisbase, mLocation, mMapset, mMap, mLayer
    void copyNames( QgsGrassModelItem *item );

    void populate();
    bool populated() { return mPopulated; }

    int type() { return mType; }

    // Map URI
    QString uri();

    QgsGrassModelItem *child( int i );
    QgsGrassModelItem *mParent;
    QVariant data( int role = Qt::DisplayRole );
    QString name();
    QString info();
    QString htmlTableRow( QString s1, QString s2 );
    QString htmlTableRow( QStringList  list );

    int mType;

    QString mGisbase;
    QString mLocation;
    QString mMapset;
    QString mMap;
    QString mLayer;

    QVector<QgsGrassModelItem*> mChildren;
    bool mPopulated;

    QgsGrassModel *mModel;
};

QgsGrassModelItem::QgsGrassModelItem()
    : mParent( 0 )
    , mType( QgsGrassModel::None )
    , mPopulated( false )
    , mModel( 0 )
{
}

QgsGrassModelItem::~QgsGrassModelItem()
{
  for ( int i = 0; i < mChildren.size(); i++ )
  {
    delete mChildren[i];
  }
  mChildren.clear();
}

void QgsGrassModelItem::copyNames( QgsGrassModelItem *item )
{
  mModel = item->mModel;
  mGisbase = item->mGisbase;
  mLocation = item->mLocation;
  mMapset = item->mMapset;
  mMap = item->mMap;
  mLayer = item->mLayer;
}

QVariant QgsGrassModelItem::data( int role )
{
  if ( role != Qt::DisplayRole )
    return QVariant();

  return name();
}

QString QgsGrassModelItem::info()
{
  QString tblStart = "<table border=1 cellspacing=1 cellpadding=1>";
  switch ( mType )
  {
    case QgsGrassModel::Location:
      return QObject::tr( "Location: %1" ).arg( mLocation );
      break;
    case QgsGrassModel::Mapset:
      return QObject::tr( "Location: %1<br>Mapset: %2" ).arg( mLocation ).arg( mMapset );
      break;
    case QgsGrassModel::Vectors:
    case QgsGrassModel::Rasters:
    case QgsGrassModel::Regions:
      return QObject::tr( "Location: %1<br>Mapset: %2" ).arg( mLocation ).arg( mMapset );
      break;
    case QgsGrassModel::Raster:
    {
      QString str = tblStart;
      str += htmlTableRow( QObject::tr( "<b>Raster</b>" ), QString( "<b>%1</b>" ).arg( mMap ) );

      struct Cell_head head;
      int rasterType = -1;
      QgsGrass::setLocation( mGisbase, mLocation );

      if ( G_get_cellhd( mMap.toLocal8Bit().data(),
                         mMapset.toLocal8Bit().data(), &head ) != 0 )
      {
        str += "<tr><td colspan=2>" + QObject::tr( "Cannot open raster header" ) + "</td></tr>";
      }
      else
      {
        str += htmlTableRow( QObject::tr( "Rows" ), QString::number( head.rows ) );
        str += htmlTableRow( QObject::tr( "Columns" ), QString::number( head.cols ) );
        str += htmlTableRow( QObject::tr( "N-S resolution" ), QString::number( head.ns_res ) );
        str += htmlTableRow( QObject::tr( "E-W resolution" ), QString::number( head.ew_res ) );
        str += htmlTableRow( QObject::tr( "North" ), QString::number( head.north ) );
        str += htmlTableRow( QObject::tr( "South" ), QString::number( head.south ) );
        str += htmlTableRow( QObject::tr( "East" ), QString::number( head.east ) );
        str += htmlTableRow( QObject::tr( "West" ), QString::number( head.west ) );

        rasterType = G_raster_map_type( mMap.toLocal8Bit().data(),
                                        mMapset.toLocal8Bit().data() );

        QString format;
        if ( rasterType == CELL_TYPE )
        {
          format = "integer (" + QString::number( head.format );
          format += head.format == 0 ? " byte)" : "bytes)";
        }
        else if ( rasterType == FCELL_TYPE )
        {
          format += "floating point (4 bytes)";
        }
        else if ( rasterType == DCELL_TYPE )
        {
          format += "floating point (8 bytes)";
        }
        else
        {
          format += "unknown";
        }
        str += htmlTableRow( QObject::tr( "Format" ), format );
      }

      // Range of values
      struct FPRange range;
      if ( G_read_fp_range( mMap.toLocal8Bit().data(),
                            mMapset.toLocal8Bit().data(), &range ) != -1 )
      {
        double min, max;
        G_get_fp_range_min_max( &range, &min, &max );

        str += htmlTableRow( QObject::tr( "Minimum value" ), QString::number( min ) );
        str += htmlTableRow( QObject::tr( "Maximum value" ), QString::number( max ) );
      }

      // History
      struct History hist;
      if ( G_read_history( mMap.toLocal8Bit().data(),
                           mMapset.toLocal8Bit().data(), &hist ) >= 0 )
      {
        if ( QString( hist.datsrc_1 ).length() > 0
             || QString( hist.datsrc_2 ).length() > 0 )
        {
          str += htmlTableRow( QObject::tr( "Data source" ),
                               QString( "%1 %2" ).arg( hist.datsrc_1 ).arg( hist.datsrc_2 ) );
        }
        if ( QString( hist.keywrd ).length() > 0 )
        {
          str += htmlTableRow( QObject::tr( "Data description" ), QString( hist.keywrd ) );
        }
        if ( hist.edlinecnt > 0 )
        {
          QString h;
          for ( int i = 0; i < hist.edlinecnt; i++ )
          {
            h += QString( hist.edhist[i] ) + "<br>";
          }
          str += htmlTableRow( QObject::tr( "Comments" ), h );
        }
      }

      // Categories
      if ( rasterType == CELL_TYPE )
      {
        struct Categories Cats;
        int ret = G_read_cats( mMap.toLocal8Bit().data(),
                               mMapset.toLocal8Bit().data(), &Cats );

        if ( ret == 0 )
        {
          if ( Cats.ncats > 0 )
          {
            str += "<tr><td colspan=2>" + QObject::tr( "Categories" ) + "</td></tr>";
            for ( int i = 0; i < Cats.ncats; i++ )
            {
              str += htmlTableRow(
                       QString::number(( int )Cats.q.table[i].dLow ),
                       QString( Cats.labels[i] ) );
            }
          }
          G_free_cats( &Cats );
        }
      }
      str += "</table>";

      return str;
    }
    break;

    case QgsGrassModel::Vector:
    {
      QString str = tblStart;
      str += htmlTableRow( QObject::tr( "<b>Vector</b>" ), QString( "<b>%1</b>" ).arg( mMap ) );

      QgsGrass::setLocation( mGisbase, mLocation );

      try
      {
        struct Map_info Map;
        int level = Vect_open_old_head( &Map, mMap.toLocal8Bit().data(),
                                        mMapset.toLocal8Bit().data() );

        if ( level >= 2 )
        {
          int is3d = Vect_is_3d( &Map );

          // Number of elements
          str += htmlTableRow( QObject::tr( "Points" ), QString::number( Vect_get_num_primitives( &Map, GV_POINT ) ) );
          str += htmlTableRow( QObject::tr( "Lines" ), QString::number( Vect_get_num_primitives( &Map, GV_LINE ) ) );
          str += htmlTableRow( QObject::tr( "Boundaries" ), QString::number( Vect_get_num_primitives( &Map, GV_BOUNDARY ) ) );
          str += htmlTableRow( QObject::tr( "Centroids" ), QString::number( Vect_get_num_primitives( &Map, GV_CENTROID ) ) );
          if ( is3d )
          {
            str += htmlTableRow( QObject::tr( "Faces" ), QString::number( Vect_get_num_primitives( &Map, GV_FACE ) ) );
            str += htmlTableRow( QObject::tr( "Kernels" ), QString::number( Vect_get_num_primitives( &Map, GV_KERNEL ) ) );
          }

          str += htmlTableRow( QObject::tr( "Areas" ), QString::number( Vect_get_num_areas( &Map ) ) );
          str += htmlTableRow( QObject::tr( "Islands" ), QString::number( Vect_get_num_islands( &Map ) ) );


          // Box and dimension
          BOUND_BOX box;
          char buffer[100];

          Vect_get_map_box( &Map, &box );

          QgsGrass::setMapset( mGisbase, mLocation, mMapset );
          struct Cell_head window;
          G_get_window( &window );
          int proj = window.proj;

          G_format_northing( box.N, buffer, proj );
          str += htmlTableRow( QObject::tr( "North" ), QString( buffer ) );
          G_format_northing( box.S, buffer, proj );
          str += htmlTableRow( QObject::tr( "South" ), QString( buffer ) );
          G_format_easting( box.E, buffer, proj );
          str += htmlTableRow( QObject::tr( "East" ), QString( buffer ) );
          G_format_easting( box.W, buffer, proj );
          str += htmlTableRow( QObject::tr( "West" ), QString( buffer ) );
          if ( is3d )
          {
            str += htmlTableRow( QObject::tr( "Top" ), QString::number( box.T ) );
            str += htmlTableRow( QObject::tr( "Bottom" ), QString::number( box.B ) );
          }

          str += htmlTableRow( "3D", is3d ? QObject::tr( "yes" ) : QObject::tr( "no" ) );

          str += "</table>";

          // History
          Vect_hist_rewind( &Map );
          char hbuffer[1001];
          str += "<p>" + QObject::tr( "History<br>" );
          QRegExp rx( "^-+$" );
          while ( Vect_hist_read( hbuffer, 1000, &Map ) != NULL )
          {
            QString row = QString( hbuffer );
            if ( rx.indexIn( row ) != -1 )
            {
              str += "<hr>";
            }
            else
            {
              str += row + "<br>";
            }
          }
        }
        else
        {
          str += "</table>";
        }
        Vect_close( &Map );
      }
      catch ( QgsGrass::Exception &e )
      {
        QgsDebugMsg( QString( "Cannot open GRASS vector: %1" ).arg( e.what() ) );
        str += "</table>";
        str += QString( "%1 <br>" ).arg( e.what() );
      }

      return str;
    }
    break;

    case QgsGrassModel::VectorLayer:
    {
      QString str = tblStart;
      str += htmlTableRow( QObject::tr( "<b>Vector</b>" ), QString( "<b>%1</b>" ).arg( mMap ) );
      str += htmlTableRow( QObject::tr( "<b>Layer</b>" ), QString( "<b>%1</b>" ).arg( mLayer ) );

      QgsGrass::setLocation( mGisbase, mLocation );

      try
      {
        struct Map_info Map;
        int level = Vect_open_old_head( &Map, mMap.toLocal8Bit().data(),
                                        mMapset.toLocal8Bit().data() );

        if ( level >= 2 )
        {
          struct field_info *fi;

          int field = QgsGrassProvider::grassLayer( mLayer );
          if ( field != -1 )
          {
            // Number of features
            int type = QgsGrassProvider::grassLayerType( mLayer );
            if ( type != -1 )
            {
              str += htmlTableRow( QObject::tr( "Features" ),
                                   QString::number( Vect_cidx_get_type_count( &Map, field, type ) ) );
            }

            fi = Vect_get_field( &Map, field );

            // Database link
            if ( fi )
            {
              str += htmlTableRow( QObject::tr( "Driver" ), QString( fi->driver ) );
              str += htmlTableRow( QObject::tr( "Database" ), QString( fi->database ) );
              str += htmlTableRow( QObject::tr( "Table" ), QString( fi->table ) );
              str += htmlTableRow( QObject::tr( "Key column" ), QString( fi->key ) );
            }
          }
        }
        str += "</table>";

        Vect_close( &Map );
      }
      catch ( QgsGrass::Exception &e )
      {
        QgsDebugMsg( QString( "Cannot open GRASS vector: %1" ).arg( e.what() ) );
        str += "</table>";
        str += QString( "%1 <br>" ).arg( e.what() );
      }
      return str;
    }
    break;

    case QgsGrassModel::Region:
    {
      QString str = tblStart;
      str += htmlTableRow( QObject::tr( "<b>Region</b>" ), QString( "<b>%1</b>" ).arg( mMap ) );

      struct Cell_head window;
      QgsGrass::setLocation( mGisbase, mLocation );
      if ( G__get_window( &window, "windows", mMap.toLocal8Bit().data(), mMapset.toLocal8Bit().data() ) != NULL )
      {
        str += "<tr><td colspan=2>" + QObject::tr( "Cannot open region header" ) + "</td></tr>";
      }
      else
      {
        QString proj;
        switch ( window.proj )
        {
          case PROJECTION_XY:
            proj = QObject::tr( "XY" );
            break;
          case PROJECTION_UTM:
            proj = QObject::tr( "UTM" );
            break;
          case PROJECTION_SP:
            proj = QObject::tr( "SP" );
            break;
          case PROJECTION_LL:
            proj = QObject::tr( "LL" );
            break;
          default:
            proj = QObject::tr( "Other" );
        }
        str += htmlTableRow( QObject::tr( "Projection Type" ), proj );
        if ( window.proj == PROJECTION_UTM )
          str += htmlTableRow( QObject::tr( "Zone" ), QString::number( window.zone ) );
        str += htmlTableRow( QObject::tr( "North" ), qgsDoubleToString( window.north ) );
        str += htmlTableRow( QObject::tr( "South" ), qgsDoubleToString( window.south ) );
        str += htmlTableRow( QObject::tr( "East" ), qgsDoubleToString( window.east ) );
        str += htmlTableRow( QObject::tr( "West" ), qgsDoubleToString( window.west ) );
        str += htmlTableRow( QObject::tr( "Columns" ), QString::number( window.cols ) );
        str += htmlTableRow( QObject::tr( "Rows" ), QString::number( window.rows ) );
        str += htmlTableRow( QObject::tr( "E-W resolution" ), qgsDoubleToString( window.ew_res ) );
        str += htmlTableRow( QObject::tr( "N-S resolution" ), qgsDoubleToString( window.ns_res ) );
        str += htmlTableRow( QObject::tr( "Top" ), QString::number( window.top ) );
        str += htmlTableRow( QObject::tr( "Bottom" ), QString::number( window.bottom ) );
        str += htmlTableRow( QObject::tr( "3D Cols" ), QString::number( window.cols3 ) );
        str += htmlTableRow( QObject::tr( "3D Rows" ), QString::number( window.rows3 ) );
        str += htmlTableRow( QObject::tr( "Depths" ), QString::number( window.depths ) );
        str += htmlTableRow( QObject::tr( "E-W 3D resolution" ), qgsDoubleToString( window.ew_res3 ) );
        str += htmlTableRow( QObject::tr( "N-S 3D resolution" ), qgsDoubleToString( window.ns_res3 ) );
      }
      str += "</table>";
      return str;
    }
    break;
  }
  return QString();
}

QString QgsGrassModelItem::htmlTableRow( QString s1, QString s2 )
{
  QStringList sl( s1 );
  sl.append( s2 );
  return htmlTableRow( sl );
}

QString QgsGrassModelItem::htmlTableRow( QStringList list )
{
  QString s = "<tr>";
  for ( int i = 0; i < list.size(); i++ )
  {
    s.append( "<td>" + list.at( i ) + "</td>" );
  }
  s.append( "</tr>" );
  return s;
}

QString QgsGrassModelItem::name()
{
  switch ( mType )
  {
    case QgsGrassModel::Location:
      return mLocation;
      break;
    case QgsGrassModel::Mapset:
      return mMapset;
      break;
    case QgsGrassModel::Vectors:
      return "vector";
      break;
    case QgsGrassModel::Rasters:
      return "raster";
      break;
    case QgsGrassModel::Regions:
      return "region";
      break;
    case QgsGrassModel::Vector:
    case QgsGrassModel::Raster:
    case QgsGrassModel::Region:
      return mMap;
      break;
    case QgsGrassModel::VectorLayer:
      return mLayer;
      break;
  }
  return QString();
}

QString QgsGrassModelItem::uri()
{
  switch ( mType )
  {
    case QgsGrassModel::VectorLayer:
      return mGisbase + "/" + mLocation + "/" + mMapset + "/"
             + mMap + "/" + mLayer;
      break;
    case QgsGrassModel::Raster:
      return mGisbase + "/" + mLocation + "/" + mMapset + "/cellhd/" + mMap;
      break;
  }
  return QString();
}

QgsGrassModelItem *QgsGrassModelItem::child( int i )
{
  Q_ASSERT( i >= 0 );
  Q_ASSERT( i < mChildren.size() );
  //return &(mChildren[i]);
  return mChildren[i];
}

void QgsGrassModelItem::populate()
{
  QgsDebugMsg( "called." );

  if ( mPopulated )
    return;

  mModel->refreshItem( this );
}

/*********************** MODEL ***********************/

QgsGrassModel::QgsGrassModel( QObject * parent )
    : QAbstractItemModel( parent )
{
  QgsDebugMsg( "called." );

  // Icons
  QStyle *style = QApplication::style();
  mIconDirectory = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
  mIconDirectory.addPixmap( style->standardPixmap( QStyle::SP_DirOpenIcon ),
                            QIcon::Normal, QIcon::On );
  QString location = QgsGrass::getDefaultGisdbase()
                     + "/" + QgsGrass::getDefaultLocation();

  mIconFile = QIcon( style->standardPixmap( QStyle::SP_FileIcon ) );

  mIconPointLayer = QgsGrassPlugin::getThemeIcon( "/mIconPointLayer.svg" );
  mIconLineLayer = QgsGrassPlugin::getThemeIcon( "/mIconLineLayer.svg" );
  mIconPolygonLayer = QgsGrassPlugin::getThemeIcon( "/mIconPolygonLayer.svg" );
  mIconVectorLayer = QgsGrassPlugin::getThemeIcon( "/grass/grass_browser_vector_layer.png" );
  mIconRasterLayer = QgsGrassPlugin::getThemeIcon( "/grass/grass_browser_raster_layer.png" );

  mRoot = new QgsGrassModelItem();
  mRoot->mType = QgsGrassModel::Location;
  mRoot->mModel = this;
  mRoot->mGisbase = QgsGrass::getDefaultGisdbase();
  mRoot->mLocation = QgsGrass::getDefaultLocation();
  //mRoot->refresh();
  refreshItem( mRoot );
}

QgsGrassModel::~QgsGrassModel() { }

void QgsGrassModel::refresh()
{
  QgsDebugMsg( "called." );

  //mRoot->refresh();
  refreshItem( mRoot );
}

QModelIndex QgsGrassModel::index( QgsGrassModelItem *item )
{
  // Item index
  QModelIndex index;
  if ( item->mParent )
  {
    Q_ASSERT( item->mParent->mChildren.size() > 0 );
    //QVector<QgsGrassModelItem> children = item->mParent->mChildren;
    //int row = (item - &(item->mParent->mChildren.at(0)));
    int row = -1;
    for ( int i = 0; i < item->mParent->mChildren.size(); i++ )
    {
      if ( item == item->mParent->mChildren[i] )
      {
        row = i;
        break;
      }
    }
    Q_ASSERT( row >= 0 );
    index = createIndex( row, 0, item );
  }
  else
  {
    index = QModelIndex();
  }
  return index;
}

void QgsGrassModel::removeItems( QgsGrassModelItem *item, QStringList list )
{
  QModelIndex index = QgsGrassModel::index( item );
  // Remove items not present in the list
  for ( int i = 0; i < item->mChildren.size(); )
  {
    if ( !list.contains( item->mChildren[i]->name() ) )
    {
      QgsDebugMsg( QString( "remove %1" ).arg( item->mChildren[i]->name() ) );
      beginRemoveRows( index, i, i );
      delete item->mChildren[i];
      item->mChildren.remove( i );
      endRemoveRows();
    }
    else
    {
      i++;
    }
  }
}

void QgsGrassModel::addItems( QgsGrassModelItem *item, QStringList list, int type )
{
// QgsDebugMsg("entered.");
  QModelIndex index = QgsGrassModel::index( item );

  // Add new items

  for ( int i = 0; i < list.size(); i++ )
  {
    QString name = list.at( i );
// QgsDebugMsg(QString("? add %1").arg(name));

    int insertAt = item->mChildren.size();
    for ( int i = 0; i < item->mChildren.size(); i++ )
    {
      if ( item->mChildren[i]->name() == name )
      {
        insertAt = -1;
        break;
      }
      if ( QString::localeAwareCompare( item->mChildren[i]->name(), name ) > 0 )
      {
        insertAt = i;
        break;
      }
    }

    if ( insertAt >= 0 )
    {
      QgsDebugMsg( QString( "insert %1 at %2" ).arg( name ).arg( insertAt ) );
      beginInsertRows( index, insertAt, insertAt );
      QgsGrassModelItem *newItem = new QgsGrassModelItem();
      item->mChildren.insert( insertAt, newItem );
      //QgsGrassModelItem *newItem = &(item->mChildren[insertAt]);
      newItem->mType = type;
      newItem->mParent = item;
      newItem->copyNames( item );
      switch ( newItem->mType )
      {
        case QgsGrassModel::Location:
          newItem->mLocation = name;
          break;
        case QgsGrassModel::Mapset:
          newItem->mMapset = name;
          break;
        case QgsGrassModel::Vectors:
        case QgsGrassModel::Rasters:
        case QgsGrassModel::Regions:
          break;
        case QgsGrassModel::Vector:
        case QgsGrassModel::Raster:
        case QgsGrassModel::Region:
          newItem->mMap = name;
          break;
        case QgsGrassModel::VectorLayer:
          newItem->mLayer = name;
          break;
      }

      endInsertRows();
    }
  }
}

void QgsGrassModel::refreshItem( QgsGrassModelItem *item )
{
  QgsDebugMsg( QString( "called with item type %1" ).arg( item->mType ) );

  // to avoid and endless recusion with Qt 4.4 let's pretend we already have populated
  item->mPopulated = true;

  switch ( item->mType )
  {
    case QgsGrassModel::Location:
    {
      QStringList list = QgsGrass::mapsets( item->mGisbase, item->mLocation );
      removeItems( item, list );
      addItems( item, list, QgsGrassModel::Mapset );
    }
    break;

    case QgsGrassModel::Mapset:
    {
      QStringList vectors = QgsGrass::vectors( item->mGisbase,
                            item->mLocation, item->mMapset );
      QStringList rasters = QgsGrass::rasters( item->mGisbase,
                            item->mLocation, item->mMapset );
      QStringList regions = QgsGrass::elements( item->mGisbase,
                            item->mLocation, item->mMapset,
                            "windows" );

      QStringList list;
      if ( vectors.count() > 0 )
        list.append( "vector" );
      if ( rasters.count() > 0 )
        list.append( "raster" );
      if ( regions.count() > 0 )
        list.append( "region" );

      removeItems( item, list );

      if ( vectors.count() > 0 )
        addItems( item, QStringList( "vector" ), QgsGrassModel::Vectors );

      if ( rasters.count() > 0 )
        addItems( item, QStringList( "raster" ), QgsGrassModel::Rasters );

      if ( regions.count() > 0 )
        addItems( item, QStringList( "region" ), QgsGrassModel::Regions );

    }
    break;

    case QgsGrassModel::Vectors:
    case QgsGrassModel::Rasters:
    case QgsGrassModel::Regions:
    {
      QStringList list;
      int type = 0;
      if ( item->mType == QgsGrassModel::Vectors )
      {
        list = QgsGrass::vectors( item->mGisbase, item->mLocation,
                                  item->mMapset );
        type = QgsGrassModel::Vector;
      }
      else if ( item->mType == QgsGrassModel::Rasters )
      {
        list = QgsGrass::rasters( item->mGisbase, item->mLocation,
                                  item->mMapset );
        type = QgsGrassModel::Raster;
      }
      else if ( item->mType == QgsGrassModel::Regions )
      {
        list = QgsGrass::elements( item->mGisbase, item->mLocation,
                                   item->mMapset, "windows" );
        type = QgsGrassModel::Region;
      }

      removeItems( item, list );
      addItems( item, list, type );

    }
    break;

    case QgsGrassModel::Vector:
    {
      QStringList list = QgsGrass::vectorLayers(
                           QgsGrass::getDefaultGisdbase(),
                           QgsGrass::getDefaultLocation(),
                           item->mMapset, item->mMap );

      removeItems( item, list );
      addItems( item, list, QgsGrassModel::VectorLayer );
    }
    break;

    case QgsGrassModel::Raster:
      break;

    case QgsGrassModel::Region:
      break;

    case QgsGrassModel::VectorLayer:
      break;
  }
  for ( int i = 0; i < item->mChildren.size(); i++ )
  {
    if ( item->mChildren[i]->mPopulated )
    {
      refreshItem( item->mChildren[i] );
    }
  }
}

QModelIndex QgsGrassModel::index( int row, int column,
                                  const QModelIndex & parent ) const
{
// QgsDebugMsg(QString("row = %1 column = %2").arg(row).arg(column));

  // It is strange(?) but Qt can call this method with row < 0
  // for example if beginInsertRows(,0,0) is called and the first
  // item was previously deleted => check if row < 0
  // and return empty QModelIndex, but is it correct?
  if ( row < 0 )
    return QModelIndex();

  QgsGrassModelItem *item;
  if ( !parent.isValid() )
  {
    item = mRoot;
  }
  else
  {
    item = static_cast<QgsGrassModelItem*>( parent.internalPointer() );
  }
  //if ( !item->populated() )
  //refreshItem(item);
  if ( !item->populated() )
    item->populate();
  return createIndex( row, column, item->child( row ) );
}

QModelIndex QgsGrassModel::parent( const QModelIndex & index ) const
{
// QgsDebugMsg("entered.");

  if ( !index.isValid() )
    return QModelIndex();

  QgsGrassModelItem *item =
    static_cast<QgsGrassModelItem*>( index.internalPointer() );

  QgsGrassModelItem *parentNode = item->mParent;

  if ( parentNode == 0 || parentNode == mRoot )
    return QModelIndex();

  // parent's row
  QVector<QgsGrassModelItem*> children = parentNode->mParent ?
                                         parentNode->mParent->mChildren : mRoot->mChildren;
  int row = -1;
  for ( int i = 0; i < children.size(); i++ )
  {
    if ( parentNode == children[i] )
    {
      row = i;
      break;
    }
  }
  Q_ASSERT( row >= 0 );
  return createIndex( row, 0, parentNode );
}

int QgsGrassModel::rowCount( const QModelIndex & parent ) const
{
// QgsDebugMsg("entered.");
  QgsGrassModelItem *item;
  if ( !parent.isValid() )
  {
    item = mRoot;
  }
  else
  {
    item = static_cast<QgsGrassModelItem*>( parent.internalPointer() );
  }
// QgsDebugMsg(QString("name = %1").arg(item->name()));
// QgsDebugMsg(QString("count = %1").arg(item->mChildren.size()));
  if ( !item->populated() )
    item->populate();
  //if ( !item->populated() )
  //refreshItem(item);
  return item->mChildren.size();
}

int QgsGrassModel::columnCount( const QModelIndex & parent ) const
{
  Q_UNUSED( parent );
// QgsDebugMsg("entered.");
  return 1;
}

QVariant QgsGrassModel::data( const QModelIndex &index, int role ) const
{
// QgsDebugMsg("entered.");

  if ( !index.isValid() )
    return QVariant();
  if ( role != Qt::DisplayRole && role != Qt::DecorationRole )
    return QVariant();

  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );

  if ( role == Qt::DecorationRole )
  {
    switch ( item->type() )
    {
      case QgsGrassModel::Vector :
        return mIconVectorLayer;
        break;

      case QgsGrassModel::Raster :
        return mIconRasterLayer;
        break;

      case QgsGrassModel::Region :
        return mIconFile;
        break;

      case QgsGrassModel::VectorLayer :
        if ( item->mLayer.contains( "point" ) )
        {
          return mIconPointLayer;
        }
        else if ( item->mLayer.contains( "line" ) )
        {
          return mIconLineLayer;
        }
        else if ( item->mLayer.contains( "polygon" ) )
        {
          return mIconPolygonLayer;
        }
        else
        {
          return mIconFile;
        }
        break;

      default:
        return mIconDirectory;
    }
  }
  return item->data( role );
}

QString QgsGrassModel::itemName( const QModelIndex &index )
{
  if ( !index.isValid() )
    return QString();

  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );

  return item->name();
}

QString QgsGrassModel::itemMapset( const QModelIndex &index )
{
  if ( !index.isValid() )
    return QString();

  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );

  return item->mMapset;
}

QString QgsGrassModel::itemMap( const QModelIndex &index )
{
  if ( !index.isValid() )
    return QString();

  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );

  return item->mMap;
}

QString QgsGrassModel::itemInfo( const QModelIndex &index )
{
  if ( !index.isValid() )
    return QString();

  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );

  return item->info();
}

int QgsGrassModel::itemType( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QgsGrassModel::None;
  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );
  return item->type();
}

QString QgsGrassModel::uri( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QString();
  QgsGrassModelItem *item;
  item = static_cast<QgsGrassModelItem*>( index.internalPointer() );
  return item->uri();
}

void QgsGrassModel::setLocation( const QString &gisbase, const QString &location )
{
  removeItems( mRoot, QStringList() );
  mGisbase = gisbase;
  mLocation = location;
  mRoot->mGisbase = gisbase;
  mRoot->mLocation = location;
  refreshItem( mRoot );
}

QVariant QgsGrassModel::headerData( int section,
                                    Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section );
  Q_UNUSED( orientation );
  Q_UNUSED( role );
// QgsDebugMsg("entered.");

  //TODO
  //if (orientation == Qt::Horizontal && role == Qt::DisplayRole)

  return QVariant();
}

Qt::ItemFlags QgsGrassModel::flags( const QModelIndex &index ) const
{
// QgsDebugMsg("entered.");

  //TODO
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
