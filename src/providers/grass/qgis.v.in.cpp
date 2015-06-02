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

#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#endif
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

static struct line_pnts *line = Vect_new_line_struct();

void writePoint( struct Map_info* map, int type, QgsPoint point, struct line_cats *cats )
{
  Vect_reset_line( line );
  Vect_append_point( line, point.x(), point.y(), 0 );
  Vect_write_line( map, type, line, cats );
}

void writePolyline( struct Map_info* map, int type, QgsPolyline polyline, struct line_cats *cats )
{
  Vect_reset_line( line );
  foreach ( QgsPoint point, polyline )
  {
    Vect_append_point( line, point.x(), point.y(), 0 );
  }
  Vect_write_line( map, type, line, cats );
}

void exitIfCanceled( QDataStream& stdinStream, bool isPolygon,
                     const QString & tmpName, struct Map_info * tmpMap,
                     const QString & finalName, struct Map_info * finalMap )
{
  bool isCanceled;
  stdinStream >> isCanceled;
  if ( !isCanceled )
  {
    return;
  }
  if ( isPolygon )
  {
    Vect_close( tmpMap );
    Vect_delete( tmpName.toUtf8().data() );
  }
  Vect_close( finalMap );
  Vect_delete( finalName.toUtf8().data() );
  G_warning( "import canceled -> maps deleted" );
  exit( EXIT_SUCCESS );
}

// G_set_percent_routine only works in GRASS >= 7
//int percent_routine (int)
//{
// TODO: use it to interrupt cleaning functions
//}

int main( int argc, char **argv )
{
  struct Option *mapOption;

  G_gisinit( argv[0] );
  G_define_module();
  mapOption = G_define_standard_option( G_OPT_V_OUTPUT );

  if ( G_parser( argc, argv ) )
    exit( EXIT_FAILURE );

  QFile stdinFile;
  stdinFile.open( stdin, QIODevice::ReadOnly );
  QDataStream stdinStream( &stdinFile );

  QFile stdoutFile;
  stdoutFile.open( stdout, QIODevice::WriteOnly );
  QDataStream stdoutStream( &stdoutFile );

  qint32 typeQint32;
  stdinStream >> typeQint32;
  QGis::WkbType wkbType = ( QGis::WkbType )typeQint32;
  QGis::WkbType wkbFlatType = QGis::flatType( wkbType );
  bool isPolygon = QGis::singleType( wkbFlatType ) == QGis::WKBPolygon;

  QString finalName = QString( mapOption->answer );
  struct Map_info finalMap, tmpMap;
  Vect_open_new( &finalMap, mapOption->answer, 0 );
  struct Map_info * map = &finalMap;
  QDateTime now = QDateTime::currentDateTime();
  QString tmpName = QString( "%1_tmp_%2" ).arg( mapOption->answer ).arg( now.toString( "yyyyMMddhhmmss" ) );
  if ( isPolygon )
  {
    Vect_open_new( &tmpMap, tmpName.toUtf8().data(), 0 );
    map = &tmpMap;
  }

  QgsFields srcFields;
  stdinStream >> srcFields;
  // TODO: find (in QgsGrassVectorImport) if there is unique 'id' or 'cat' field and use it as cat
  int keyNum = 1;
  QString key;
  while ( true )
  {
    key = "cat" + ( keyNum == 1 ? "" : QString::number( keyNum ) );
    if ( srcFields.indexFromName( key ) == -1 )
    {
      break;
    }
    keyNum++;
  }

  QgsFields fields;
  fields.append( QgsField( key, QVariant::Int ) );
  fields.extend( srcFields );

  struct field_info *fieldInfo = Vect_default_field_info( &finalMap, 1, NULL, GV_1TABLE );
  if ( Vect_map_add_dblink( &finalMap, 1, NULL, fieldInfo->table, key.toLatin1().data(),
                            fieldInfo->database, fieldInfo->driver ) != 0 )
  {
    G_fatal_error( "Cannot add link" );
  }

  dbDriver *driver = db_start_driver_open_database( fieldInfo->driver, fieldInfo->database );
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

  QgsFeature feature;
  struct line_cats *cats = Vect_new_cats_struct();

  qint32 featureCount = 0;
  while ( true )
  {
    exitIfCanceled( stdinStream, isPolygon, tmpName, &tmpMap, finalName, &finalMap );
    stdinStream >> feature;
    stdoutStream << ( bool )true; // feature received
    stdoutFile.flush();
    if ( !feature.isValid() )
    {
      break;
    }

    QgsGeometry* geometry = feature.geometry();
    if ( geometry )
    {
      // geometry type may be probably different from provider type (e.g. multi x single)
      QGis::WkbType geometryType = QGis::flatType( geometry->wkbType() );
      if ( !isPolygon )
      {
        Vect_reset_cats( cats );
        Vect_cat_set( cats, 1, ( int )feature.id() );
      }

      if ( geometryType == QGis::WKBPoint )
      {
        QgsPoint point = geometry->asPoint();
        writePoint( map, GV_POINT, point, cats );
      }
      else if ( geometryType == QGis::WKBMultiPoint )
      {
        QgsMultiPoint multiPoint = geometry->asMultiPoint();
        foreach ( QgsPoint point, multiPoint )
        {
          writePoint( map, GV_POINT, point, cats );
        }
      }
      else if ( geometryType == QGis::WKBLineString )
      {
        QgsPolyline polyline = geometry->asPolyline();
        writePolyline( map, GV_LINE, polyline, cats );
      }
      else if ( geometryType == QGis::WKBMultiLineString )
      {
        QgsMultiPolyline multiPolyline = geometry->asMultiPolyline();
        foreach ( QgsPolyline polyline, multiPolyline )
        {
          writePolyline( map, GV_LINE, polyline, cats );
        }
      }
      else if ( geometryType == QGis::WKBPolygon )
      {
        QgsPolygon polygon = geometry->asPolygon();
        foreach ( QgsPolyline polyline, polygon )
        {
          writePolyline( map, GV_BOUNDARY, polyline, cats );
        }
      }
      else if ( geometryType == QGis::WKBMultiPolygon )
      {
        QgsMultiPolygon multiPolygon = geometry->asMultiPolygon();
        foreach ( QgsPolygon polygon, multiPolygon )
        {
          foreach ( QgsPolyline polyline, polygon )
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
      attributes.insert( 0, QVariant( feature.id() ) );
      try
      {
        QgsGrass::insertRow( driver, QString( fieldInfo->table ), attributes );
      }
      catch ( QgsGrass::Exception &e )
      {
        G_fatal_error( "Cannot insert: %s", e.what() );
      }
    }
    featureCount++;
  }

  if ( isPolygon )
  {
    double snapTreshold = 0;
    Vect_build_partial( map, GV_BUILD_BASE );

    if ( snapTreshold > 0 )
    {
      Vect_snap_lines( map, GV_BOUNDARY, snapTreshold, NULL );
    }
    Vect_break_polygons( map, GV_BOUNDARY, NULL );
    Vect_remove_duplicates( map, GV_BOUNDARY | GV_CENTROID, NULL );
    while ( true )
    {
      Vect_break_lines( map, GV_BOUNDARY, NULL );
      Vect_remove_duplicates( map, GV_BOUNDARY, NULL );
      if ( Vect_clean_small_angles_at_nodes( map, GV_BOUNDARY, NULL ) == 0 )
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

      int type = Vect_read_line( map, line, NULL, i );
      if ( !( type & GV_BOUNDARY ) )
      {
        continue;
      }

      if ( Vect_line_length( line ) == 0 )
      {
        Vect_delete_line( map, i );
      }
    }

    Vect_merge_lines( map, GV_BOUNDARY, NULL, NULL );
#if GRASS_VERSION_MAJOR < 7
    Vect_remove_bridges( map, NULL );
#else
    int linesRemoved, bridgesRemoved;
    Vect_remove_bridges( map, NULL, &linesRemoved, &bridgesRemoved );
#endif
    Vect_build_partial( map, GV_BUILD_ATTACH_ISLES );

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
      QgsPoint point( x, y );
      QgsFeature feature( area );
      feature.setGeometry( QgsGeometry::fromPoint( point ) );
      feature.setValid( true );
      centroids.insert( area, feature );
      spatialIndex.insertFeature( feature );
    }
    // read once more to assign centroids to polygons
    while ( true )
    {
      exitIfCanceled( stdinStream, isPolygon, tmpName, &tmpMap, finalName, &finalMap );
      stdinStream >> feature;
      stdoutStream << ( bool )true; // feature received
      stdoutFile.flush();
      if ( !feature.isValid() )
      {
        break;
      }
      if ( !feature.geometry() )
      {
        continue;
      }

      QList<QgsFeatureId> idList = spatialIndex.intersects( feature.geometry()->boundingBox() );
      foreach ( QgsFeatureId id, idList )
      {
        QgsFeature& centroid = centroids[id];
        if ( feature.geometry()->contains( centroid.geometry() ) )
        {
          QgsAttributes attr = centroid.attributes();
          attr.append( feature.id() );
          centroid.setAttributes( attr );
        }
      }
    }

    Vect_copy_map_lines( &tmpMap, &finalMap );
    Vect_close( &tmpMap );
    Vect_delete( tmpName.toUtf8().data() );

    foreach ( QgsFeature centroid, centroids.values() )
    {
      QgsPoint point = centroid.geometry()->asPoint();

      if ( centroid.attributes().size() > 0 )
      {
        Vect_reset_cats( cats );
        foreach ( QVariant attribute, centroid.attributes() )
        {
          Vect_cat_set( cats, 1, attribute.toInt() );
        }
        writePoint( &finalMap, GV_CENTROID, point, cats );
      }
    }
  }

  db_close_database_shutdown_driver( driver );
  Vect_build( &finalMap );
  Vect_close( &finalMap );

  stdoutStream << ( bool )true; // to keep caller waiting until finished
  stdoutFile.flush();
  // TODO history

  exit( EXIT_SUCCESS );
}
