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

#if PROJ_VERSION_MAJOR>=6
#include "qgsprojutils.h"
#include <proj.h>
#endif

QList<QgsDatumTransform::TransformDetails> QgsDatumTransform::operations( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, bool includeSuperseded )
{
  QList< QgsDatumTransform::TransformDetails > res;
#if PROJ_VERSION_MAJOR<6
  Q_UNUSED( source )
  Q_UNUSED( destination )
  Q_UNUSED( includeSuperseded )
#else
  if ( !source.projObject() || !destination.projObject() )
    return res;

  PJ_CONTEXT *pjContext = QgsProjContext::get();

  PJ_OPERATION_FACTORY_CONTEXT *operationContext = proj_create_operation_factory_context( pjContext, nullptr );

  // We want to return ALL grids, not just those available for use
  proj_operation_factory_context_set_grid_availability_use( pjContext, operationContext, PROJ_GRID_AVAILABILITY_IGNORED );

  // See https://lists.osgeo.org/pipermail/proj/2019-May/008604.html
  proj_operation_factory_context_set_spatial_criterion( pjContext, operationContext,  PROJ_SPATIAL_CRITERION_PARTIAL_INTERSECTION );

#if PROJ_VERSION_MAJOR>6 || (PROJ_VERSION_MAJOR==6 && PROJ_VERSION_MINOR>=2)
  if ( includeSuperseded )
    proj_operation_factory_context_set_discard_superseded( pjContext, operationContext, false );
#else
  Q_UNUSED( includeSuperseded )
#endif
  if ( PJ_OBJ_LIST *ops = proj_create_operations( pjContext, source.projObject(), destination.projObject(), operationContext ) )
  {
    int count = proj_list_get_count( ops );
    for ( int i = 0; i < count; ++i )
    {
      QgsProjUtils::proj_pj_unique_ptr op( proj_list_get( pjContext, ops, i ) );
      if ( !op )
        continue;

      res.push_back( transformDetailsFromPj( op.get() ) );
    }
    proj_list_destroy( ops );
  }
  proj_operation_factory_context_destroy( operationContext );
#endif
  return res;
}

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

#if PROJ_VERSION_MAJOR>=6
QgsDatumTransform::TransformDetails QgsDatumTransform::transformDetailsFromPj( PJ *op )
{
  PJ_CONTEXT *pjContext = QgsProjContext::get();
  TransformDetails details;
  if ( !op )
    return details;

  QgsProjUtils::proj_pj_unique_ptr normalized( proj_normalize_for_visualization( pjContext, op ) );
  if ( normalized )
    details.proj = QString( proj_as_proj_string( pjContext, normalized.get(), PJ_PROJ_5, nullptr ) );

  if ( details.proj.isEmpty() )
    details.proj = QString( proj_as_proj_string( pjContext, op, PJ_PROJ_5, nullptr ) );
  details.name = QString( proj_get_name( op ) );
  details.accuracy = proj_coordoperation_get_accuracy( pjContext, op );
  details.isAvailable = proj_coordoperation_is_instantiable( pjContext, op );

  details.authority = QString( proj_get_id_auth_name( op, 0 ) );
  details.code = QString( proj_get_id_code( op, 0 ) );

  const char *areaOfUseName = nullptr;
  double westLon = 0;
  double southLat = 0;
  double eastLon = 0;
  double northLat = 0;
  if ( proj_get_area_of_use( pjContext, op, &westLon, &southLat, &eastLon, &northLat, &areaOfUseName ) )
  {
    details.areaOfUse = QString( areaOfUseName );
    // don't use the constructor which normalizes!
    details.bounds.setXMinimum( westLon );
    details.bounds.setYMinimum( southLat );
    details.bounds.setXMaximum( eastLon );
    details.bounds.setYMaximum( northLat );
  }

#if PROJ_VERSION_MAJOR>6 || (PROJ_VERSION_MAJOR==6 && PROJ_VERSION_MINOR>=2)
  details.remarks = QString( proj_get_remarks( op ) );
  details.scope = QString( proj_get_scope( op ) );
#endif

  for ( int j = 0; j < proj_coordoperation_get_grid_used_count( pjContext, op ); ++j )
  {
    const char *shortName = nullptr;
    const char *fullName = nullptr;
    const char *packageName = nullptr;
    const char *url = nullptr;
    int directDownload = 0;
    int openLicense = 0;
    int isAvailable = 0;
    proj_coordoperation_get_grid_used( pjContext, op, j, &shortName, &fullName, &packageName, &url, &directDownload, &openLicense, &isAvailable );
    GridDetails gridDetails;
    gridDetails.shortName = QString( shortName );
    gridDetails.fullName = QString( fullName );
    gridDetails.packageName = QString( packageName );
    gridDetails.url = QString( url );
    gridDetails.directDownload = directDownload;
    gridDetails.openLicense = openLicense;
    gridDetails.isAvailable = isAvailable;

    details.grids.append( gridDetails );
  }

#if PROJ_VERSION_MAJOR>6 || (PROJ_VERSION_MAJOR==6 && PROJ_VERSION_MINOR>=2)
  for ( int j = 0; j < proj_concatoperation_get_step_count( pjContext, op ); ++j )
  {
    QgsProjUtils::proj_pj_unique_ptr step( proj_concatoperation_get_step( pjContext, op, j ) );
    if ( step )
    {
      SingleOperationDetails singleOpDetails;
      singleOpDetails.remarks = QString( proj_get_remarks( step.get() ) );
      singleOpDetails.scope = QString( proj_get_scope( step.get() ) );
      singleOpDetails.authority = QString( proj_get_id_auth_name( step.get(), 0 ) );
      singleOpDetails.code = QString( proj_get_id_code( step.get(), 0 ) );

      const char *areaOfUseName = nullptr;
      if ( proj_get_area_of_use( pjContext, step.get(), nullptr, nullptr, nullptr, nullptr, &areaOfUseName ) )
      {
        singleOpDetails.areaOfUse = QString( areaOfUseName );
      }
      details.operationDetails.append( singleOpDetails );
    }
  }
#endif

  return details;
}
#endif
