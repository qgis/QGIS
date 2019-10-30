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

QList< QgsVectorLayerServerProperties::WmsDimensionInfo > QgsServerApiUtils::temporalDimensions( const QgsVectorLayer *layer )
{
  QList< QgsVectorLayerServerProperties::WmsDimensionInfo > dimensions { layer->serverProperties()->wmsDimensions() };
  // Filter only date and time
  dimensions.erase( std::remove_if( dimensions.begin(),
                                    dimensions.end(),
                                    [ ]( QgsVectorLayerServerProperties::WmsDimensionInfo & dim )
  {
    return dim.name.toLower() != QStringLiteral( "time" )
           && dim.name.toLower() != QStringLiteral( "date" ) ;
  } ), dimensions.end() );

  // Automatically pick up the first date/datetime field if dimensions is empty
  if ( dimensions.isEmpty() )
  {
    const auto constFields { layer->fields() };
    for ( const auto &f : constFields )
    {
      if ( f.isDateOrTime() )
      {
        dimensions.append( QgsVectorLayerServerProperties::WmsDimensionInfo( f.type() == QVariant::DateTime ?
                           QStringLiteral( "time" ) :
                           QStringLiteral( "date" ), f.name() ) );
        break;
      }
    }
  }
  return dimensions;
}

///@cond PRIVATE
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
  if ( parts.length() != 2 )
  {
    throw QgsServerApiBadRequestException( QStringLiteral( "%1 is not a valid datetime interval." ).arg( interval ), QStringLiteral( "Server" ), Qgis::Critical );
  }
  T result { parseDate( parts[0] ), parseDate( parts[1] ) };
  // Check validity
  if ( result.isEmpty() )
  {
    throw QgsServerApiBadRequestException( QStringLiteral( "%1 is not a valid datetime interval (empty)." ).arg( interval ), QStringLiteral( "Server" ), Qgis::Critical );
  }
  return result;
}
/// @endcond

QgsDateRange QgsServerApiUtils::parseTemporalDateInterval( const QString &interval )
{
  return QgsServerApiUtils::parseTemporalInterval<QgsDateRange, QDate>( interval );
}

QgsDateTimeRange QgsServerApiUtils::parseTemporalDateTimeInterval( const QString &interval )
{
  return QgsServerApiUtils::parseTemporalInterval<QgsDateTimeRange, QDateTime>( interval );
}

QgsExpression QgsServerApiUtils::temporalFilterExpression( const QgsVectorLayer *layer, const QString &interval )
{
  QgsExpression expression;
  QStringList conditions;

  const auto dimensions { QgsServerApiUtils::temporalDimensions( layer ) };
  if ( dimensions.isEmpty() )
  {
    return expression;
  }

  // helper to get the field type from the field name
  auto fieldTypeFromName = [ & ]( const QString & fieldName, const QgsVectorLayer * layer ) -> QVariant::Type
  {
    int fieldIdx { layer->fields().lookupField( fieldName ) };
    if ( fieldIdx < 0 )
    {
      return QVariant::Type::Invalid;
    }
    const QgsField field { layer->fields().at( fieldIdx ) };
    QString fieldRefValue;
    return field.type();
  };

  // helper to cast the field value
  auto refFieldCast = [ & ]( const QString & fieldName, QVariant::Type queryType, QVariant::Type fieldType ) -> QString
  {

    const auto fieldRealType { fieldTypeFromName( fieldName, layer ) };
    if ( fieldRealType == QVariant::Type::Invalid )
    {
      return QString();
    }

    QString fieldRefValue;
    // Downcast only datetime -> date
    // always cast strings
    if ( fieldRealType == QVariant::Type::String )
    {
      // Cast to query type but only downcast
      if ( fieldType != queryType || fieldType == QVariant::Type::Date )
      {
        return QStringLiteral( "to_date( %1 )" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
      }
      else
      {
        return QStringLiteral( "%2( %1 )" ).arg( QgsExpression::quotedColumnRef( fieldName ) )
               .arg( queryType == QVariant::Type::Date ? QStringLiteral( "to_date" ) : QStringLiteral( "to_datetime" ) );
      }
    }
    else if ( fieldType == queryType || fieldType == QVariant::Type::Date )
    {
      return QgsExpression::quotedColumnRef( fieldName );
    }
    else
    {
      return QStringLiteral( "%2( %1 )" ).arg( QgsExpression::quotedColumnRef( fieldName ) )
             .arg( queryType == QVariant::Type::Date ? QStringLiteral( "to_date" ) : QStringLiteral( "to_datetime" ) );
    }
  };

  // Quote and cast a query value
  auto quoteValue = [ ]( const QString & value ) -> QString
  {
    if ( value.length() == 10 )
    {
      return QStringLiteral( "to_date( %1 )" ).arg( QgsExpression::quotedValue( value ) );
    }
    else
    {
      return QStringLiteral( "to_datetime( %1 )" ).arg( QgsExpression::quotedValue( value ) );
    }
  };

  // helper to build the interval filter, fieldType is the underlying field type, queryType is the input query type
  auto makeFilter = [ &quoteValue ]( const QString & fieldBegin, const QString & fieldEnd,
                                     const QString & fieldBeginCasted, const QString & fieldEndCasted,
                                     const QString & queryBegin, const QString & queryEnd ) -> QString
  {
    QString result;

    // It's a closed interval query, go for overlap
    if ( ! queryBegin.isEmpty() && ! queryEnd.isEmpty() )
    {
      // Overlap of two intervals
      if ( ! fieldEndCasted.isEmpty() )
      {
        result = QStringLiteral( "( %1 IS NULL OR %2 <= %6 ) AND ( %4 IS NULL OR %5 >= %3 )" )
        .arg( fieldBegin,
              fieldBeginCasted,
              quoteValue( queryBegin ),
              fieldEnd,
              fieldEndCasted,
              quoteValue( queryEnd ) );
      }
      else // Overlap of single value
      {
        result = QStringLiteral( "( %1 IS NULL OR ( %2 <= %3 AND %3 <= %4 ) )" )
                 .arg( fieldBegin,
                       quoteValue( queryBegin ),
                       fieldBeginCasted,
                       quoteValue( queryEnd ) );
      }

    }
    else if ( ! queryBegin.isEmpty() ) // >=
    {
      if ( ! fieldEndCasted.isEmpty() )
      {
        result = QStringLiteral( "( %1 IS NULL OR %2 >= %3 )" ).arg( fieldEnd, fieldEndCasted, quoteValue( queryBegin ) );
      }
      else
      {
        result = QStringLiteral( "( %1 IS NULL OR %2 >= %3 )" ).arg( fieldBegin, fieldBeginCasted, quoteValue( queryBegin ) );
      }
    }
    else // <=
    {
      result = QStringLiteral( "( %1 IS NULL OR %2 <= %3 )" ).arg( fieldBegin, fieldBeginCasted, quoteValue( queryEnd ) );
    }
    return result;
  };

  // Determine if it is a date or a datetime interval (mixed are not supported)
  QString testType { interval };
  if ( interval.contains( '/' ) )
  {
    const QStringList parts { interval.split( '/' ) };
    testType = parts[0];
    if ( testType.isEmpty() || testType == QStringLiteral( ".." ) )
    {
      testType = parts[1];
    }
  }
  // Determine query input type: datetime is always longer than 10 chars
  const bool inputQueryIsDateTime { testType.length() > 10 };
  const QVariant::Type queryType { inputQueryIsDateTime ? QVariant::Type::DateTime : QVariant::Type::Date };

  // Is it an interval?
  if ( interval.contains( '/' ) )
  {
    if ( ! inputQueryIsDateTime )
    {
      QgsDateRange dateInterval { QgsServerApiUtils::parseTemporalDateInterval( interval ) };

      for ( const auto &dimension : qgis::as_const( dimensions ) )
      {

        // Determine the field type from the dimension name "time"/"date"
        const QVariant::Type fieldType { dimension.name.toLower() == QStringLiteral( "time" ) ? QVariant::Type::DateTime :  QVariant::Type::Date };

        const auto fieldBeginCasted { refFieldCast( dimension.fieldName, queryType, fieldType ) };
        if ( fieldBeginCasted.isEmpty() )
        {
          continue;
        }

        const auto fieldBegin = QgsExpression::quotedColumnRef( dimension.fieldName );
        const auto fieldEnd = QgsExpression::quotedColumnRef( dimension.endFieldName );

        // This may be empty:
        const auto fieldEndCasted { refFieldCast( dimension.endFieldName, queryType, fieldType ) };
        if ( ! dateInterval.begin().isValid( ) && ! dateInterval.end().isValid( ) )
        {
          // Nothing to do here: log?
        }
        else
        {
          conditions.push_back( makeFilter( fieldBegin,
                                            fieldEnd,
                                            fieldBeginCasted,
                                            fieldEndCasted,
                                            dateInterval.begin().toString( Qt::DateFormat::ISODate ),
                                            dateInterval.end().toString( Qt::DateFormat::ISODate ) ) );

        }
      }
    }
    else // try datetime
    {
      QgsDateTimeRange dateTimeInterval { QgsServerApiUtils::parseTemporalDateTimeInterval( interval ) };
      for ( const auto &dimension : qgis::as_const( dimensions ) )
      {

        // Determine the field type from the dimension name "time"/"date"
        const QVariant::Type fieldType { dimension.name.toLower() == QStringLiteral( "time" ) ? QVariant::Type::DateTime :  QVariant::Type::Date };

        const auto fieldfBeginCasted { refFieldCast( dimension.fieldName, queryType, fieldType ) };
        if ( fieldfBeginCasted.isEmpty() )
        {
          continue;
        }
        const auto fieldBegin = QgsExpression::quotedColumnRef( dimension.fieldName );
        const auto fieldEnd = QgsExpression::quotedColumnRef( dimension.endFieldName );

        // This may be empty:
        const auto fieldEndCasted { refFieldCast( dimension.endFieldName, queryType, fieldType ) };
        if ( ! dateTimeInterval.begin().isValid( ) && ! dateTimeInterval.end().isValid( ) )
        {
          // Nothing to do here: log?
        }
        else
        {
          // Cast the query value according to the field type
          QString beginQuery;
          QString endQuery;
          // Drop the time
          if ( fieldType == QVariant::Type::Date )
          {
            beginQuery = dateTimeInterval.begin().date().toString( Qt::DateFormat::ISODate );
            endQuery = dateTimeInterval.end().date().toString( Qt::DateFormat::ISODate );
          }
          else
          {
            beginQuery = dateTimeInterval.begin().toString( Qt::DateFormat::ISODate );
            endQuery = dateTimeInterval.end().toString( Qt::DateFormat::ISODate );
          }
          conditions.push_back( makeFilter( fieldBegin,
                                            fieldEnd,
                                            fieldfBeginCasted,
                                            fieldEndCasted,
                                            beginQuery,
                                            endQuery ) );
        }
      }
    }
  }
  else // single value
  {

    for ( const auto &dimension : qgis::as_const( dimensions ) )
    {
      // Determine the field type from the dimension name "time"/"date"
      const bool fieldIsDateTime { dimension.name.toLower() == QStringLiteral( "time" ) };
      const QVariant::Type fieldType { fieldIsDateTime ? QVariant::Type::DateTime :  QVariant::Type::Date };

      const auto fieldRefBegin { refFieldCast( dimension.fieldName, queryType, fieldType ) };
      if ( fieldRefBegin.isEmpty() )
      {
        continue;
      }
      const auto fieldBegin = QgsExpression::quotedColumnRef( dimension.fieldName );

      // This may be empty:
      const auto fieldRefEnd { refFieldCast( dimension.endFieldName, queryType, fieldType ) };
      const auto fieldEnd = QgsExpression::quotedColumnRef( dimension.endFieldName );

      QString condition;
      QString castedValue;

      // field has possibly been downcasted
      if ( ! inputQueryIsDateTime || ! fieldIsDateTime )
      {
        QString castedInterval { interval };
        // Check if we need to downcast interval from datetime
        if ( inputQueryIsDateTime )
        {
          castedInterval = QDate::fromString( castedInterval, Qt::DateFormat::ISODate ).toString( Qt::DateFormat::ISODate );
        }
        castedValue = QStringLiteral( "to_date( %1 )" ).arg( QgsExpression::quotedValue( castedInterval ) );
      }
      else
      {
        QString castedInterval { interval };
        // Check if we need to upcast interval to datetime
        if ( ! inputQueryIsDateTime )
        {
          castedInterval = QDateTime::fromString( castedInterval, Qt::DateFormat::ISODate ).toString( Qt::DateFormat::ISODate );
        }
        castedValue = QStringLiteral( "to_datetime( %1 )" ).arg( QgsExpression::quotedValue( castedInterval ) );
      }

      if ( ! fieldRefEnd.isEmpty() )
      {
        condition = QStringLiteral( "( %1 IS NULL OR %2 <= %3 ) AND ( %5 IS NULL OR %3 <= %4 )" ).arg(
                      fieldBegin,
                      fieldRefBegin,
                      castedValue,
                      fieldRefEnd,
                      fieldEnd );
      }
      else
      {
        condition = QStringLiteral( "( %1 IS NULL OR %2 = %3 )" )
                    .arg( fieldBegin,
                          fieldRefBegin,
                          castedValue );

      }
      conditions.push_back( condition );

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

json QgsServerApiUtils::temporalExtent( const QgsVectorLayer *layer )
{
  // Helper to get min/max from a dimension
  auto range = [ & ]( const QgsVectorLayerServerProperties::WmsDimensionInfo & dimInfo ) -> QgsDateTimeRange
  {
    QgsDateTimeRange result;
    // min
    int fieldIdx { layer->fields().lookupField( dimInfo.fieldName )};
    if ( fieldIdx < 0 )
    {
      return result;
    }
    QDateTime min { layer->minimumValue( fieldIdx ).toDateTime() };
    QDateTime max { layer->maximumValue( fieldIdx ).toDateTime() };
    if ( ! dimInfo.endFieldName.isEmpty() )
    {
      fieldIdx = layer->fields().lookupField( dimInfo.endFieldName );
      if ( fieldIdx >= 0 )
      {
        QDateTime minEnd { layer->minimumValue( fieldIdx ).toDateTime() };
        QDateTime maxEnd { layer->maximumValue( fieldIdx ).toDateTime() };
        if ( minEnd.isValid() )
        {
          min = std::min<QDateTime>( min, layer->minimumValue( fieldIdx ).toDateTime() );
        }
        if ( maxEnd.isValid() )
        {
          max = std::max<QDateTime>( max, layer->maximumValue( fieldIdx ).toDateTime() );
        }
      }
    }
    return { min, max };
  };

  const QList<QgsVectorLayerServerProperties::WmsDimensionInfo> dimensions { QgsServerApiUtils::temporalDimensions( layer ) };
  if ( dimensions.isEmpty() )
  {
    return nullptr;
  }
  else
  {
    try
    {
      QgsDateTimeRange extent;
      bool isFirst = true;
      for ( const auto &dimension : dimensions )
      {
        // Get min/max for dimension
        if ( isFirst )
        {
          extent = range( dimension );
          isFirst = false;
        }
        else
        {
          extent.extend( range( dimension ) );
        }
      }
      json ret = json::array();
      const QString beginVal { extent.begin().toString( Qt::DateFormat::ISODate ) };
      const QString endVal { extent.end().toString( Qt::DateFormat::ISODate ) };
      // We cannot mix nullptr and std::string :(
      if ( beginVal.isEmpty() && endVal.isEmpty() )
      {
        ret.push_back( { nullptr, nullptr } );
      }
      else if ( beginVal.isEmpty() )
      {
        ret.push_back( { nullptr, endVal.toStdString() } );
      }
      else if ( endVal.isEmpty() )
      {
        ret.push_back( { beginVal.toStdString(), nullptr } );
      }
      else
      {
        ret.push_back( { beginVal.toStdString(), endVal.toStdString() } );
      }
      return ret;
    }
    catch ( std::exception &ex )
    {
      const QString errorMessage { QStringLiteral( "Error creating temporal extent: %1" ).arg( ex.what() ) };
      QgsMessageLog::logMessage( errorMessage, QStringLiteral( "Server" ), Qgis::Critical );
      throw  QgsServerApiInternalServerError( errorMessage );
    }
  }
}

QVariantList QgsServerApiUtils::temporalExtentList( const QgsVectorLayer *layer ) SIP_PYNAME( temporalExtent )
{
  QVariantList list;
  list.push_back( QgsJsonUtils::parseArray( QString::fromStdString( temporalExtent( layer )[0].dump() ) ) );
  return list;
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

