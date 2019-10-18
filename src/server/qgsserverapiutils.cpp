/***************************************************************************
                          qgsserverapiutils.cpp

  Class defining utilities for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverapiutils.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsserverprojectutils.h"
#include "qgsmessagelog.h"


#include "nlohmann/json.hpp"

#include <QUrl>

QgsRectangle QgsServerApiUtils::parseBbox( const QString &bbox )
{
  const auto parts { bbox.split( ',', QString::SplitBehavior::SkipEmptyParts ) };
  // Note: Z is ignored
  auto ok { true };
  if ( parts.count() == 4 ||  parts.count() == 6 )
  {
    const auto hasZ { parts.count() == 6 };
    auto toDouble = [ & ]( const int i ) -> double
    {
      if ( ! ok )
        return 0;
      return parts[i].toDouble( &ok );
    };
    QgsRectangle rect;
    if ( hasZ )
    {
      rect = QgsRectangle( toDouble( 0 ), toDouble( 1 ),
                           toDouble( 3 ), toDouble( 4 ) );
    }
    else
    {
      rect = QgsRectangle( toDouble( 0 ), toDouble( 1 ),
                           toDouble( 2 ), toDouble( 3 ) );
    }
    if ( ok )
    {
      return rect;
    }
  }
  return QgsRectangle();
}


template<typename T, class T2> T QgsServerApiUtils::parseTemporalInterval( const QString &interval )
{
  auto parseDate = [ ]( const QString & date ) -> T2
  {
    T2 result;
    if ( date == QStringLiteral( ".." ) || date.isEmpty() )
    {
      return result;
    }
    else
    {
      T2 result { T2::fromString( date, Qt::DateFormat::ISODate ) };
      if ( !result.isValid() )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "%1 is not a valid date/datetime." ).arg( date ) );
      }
      return result;
    }
  };
  const QStringList parts { interval.split( '/' ) };
  return { parseDate( parts[0] ), parseDate( parts[1] ) };

}

QgsServerApiUtils::TemporalDateInterval QgsServerApiUtils::parseTemporalDateInterval( const QString &interval )
{
  return QgsServerApiUtils::parseTemporalInterval<QgsServerApiUtils::TemporalDateInterval, QDate>( interval );
}

QgsServerApiUtils::TemporalDateTimeInterval QgsServerApiUtils::parseTemporalDateTimeInterval( const QString &interval )
{
  return QgsServerApiUtils::parseTemporalInterval<QgsServerApiUtils::TemporalDateTimeInterval, QDateTime>( interval );
}


QgsExpression QgsServerApiUtils::temporalFilterExpression( const QgsVectorLayer *layer, const QString &interval )
{
  QgsExpression expression;
  QStringList conditions;

  // Is it an interval?
  if ( interval.contains( '/' ) )
  {
    // Try date first
    try
    {
      TemporalDateInterval dateInterval { QgsServerApiUtils::parseTemporalDateInterval( interval ) };
      for ( const auto &fieldName : layer->includeAttributesOapifTemporalFilters() )
      {
        int fieldIdx { layer->fields().lookupField( fieldName ) };
        if ( fieldIdx < 0 )
        {
          continue;
        }
        const QgsField field { layer->fields().at( fieldIdx ) };
        QString fieldValue;
        if ( field.type() == QVariant::Date )
        {
          fieldValue = QStringLiteral( R"raw("%1")raw" ).arg( fieldName );
        }
        else
        {
          fieldValue = QStringLiteral( R"raw(to_date( "%1" ))raw" ).arg( fieldName );
        }

        if ( ! dateInterval.begin.isValid( ) && ! dateInterval.end.isValid( ) )
        {
          // Nothing to do here: log?
        }
        else
        {
          if ( dateInterval.begin.isValid( ) )
          {
            conditions.push_back( QStringLiteral( R"raw(%1 >= to_date('%2'))raw" )
                                  .arg( fieldValue )
                                  .arg( dateInterval.begin.toString( Qt::DateFormat::ISODate ) ) );

          }
          if ( dateInterval.end.isValid( ) )
          {
            conditions.push_back( QStringLiteral( R"raw(%1 <= to_date('%2'))raw" )
                                  .arg( fieldValue )
                                  .arg( dateInterval.end.toString( Qt::DateFormat::ISODate ) ) );

          }
        }

      }
    }
    catch ( QgsServerApiBadRequestException & )    // try datetime
    {
      TemporalDateTimeInterval dateTimeInterval { QgsServerApiUtils::parseTemporalDateTimeInterval( interval ) };
      for ( const auto &fieldName : layer->includeAttributesOapifTemporalFilters() )
      {
        int fieldIdx { layer->fields().lookupField( fieldName ) };
        if ( fieldIdx < 0 )
        {
          continue;
        }
        const QgsField field { layer->fields().at( fieldIdx ) };
        QString fieldValue;
        if ( field.type() == QVariant::Date )
        {
          fieldValue = QStringLiteral( R"raw("%1")raw" ).arg( fieldName );
        }
        else
        {
          fieldValue = QStringLiteral( R"raw(to_datetime( "%1" ))raw" ).arg( fieldName );
        }

        if ( ! dateTimeInterval.begin.isValid( ) && ! dateTimeInterval.end.isValid( ) )
        {
          // Nothing to do here: log?
        }
        else
        {
          if ( dateTimeInterval.begin.isValid( ) )
          {
            conditions.push_back( QStringLiteral( R"raw(%1 >= to_datetime('%2'))raw" )
                                  .arg( fieldValue )
                                  .arg( dateTimeInterval.begin.toString( Qt::DateFormat::ISODate ) ) );

          }
          if ( dateTimeInterval.end.isValid( ) )
          {
            conditions.push_back( QStringLiteral( R"raw(%1 <= to_datetime('%2'))raw" )
                                  .arg( fieldValue )
                                  .arg( dateTimeInterval.end.toString( Qt::DateFormat::ISODate ) ) );

          }
        }
      }
    }
  }
  else // plain value
  {
    for ( const auto &fieldName : layer->includeAttributesOapifTemporalFilters() )
    {
      int fieldIdx { layer->fields().lookupField( fieldName ) };
      if ( fieldIdx < 0 )
      {
        continue;
      }

      const QgsField field { layer->fields().at( fieldIdx ) };

      if ( field.type() == QVariant::Date )
      {
        conditions.push_back( QStringLiteral( R"raw("%1" = to_date('%2'))raw" )
                              .arg( fieldName )
                              .arg( interval ) );
      }
      else if ( field.type() == QVariant::DateTime )
      {
        conditions.push_back( QStringLiteral( R"raw("%1" = to_datetime('%2'))raw" )
                              .arg( fieldName )
                              .arg( interval ) );
      }
      else
      {
        // Guess the type from input
        QDateTime dateTime { QDateTime::fromString( interval, Qt::DateFormat::ISODate )};
        if ( dateTime.isValid() )
        {
          conditions.push_back( QStringLiteral( R"raw(to_datetime("%1") = to_datetime('%2'))raw" )
                                .arg( fieldName )
                                .arg( interval ) );
        }
        else
        {
          QDate date { QDate::fromString( interval, Qt::DateFormat::ISODate )};
          if ( date.isValid() )
          {
            conditions.push_back( QStringLiteral( R"raw(to_date("%1") = to_date('%2'))raw" )
                                  .arg( fieldName )
                                  .arg( interval ) );
          }
          else
          {
            // Nothing done here, log?
          }
        }
      }
    }
  }
  if ( ! conditions.isEmpty() )
  {
    expression.setExpression( conditions.join( QStringLiteral( " AND " ) ) );
  }
  return expression;
}

json QgsServerApiUtils::layerExtent( const QgsVectorLayer *layer )
{
  auto extent { layer->extent() };
  if ( layer->crs().postgisSrid() != 4326 )
  {
    static const QgsCoordinateReferenceSystem targetCrs { 4326 };
    const QgsCoordinateTransform ct( layer->crs(), targetCrs, layer->transformContext() );
    extent = ct.transform( extent );
  }
  return {{ extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() }};
}

QgsCoordinateReferenceSystem QgsServerApiUtils::parseCrs( const QString &bboxCrs )
{
  QgsCoordinateReferenceSystem crs;
  // We get this:
  // http://www.opengis.net/def/crs/OGC/1.3/CRS84
  // We want this:
  // "urn:ogc:def:crs:<auth>:[<version>]:<code>"
  const auto parts { QUrl( bboxCrs ).path().split( '/' ) };
  if ( parts.count() == 6 )
  {
    return crs.fromOgcWmsCrs( QStringLiteral( "urn:ogc:def:crs:%1:%2:%3" ).arg( parts[3], parts[4], parts[5] ) );
  }
  else
  {
    return crs;
  }
}

const QVector<QgsMapLayer *> QgsServerApiUtils::publishedWfsLayers( const QgsProject *project )
{
  const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
  const QStringList wfstUpdateLayersId = QgsServerProjectUtils::wfstUpdateLayerIds( *project );
  const QStringList wfstInsertLayersId = QgsServerProjectUtils::wfstInsertLayerIds( *project );
  const QStringList wfstDeleteLayersId = QgsServerProjectUtils::wfstDeleteLayerIds( *project );
  QVector<QgsMapLayer *> result;
  const auto constLayers { project->mapLayers() };
  for ( auto it = project->mapLayers().constBegin(); it !=  project->mapLayers().constEnd(); it++ )
  {
    if ( wfstUpdateLayersId.contains( it.value()->id() ) ||
         wfstInsertLayersId.contains( it.value()->id() ) ||
         wfstDeleteLayersId.contains( it.value()->id() ) )
    {
      result.push_back( it.value() );
    }

  }
  return result;
}

QString QgsServerApiUtils::sanitizedFieldValue( const QString &value )
{
  QString result { QUrl( value ).toString() };
  static const QRegularExpression re( R"raw(;.*(DROP|DELETE|INSERT|UPDATE|CREATE|INTO))raw" );
  if ( re.match( result.toUpper() ).hasMatch() )
  {
    result = QString();
  }
  return result.replace( '\'', QStringLiteral( "\'" ) );
}

QStringList QgsServerApiUtils::publishedCrsList( const QgsProject *project )
{
  // This must be always available in OGC APIs
  QStringList result { { QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" )}};
  if ( project )
  {
    const QStringList outputCrsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
    for ( const QString &crsId : outputCrsList )
    {
      const auto crsUri { crsToOgcUri( QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsId ) ) };
      if ( ! crsUri.isEmpty() )
      {
        result.push_back( crsUri );
      }
    }
  }
  return result;
}

QString QgsServerApiUtils::crsToOgcUri( const QgsCoordinateReferenceSystem &crs )
{
  const auto parts { crs.authid().split( ':' ) };
  if ( parts.length() == 2 )
  {
    if ( parts[0] == QStringLiteral( "EPSG" ) )
      return  QStringLiteral( "http://www.opengis.net/def/crs/EPSG/9.6.2/%1" ).arg( parts[1] ) ;
    else if ( parts[0] == QStringLiteral( "OGC" ) )
    {
      return  QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/%1" ).arg( parts[1] ) ;
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error converting published CRS to URI %1: (not OGC or EPSG)" ).arg( crs.authid() ), QStringLiteral( "Server" ), Qgis::Critical );
    }
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Error converting published CRS to URI: %1" ).arg( crs.authid() ), QStringLiteral( "Server" ), Qgis::Critical );
  }
  return QString();
}

QString QgsServerApiUtils::appendMapParameter( const QString &path, const QUrl &requestUrl )
{
  QList<QPair<QString, QString> > qi;
  QString result { path };
  const auto constItems { requestUrl.queryItems( ) };
  for ( const auto &i : constItems )
  {
    if ( i.first.compare( QStringLiteral( "MAP" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      qi.push_back( i );
    }
  }
  if ( ! qi.empty() )
  {
    if ( ! path.endsWith( '?' ) )
    {
      result += '?';
    }
    result.append( QStringLiteral( "MAP=%1" ).arg( qi.first().second ) );
  }
  return result;
}

