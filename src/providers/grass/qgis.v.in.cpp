/***************************************************************************
    qgis.v.in.cpp
    ---------------------
    begin                : May 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <grass/version.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/vector.h>
}

#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QIODevice>

#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"
#include "qgsrasterblock.h"
#include "qgsspatialindex.h"
#include "qgsgrass.h"
#include "qgsgrassdatafile.h"

static struct line_pnts *gLine = Vect_new_line_struct();

void writePoint( struct Map_info *map, int type, const QgsPointXY &point, struct line_cats *cats )
{
  Vect_reset_line( gLine );
  Vect_append_point( gLine, point.x(), point.y(), 0 );
  Vect_write_line( map, type, gLine, cats );
}

void writePolyline( struct Map_info *map, int type, const QgsPolylineXY &polyline, struct line_cats *cats )
{
  Vect_reset_line( gLine );
  const auto constPolyline = polyline;
  for ( const QgsPointXY &point : constPolyline )
  {
    Vect_append_point( gLine, point.x(), point.y(), 0 );
  }
  Vect_write_line( map, type, gLine, cats );
}

static struct Map_info *finalMap = nullptr;
static struct Map_info *tmpMap = nullptr;
static QString sFinalName;
static QString sTmpName;
dbDriver *driver = nullptr;

void closeMaps()
{
  if ( tmpMap )
  {
    Vect_close( tmpMap );
    Vect_delete( sTmpName.toUtf8().constData() );
  }
  if ( finalMap )
  {
    Vect_close( finalMap );
    Vect_delete( sFinalName.toUtf8().constData() );
  }
  if ( driver )
  {
    // we should rollback transaction but there is not db_rollback_transaction()
    // With SQLite it takes very long time with large datasets to close db
    db_close_database_shutdown_driver( driver );
  }
  G_warning( "import canceled -> maps deleted" );
}

// check stream status or exit
void checkStream( QDataStream &stdinStream )
{
  if ( stdinStream.status() != QDataStream::Ok )
  {
    closeMaps();
    G_fatal_error( "Cannot read data stream" );
  }
}

void exitIfCanceled( QDataStream &stdinStream )
{
  bool isCanceled;
  stdinStream >> isCanceled;
  checkStream( stdinStream );
  if ( !isCanceled )
  {
    return;
  }
  closeMaps();
  exit( EXIT_SUCCESS );
}

// G_set_percent_routine only works in GRASS >= 7
//int percent_routine (int)
//{
// TODO: use it to interrupt cleaning funct  //stdinFile.open( stdin, QIODevice::ReadOnly | QIODevice::Unbuffered );ions
//}

int main( int argc, char **argv )
{
  struct Option *mapOption;

  G_gisinit( argv[0] );
  G_define_module();
  mapOption = G_define_standard_option( G_OPT_V_OUTPUT );

  if ( G_parser( argc, argv ) )
    exit( EXIT_FAILURE );

#ifdef Q_OS_WIN
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );
#endif
  QgsGrassDataFile stdinFile;
  stdinFile.open( stdin );
  QDataStream stdinStream( &stdinFile );

  QFile stdoutFile;
  stdoutFile.open( stdout, QIODevice::WriteOnly | QIODevice::Unbuffered );
  QDataStream stdoutStream( &stdoutFile );

  // global finalName, tmpName are used by checkStream()
  sFinalName = QString( mapOption->answer );
  QDateTime now = QDateTime::currentDateTime();
  sTmpName = QStringLiteral( "qgis_import_tmp_%1_%2" ).arg( mapOption->answer, now.toString( QStringLiteral( "yyyyMMddhhmmss" ) ) );

  qint32 typeQint32;
  stdinStream >> typeQint32;
  checkStream( stdinStream );
  QgsWkbTypes::Type wkbType = ( QgsWkbTypes::Type )typeQint32;
  QgsWkbTypes::Type wkbFlatType = QgsWkbTypes::flatType( wkbType );
  bool isPolygon = QgsWkbTypes::singleType( wkbFlatType ) == QgsWkbTypes::Polygon;

  finalMap = QgsGrass::vectNewMapStruct();
  Vect_open_new( finalMap, mapOption->answer, 0 );
  struct Map_info *map = finalMap;
  // keep tmp name in sync with QgsGrassMapsetItem::createChildren
  if ( isPolygon )
  {
    tmpMap = QgsGrass::vectNewMapStruct();
    // TODO: use Vect_open_tmp_new with GRASS 7
    Vect_open_new( tmpMap, sTmpName.toUtf8().constData(), 0 );
    map = tmpMap;
  }

  QgsFields srcFields;
  stdinStream >> srcFields;
  checkStream( stdinStream );
  // TODO: find (in QgsGrassVectorImport) if there is unique 'id' or 'cat' field and use it as cat
  int keyNum = 1;
  QString key;
  while ( true )
  {
    key = "cat" + ( keyNum == 1 ? QString() : QString::number( keyNum ) );
    if ( srcFields.indexFromName( key ) == -1 )
    {
      break;
    }
    keyNum++;
  }

  QgsFields fields;
  fields.append( QgsField( key, QVariant::Int ) );
  fields.extend( srcFields );

  struct field_info *fieldInfo = Vect_default_field_info( finalMap, 1, nullptr, GV_1TABLE );
  if ( Vect_map_add_dblink( finalMap, 1, nullptr, fieldInfo->table, key.toLatin1().data(),
                            fieldInfo->database, fieldInfo->driver ) != 0 )
  {
    G_fatal_error( "Cannot add link" );
  }

  driver = db_start_driver_open_database( fieldInfo->driver, fieldInfo->database );
  if ( !driver )
  {
    G_fatal_error( "Cannot open database %s by driver %s", fieldInfo->database, fieldInfo->driver );
  }
  try
  {
    QgsGrass::createTable( driver, QString( fieldInfo->table ), fields );
  }
  catch ( QgsGrass::Exception &e )
  {
    G_fatal_error( "Cannot create table: %s", e.what() );
  }

  if ( db_grant_on_table( driver, fieldInfo->table, DB_PRIV_SELECT, DB_GROUP | DB_PUBLIC ) != DB_OK )
  {
    // TODO: fatal?
    //G_fatal_error(("Unable to grant privileges on table <%s>"), fieldInfo->table);
  }
  db_begin_transaction( driver );

  QgsFeature feature;
  struct line_cats *cats = Vect_new_cats_struct();

  qint32 fidToCatPlus;
  stdinStream >> fidToCatPlus;

  qint32 featureCount;
  stdinStream >> featureCount;

  qint32 count = 0;
  while ( true )
  {
    exitIfCanceled( stdinStream );
    stdinStream >> feature;
    checkStream( stdinStream );
#ifndef Q_OS_WIN
    // cannot be used on Windows, see notes in qgis.r.in
//#if 0
    stdoutStream << true; // feature received
    stdoutFile.flush();
//#endif
#endif
    if ( !feature.isValid() )
    {
      break;
    }

    QgsGeometry geometry = feature.geometry();
    if ( !geometry.isNull() )
    {
      // geometry type may be probably different from provider type (e.g. multi x single)
      QgsWkbTypes::Type geometryType = QgsWkbTypes::flatType( geometry.wkbType() );
      if ( !isPolygon )
      {
        Vect_reset_cats( cats );
        Vect_cat_set( cats, 1, static_cast<int>( feature.id() ) + fidToCatPlus );
      }

      if ( geometryType == QgsWkbTypes::Point )
      {
        QgsPointXY point = geometry.asPoint();
        writePoint( map, GV_POINT, point, cats );
      }
      else if ( geometryType == QgsWkbTypes::MultiPoint )
      {
        QgsMultiPointXY multiPoint = geometry.asMultiPoint();
        const auto constMultiPoint = multiPoint;
        for ( const QgsPointXY &point : constMultiPoint )
        {
          writePoint( map, GV_POINT, point, cats );
        }
      }
      else if ( geometryType == QgsWkbTypes::LineString )
      {
        QgsPolylineXY polyline = geometry.asPolyline();
        writePolyline( map, GV_LINE, polyline, cats );
      }
      else if ( geometryType == QgsWkbTypes::MultiLineString )
      {
        QgsMultiPolylineXY multiPolyline = geometry.asMultiPolyline();
        const auto constMultiPolyline = multiPolyline;
        for ( const QgsPolylineXY &polyline : constMultiPolyline )
        {
          writePolyline( map, GV_LINE, polyline, cats );
        }
      }
      else if ( geometryType == QgsWkbTypes::Polygon )
      {
        QgsPolygonXY polygon = geometry.asPolygon();
        const auto constPolygon = polygon;
        for ( const QgsPolylineXY &polyline : constPolygon )
        {
          writePolyline( map, GV_BOUNDARY, polyline, cats );
        }
      }
      else if ( geometryType == QgsWkbTypes::MultiPolygon )
      {
        QgsMultiPolygonXY multiPolygon = geometry.asMultiPolygon();
        const auto constMultiPolygon = multiPolygon;
        for ( const QgsPolygonXY &polygon : constMultiPolygon )
        {
          const auto constPolygon = polygon;
          for ( const QgsPolylineXY &polyline : constPolygon )
          {
            writePolyline( map, GV_BOUNDARY, polyline, cats );
          }
        }
      }
      else
      {
        G_fatal_error( "Geometry type not supported" );
      }

      QgsAttributes attributes = feature.attributes();
      attributes.insert( 0, QVariant( feature.id() + fidToCatPlus ) );
      try
      {
        // TODO: inserting row is extremely slow on Windows (at least with SQLite), v.in.ogr is fast
        QgsGrass::insertRow( driver, QString( fieldInfo->table ), attributes );
      }
      catch ( QgsGrass::Exception &e )
      {
        G_fatal_error( "Cannot insert: %s", e.what() );
      }
    }
    count++;
    G_percent( count, featureCount, 1 );
  }
  db_commit_transaction( driver );
  db_close_database_shutdown_driver( driver );

  if ( isPolygon )
  {
    G_message( "Building partial topology" );
    Vect_build_partial( map, GV_BUILD_BASE );

#if 0
    double snapThreshold = 0.0;
    if ( snapThreshold > 0.0 )
    {
      Vect_snap_lines( map, GV_BOUNDARY, snapThreshold, nullptr );
    }
#endif

    G_message( "Breaking polygons" );
    Vect_break_polygons( map, GV_BOUNDARY, nullptr );
    G_message( "Removing duplicates" );
    Vect_remove_duplicates( map, GV_BOUNDARY | GV_CENTROID, nullptr );
    for ( int i = 0; i < 3; i++ )
    {
      G_message( "Breaking lines" );
      Vect_break_lines( map, GV_BOUNDARY, nullptr );
      G_message( "Removing duplicates" );
      Vect_remove_duplicates( map, GV_BOUNDARY, nullptr );
      G_message( "Cleaning small dangles at nodes" );
      if ( Vect_clean_small_angles_at_nodes( map, GV_BOUNDARY, nullptr ) == 0 )
      {
        break;
      }
    }
    // TODO: review if necessary and if there is GRASS function
    // remove zero length
    int nlines = Vect_get_num_lines( map );
    struct line_pnts *line = Vect_new_line_struct();
    for ( int i = 1; i <= nlines; i++ )
    {
      if ( !Vect_line_alive( map, i ) )
      {
        continue;
      }

      int type = Vect_read_line( map, line, nullptr, i );
      if ( !( type & GV_BOUNDARY ) )
      {
        continue;
      }

      if ( Vect_line_length( line ) == 0 )
      {
        Vect_delete_line( map, i );
      }
    }

    G_message( "Merging lines" );
    Vect_merge_lines( map, GV_BOUNDARY, nullptr, nullptr );
    G_message( "Removing bridges" );
    int linesRemoved, bridgesRemoved;
    Vect_remove_bridges( map, nullptr, &linesRemoved, &bridgesRemoved );
    G_message( "Attaching islands" );
    Vect_build_partial( map, GV_BUILD_ATTACH_ISLES );

    G_message( "Creating centroids" );
    QMap<QgsFeatureId, QgsFeature> centroids;
    QgsSpatialIndex spatialIndex;
    int nAreas = Vect_get_num_areas( map );
    for ( int area = 1; area <= nAreas; area++ )
    {
      double x, y;
      if ( Vect_get_point_in_area( map, area, &x, &y ) < 0 )
      {
        // TODO: send warning
        continue;
      }
      QgsPointXY point( x, y );
      QgsFeature feature( area );
      feature.setGeometry( QgsGeometry::fromPointXY( point ) );
      feature.setValid( true );
      centroids.insert( area, feature );
      spatialIndex.addFeature( feature );
    }

    G_message( "Attaching input polygons to cleaned areas" );
    // read once more to assign centroids to polygons
    count = 0;
    while ( true )
    {
      exitIfCanceled( stdinStream );
      stdinStream >> feature;
      checkStream( stdinStream );
#ifndef Q_OS_WIN
#if 0
      stdoutStream << true; // feature received
      stdoutFile.flush();
#endif
#endif
      if ( !feature.isValid() )
      {
        break;
      }
      if ( !feature.hasGeometry() )
      {
        continue;
      }

      QList<QgsFeatureId> idList = spatialIndex.intersects( feature.geometry().boundingBox() );
      const auto constIdList = idList;
      for ( QgsFeatureId id : constIdList )
      {
        QgsFeature &centroid = centroids[id];
        if ( feature.geometry().contains( centroid.geometry() ) )
        {
          QgsAttributes attr = centroid.attributes();
          attr.append( static_cast<int>( feature.id() ) + fidToCatPlus );
          centroid.setAttributes( attr );
        }
      }
      count++;
      G_percent( count, featureCount, 1 );
    }

    G_message( "Copying lines from temporary map" );
    Vect_copy_map_lines( tmpMap, finalMap );
    Vect_close( tmpMap );
    Vect_delete( sTmpName.toUtf8().constData() );

    int centroidsCount = centroids.size();
    count = 0;
    for ( auto it = centroids.constBegin(); it != centroids.constEnd(); ++it )
    {
      QgsPointXY point = it.value().geometry().asPoint();

      if ( it.value().attributes().size() > 0 )
      {
        Vect_reset_cats( cats );
        const auto constAttributes = it.value().attributes();
        for ( const QVariant &attribute : constAttributes )
        {
          Vect_cat_set( cats, 1, attribute.toInt() );
        }
        writePoint( finalMap, GV_CENTROID, point, cats );
      }
      G_percent( count, centroidsCount, 1 );
    }
  }

  G_message( "Building final map topology" );
  Vect_build( finalMap );
  Vect_close( finalMap );

  G_message( "Done" );

  stdoutStream << true; // to keep caller waiting until finished
  stdoutFile.flush();
  // TODO history

  exit( EXIT_SUCCESS );
}
