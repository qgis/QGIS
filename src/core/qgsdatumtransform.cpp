/***************************************************************************
               qgsdatumtransform.cpp
               ------------------------
    begin                : Dec 2017
    copyright            : (C) 2017 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdatumtransform.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsapplication.h"
#include "qgssqliteutils.h"
#include <sqlite3.h>

QList< QgsDatumTransform::TransformPair > QgsDatumTransform::datumTransformations( const QgsCoordinateReferenceSystem &srcCRS, const QgsCoordinateReferenceSystem &destCRS )
{
  QList< QgsDatumTransform::TransformPair > transformations;

  QString srcGeoId = srcCRS.geographicCrsAuthId();
  QString destGeoId = destCRS.geographicCrsAuthId();

  if ( srcGeoId.isEmpty() || destGeoId.isEmpty() )
  {
    return transformations;
  }

  QStringList srcSplit = srcGeoId.split( ':' );
  QStringList destSplit = destGeoId.split( ':' );

  if ( srcSplit.size() < 2 || destSplit.size() < 2 )
  {
    return transformations;
  }

  int srcAuthCode = srcSplit.at( 1 ).toInt();
  int destAuthCode = destSplit.at( 1 ).toInt();

  if ( srcAuthCode == destAuthCode )
  {
    return transformations; //crs have the same datum
  }

  QList<int> directTransforms;
  searchDatumTransform( QStringLiteral( "SELECT coord_op_code FROM tbl_datum_transform WHERE source_crs_code=%1 AND target_crs_code=%2 ORDER BY deprecated ASC,preferred DESC" ).arg( srcAuthCode ).arg( destAuthCode ),
                        directTransforms );
  QList<int> reverseDirectTransforms;
  searchDatumTransform( QStringLiteral( "SELECT coord_op_code FROM tbl_datum_transform WHERE source_crs_code = %1 AND target_crs_code=%2 ORDER BY deprecated ASC,preferred DESC" ).arg( destAuthCode ).arg( srcAuthCode ),
                        reverseDirectTransforms );
  QList<int> srcToWgs84;
  searchDatumTransform( QStringLiteral( "SELECT coord_op_code FROM tbl_datum_transform WHERE (source_crs_code=%1 AND target_crs_code=%2) OR (source_crs_code=%2 AND target_crs_code=%1) ORDER BY deprecated ASC,preferred DESC" ).arg( srcAuthCode ).arg( 4326 ),
                        srcToWgs84 );
  QList<int> destToWgs84;
  searchDatumTransform( QStringLiteral( "SELECT coord_op_code FROM tbl_datum_transform WHERE (source_crs_code=%1 AND target_crs_code=%2) OR (source_crs_code=%2 AND target_crs_code=%1) ORDER BY deprecated ASC,preferred DESC" ).arg( destAuthCode ).arg( 4326 ),
                        destToWgs84 );

  //add direct datum transformations
  for ( int transform : qgis::as_const( directTransforms ) )
  {
    transformations.push_back( QgsDatumTransform::TransformPair( transform, -1 ) );
  }

  //add direct datum transformations
  for ( int transform : qgis::as_const( reverseDirectTransforms ) )
  {
    transformations.push_back( QgsDatumTransform::TransformPair( -1, transform ) );
  }

  for ( int srcTransform : qgis::as_const( srcToWgs84 ) )
  {
    for ( int destTransform : qgis::as_const( destToWgs84 ) )
    {
      transformations.push_back( QgsDatumTransform::TransformPair( srcTransform, destTransform ) );
    }
  }

  return transformations;
}

void QgsDatumTransform::searchDatumTransform( const QString &sql, QList< int > &transforms )
{
  sqlite3_database_unique_ptr database;
  int openResult = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( openResult != SQLITE_OK )
  {
    return;
  }

  sqlite3_statement_unique_ptr statement;
  int prepareRes;
  statement = database.prepare( sql, prepareRes );
  if ( prepareRes != SQLITE_OK )
  {
    return;
  }

  QString cOpCode;
  while ( statement.step() == SQLITE_ROW )
  {
    cOpCode = statement.columnAsText( 0 );
    transforms.push_back( cOpCode.toInt() );
  }
}

QString QgsDatumTransform::datumTransformToProj( int datumTransform )
{
  QString transformString;

  sqlite3_database_unique_ptr database;
  int openResult = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( openResult != SQLITE_OK )
  {
    return transformString;
  }

  sqlite3_statement_unique_ptr statement;
  QString sql = QStringLiteral( "SELECT coord_op_method_code,p1,p2,p3,p4,p5,p6,p7 FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( datumTransform );
  int prepareRes;
  statement = database.prepare( sql, prepareRes );
  if ( prepareRes != SQLITE_OK )
  {
    return transformString;
  }

  if ( statement.step() == SQLITE_ROW )
  {
    //coord_op_methode_code
    int methodCode = statement.columnAsInt64( 0 );
    if ( methodCode == 9615 ) //ntv2
    {
      transformString = "+nadgrids=" + statement.columnAsText( 1 );
    }
    else if ( methodCode == 9603 || methodCode == 9606 || methodCode == 9607 )
    {
      transformString += QLatin1String( "+towgs84=" );
      double p1 = statement.columnAsDouble( 1 );
      double p2 = statement.columnAsDouble( 2 );
      double p3 = statement.columnAsDouble( 3 );
      double p4 = statement.columnAsDouble( 4 );
      double p5 = statement.columnAsDouble( 5 );
      double p6 = statement.columnAsDouble( 6 );
      double p7 = statement.columnAsDouble( 7 );
      if ( methodCode == 9603 ) //3 parameter transformation
      {
        transformString += QStringLiteral( "%1,%2,%3" ).arg( QString::number( p1 ), QString::number( p2 ), QString::number( p3 ) );
      }
      else //7 parameter transformation
      {
        transformString += QStringLiteral( "%1,%2,%3,%4,%5,%6,%7" ).arg( QString::number( p1 ), QString::number( p2 ), QString::number( p3 ), QString::number( p4 ), QString::number( p5 ), QString::number( p6 ), QString::number( p7 ) );
      }
    }
  }

  return transformString;
}

int QgsDatumTransform::projStringToDatumTransformId( const QString &string )
{
  sqlite3_database_unique_ptr database;
  int openResult = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( openResult != SQLITE_OK )
  {
    return -1;
  }

  sqlite3_statement_unique_ptr statement;
  QString sql = QStringLiteral( "SELECT coord_op_method_code,p1,p2,p3,p4,p5,p6,p7,coord_op_code FROM tbl_datum_transform" );
  int prepareRes;
  statement = database.prepare( sql, prepareRes );
  if ( prepareRes != SQLITE_OK )
  {
    return -1;
  }

  while ( statement.step() == SQLITE_ROW )
  {
    QString transformString;
    //coord_op_methode_code
    int methodCode = statement.columnAsInt64( 0 );
    if ( methodCode == 9615 ) //ntv2
    {
      transformString = "+nadgrids=" + statement.columnAsText( 1 );
    }
    else if ( methodCode == 9603 || methodCode == 9606 || methodCode == 9607 )
    {
      transformString += QLatin1String( "+towgs84=" );
      double p1 = statement.columnAsDouble( 1 );
      double p2 = statement.columnAsDouble( 2 );
      double p3 = statement.columnAsDouble( 3 );
      double p4 = statement.columnAsDouble( 4 );
      double p5 = statement.columnAsDouble( 5 );
      double p6 = statement.columnAsDouble( 6 );
      double p7 = statement.columnAsDouble( 7 );
      if ( methodCode == 9603 ) //3 parameter transformation
      {
        transformString += QStringLiteral( "%1,%2,%3" ).arg( QString::number( p1 ), QString::number( p2 ), QString::number( p3 ) );
      }
      else //7 parameter transformation
      {
        transformString += QStringLiteral( "%1,%2,%3,%4,%5,%6,%7" ).arg( QString::number( p1 ), QString::number( p2 ), QString::number( p3 ), QString::number( p4 ), QString::number( p5 ), QString::number( p6 ), QString::number( p7 ) );
      }
    }

    if ( transformString.compare( string, Qt::CaseInsensitive ) == 0 )
    {
      return statement.columnAsInt64( 8 );
    }
  }

  return -1;
}

QgsDatumTransform::TransformInfo QgsDatumTransform::datumTransformInfo( int datumTransform )
{
  QgsDatumTransform::TransformInfo info;

  sqlite3_database_unique_ptr database;
  int openResult = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( openResult != SQLITE_OK )
  {
    return info;
  }

  sqlite3_statement_unique_ptr statement;
  QString sql = QStringLiteral( "SELECT epsg_nr,source_crs_code,target_crs_code,remarks,scope,preferred,deprecated FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( datumTransform );
  int prepareRes;
  statement = database.prepare( sql, prepareRes );
  if ( prepareRes != SQLITE_OK )
  {
    return info;
  }

  int srcCrsId, destCrsId;
  if ( statement.step() != SQLITE_ROW )
  {
    return info;
  }

  info.datumTransformId = datumTransform;
  info.epsgCode = statement.columnAsInt64( 0 );
  srcCrsId = statement.columnAsInt64( 1 );
  destCrsId = statement.columnAsInt64( 2 );
  info.remarks = statement.columnAsText( 3 );
  info.scope = statement.columnAsText( 4 );
  info.preferred = statement.columnAsInt64( 5 ) != 0;
  info.deprecated = statement.columnAsInt64( 6 ) != 0;

  QgsCoordinateReferenceSystem srcCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:%1" ).arg( srcCrsId ) );
  info.sourceCrsDescription = srcCrs.description();
  info.sourceCrsAuthId = srcCrs.authid();
  QgsCoordinateReferenceSystem destCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:%1" ).arg( destCrsId ) );
  info.destinationCrsDescription = destCrs.description();
  info.destinationCrsAuthId = destCrs.authid();

  return info;
}
