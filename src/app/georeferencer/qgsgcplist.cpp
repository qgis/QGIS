/***************************************************************************
     qgsgeorefconfigdialog.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointxy.h"
#include "qgsgeorefdatapoint.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsproject.h"

#include "qgsgcplist.h"
#include <QDir>
#include <QTextStream>

void QgsGCPList::createGCPVectors( QVector<QgsPointXY> &sourcePoints, QVector<QgsPointXY> &destinationPoints, const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const
{
  const int targetSize = countEnabledPoints();
  sourcePoints.clear();
  sourcePoints.reserve( targetSize );
  destinationPoints.clear();
  destinationPoints.reserve( targetSize );

  for ( const QgsGeorefDataPoint *pt : std::as_const( *this ) )
  {
    if ( !pt->isEnabled() )
      continue;

    sourcePoints.push_back( pt->sourcePoint() );
    if ( targetCrs.isValid() )
    {
      destinationPoints.push_back( pt->transformedDestinationPoint( targetCrs, context ) );
    }
    else
    {
      destinationPoints.push_back( pt->destinationPoint() );
    }
  }
}

int QgsGCPList::countEnabledPoints() const
{
  if ( isEmpty() )
    return 0;

  int s = 0;
  const_iterator it = begin();
  while ( it != end() )
  {
    if ( ( *it )->isEnabled() )
      s++;
    ++it;
  }
  return s;
}

QList<QgsGcpPoint> QgsGCPList::asPoints() const
{
  QList<QgsGcpPoint> res;
  res.reserve( size() );
  for ( QgsGeorefDataPoint *pt : *this )
  {
    res.append( QgsGcpPoint( pt->point() ) );
  }
  return res;
}

bool QgsGCPList::saveGcps( const QString &filePath, const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context, QString &error ) const
{
  error.clear();

  QFile pointFile( filePath );
  if ( pointFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream points( &pointFile );
    points << QStringLiteral( "#CRS: %1" ).arg( targetCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    points << endl;
#else
    points << Qt::endl;
#endif

    points << "mapX,mapY,sourceX,sourceY,enable,dX,dY,residual";
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    points << endl;
#else
    points << Qt::endl;
#endif

    for ( QgsGeorefDataPoint *pt : *this )
    {
      const QgsPointXY transformedDestinationPoint = pt->transformedDestinationPoint( targetCrs, context );
      points << QStringLiteral( "%1,%2,%3,%4,%5,%6,%7,%8" )
             .arg( qgsDoubleToString( transformedDestinationPoint.x() ),
                   qgsDoubleToString( transformedDestinationPoint.y() ),
                   qgsDoubleToString( pt->sourcePoint().x() ),
                   qgsDoubleToString( pt->sourcePoint().y() ) )
             .arg( pt->isEnabled() )
             .arg( qgsDoubleToString( pt->residual().x() ),
                   qgsDoubleToString( pt->residual().y() ),
                   qgsDoubleToString( std::sqrt( pt->residual().x() * pt->residual().x() + pt->residual().y() * pt->residual().y() ) ) );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      points << endl;
#else
      points << Qt::endl;
#endif
    }
    return true;
  }
  else
  {
    error = QObject::tr( "Could not write to GCP points file %1." ).arg( QDir::toNativeSeparators( filePath ) );
    return false;
  }
}

QList<QgsGcpPoint> QgsGCPList::loadGcps( const QString &filePath, const QgsCoordinateReferenceSystem &defaultDestinationCrs, QString &error )
{
  error.clear();
  QFile pointFile( filePath );
  if ( !pointFile.open( QIODevice::ReadOnly ) )
  {
    error = QObject::tr( "Could not open GCP points file %1." ).arg( QDir::toNativeSeparators( filePath ) );
    return {};
  }

  QTextStream points( &pointFile );
  int lineNumber = 0;
  QString line = points.readLine();
  lineNumber++;

  int i = 0;
  QgsCoordinateReferenceSystem destinationCrs;
  if ( line.contains( QLatin1String( "#CRS: " ) ) )
  {
    destinationCrs = QgsCoordinateReferenceSystem( line.remove( QStringLiteral( "#CRS: " ) ) );
    line = points.readLine();
    lineNumber++;
  }
  else
    destinationCrs = defaultDestinationCrs;

  QList<QgsGcpPoint> res;
  while ( !points.atEnd() )
  {
    line = points.readLine();
    lineNumber++;
    QStringList ls;
    if ( line.contains( ',' ) ) // in previous format "\t" is delimiter of points in new - ","
      ls = line.split( ',' ); // points from new georeferencer
    else
      ls = line.split( '\t' ); // points from prev georeferencer

    if ( ls.count() < 4 )
    {
      error = QObject::tr( "Malformed content at line %1" ).arg( lineNumber );
      return {};
    }

    const QgsPointXY destinationPoint( ls.at( 0 ).toDouble(), ls.at( 1 ).toDouble() ); // map x,y
    const QgsPointXY sourcePoint( ls.at( 2 ).toDouble(), ls.at( 3 ).toDouble() ); // source x,y
    bool enable = true;
    if ( ls.count() >= 5 )
    {
      enable = ls.at( 4 ).toInt();
    }
    res.append( QgsGcpPoint( sourcePoint, destinationPoint, destinationCrs, enable ) );

    ++i;
  }
  return res;
}
