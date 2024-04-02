/***************************************************************************
    qgsjsonutils.h
     -------------
    Date                 : May 206
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsonutils.h"
#include "qgsfeatureiterator.h"
#include "qgsogrutils.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgsproject.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"
#include "qgsapplication.h"
#include "qgsfeatureid.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QTextCodec>
#include <nlohmann/json.hpp>

QgsJsonExporter::QgsJsonExporter( QgsVectorLayer *vectorLayer, int precision )
  : mPrecision( precision )
  , mLayer( vectorLayer )
{
  if ( vectorLayer )
  {
    mCrs = vectorLayer->crs();
    mTransform.setSourceCrs( mCrs );
  }

  // Default 4326
  mDestinationCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  mTransform.setDestinationCrs( mDestinationCrs );
}

void QgsJsonExporter::setVectorLayer( QgsVectorLayer *vectorLayer )
{
  mLayer = vectorLayer;
  if ( vectorLayer )
  {
    mCrs = vectorLayer->crs();
    mTransform.setSourceCrs( mCrs );
  }
}

QgsVectorLayer *QgsJsonExporter::vectorLayer() const
{
  return mLayer.data();
}

void QgsJsonExporter::setSourceCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
  mTransform.setSourceCrs( mCrs );
}

QgsCoordinateReferenceSystem QgsJsonExporter::sourceCrs() const
{
  return mCrs;
}

QString QgsJsonExporter::exportFeature( const QgsFeature &feature, const QVariantMap &extraProperties,
                                        const QVariant &id, int indent ) const
{
  return QString::fromStdString( exportFeatureToJsonObject( feature, extraProperties, id ).dump( indent ) );
}

json QgsJsonExporter::exportFeatureToJsonObject( const QgsFeature &feature, const QVariantMap &extraProperties, const QVariant &id ) const
{
  json featureJson
  {
    {  "type",  "Feature" },
  };
  if ( id.isValid() )
  {
    bool ok = false;
    auto intId = id.toLongLong( &ok );
    if ( ok )
    {
      featureJson["id"] = intId;
    }
    else
    {
      featureJson["id"] = id.toString().toStdString();
    }
  }
  else if ( FID_IS_NULL( feature.id() ) )
  {
    featureJson["id"] = nullptr;
  }
  else
  {
    featureJson["id"] = feature.id();
  }

  QgsGeometry geom = feature.geometry();
  if ( !geom.isNull() && mIncludeGeometry )
  {
    if ( mCrs.isValid() )
    {
      try
      {
        QgsGeometry transformed = geom;
        if ( mTransformGeometries && transformed.transform( mTransform ) == Qgis::GeometryOperationResult::Success )
          geom = transformed;
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse )
      }
    }
    QgsRectangle box = geom.boundingBox();

    if ( QgsWkbTypes::flatType( geom.wkbType() ) != Qgis::WkbType::Point )
    {
      featureJson[ "bbox" ] =
      {
        qgsRound( box.xMinimum(), mPrecision ),
        qgsRound( box.yMinimum(), mPrecision ),
        qgsRound( box.xMaximum(), mPrecision ),
        qgsRound( box.yMaximum(), mPrecision )
      };
    }
    featureJson[ "geometry" ] = geom.asJsonObject( mPrecision );
  }
  else
  {
    featureJson[ "geometry"  ] = nullptr;
  }

  // build up properties element
  json properties;
  if ( mIncludeAttributes || !extraProperties.isEmpty() )
  {
    //read all attribute values from the feature
    if ( mIncludeAttributes )
    {
      QgsFields fields = mLayer ? mLayer->fields() : feature.fields();
      // List of formatters through we want to pass the values
      QStringList formattersAllowList;
      formattersAllowList << QStringLiteral( "KeyValue" )
                          << QStringLiteral( "List" )
                          << QStringLiteral( "ValueRelation" )
                          << QStringLiteral( "ValueMap" );

      for ( int i = 0; i < fields.count(); ++i )
      {
        if ( ( !mAttributeIndexes.isEmpty() && !mAttributeIndexes.contains( i ) ) || mExcludedAttributeIndexes.contains( i ) )
          continue;

        QVariant val = feature.attributes().at( i );

        if ( mLayer )
        {
          const QgsEditorWidgetSetup setup = fields.at( i ).editorWidgetSetup();
          const QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
          if ( formattersAllowList.contains( fieldFormatter->id() ) )
            val = fieldFormatter->representValue( mLayer.data(), i, setup.config(), QVariant(), val );
        }

        QString name = fields.at( i ).name();
        if ( mAttributeDisplayName )
        {
          name = mLayer->attributeDisplayName( i );
        }
        properties[ name.toStdString() ] = QgsJsonUtils::jsonFromVariant( val );
      }
    }

    if ( !extraProperties.isEmpty() )
    {
      QVariantMap::const_iterator it = extraProperties.constBegin();
      for ( ; it != extraProperties.constEnd(); ++it )
      {
        properties[ it.key().toStdString() ] = QgsJsonUtils::jsonFromVariant( it.value() );
      }
    }

    // related attributes
    if ( mLayer && mIncludeRelatedAttributes )
    {
      QList< QgsRelation > relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer.data() );
      for ( const auto &relation : std::as_const( relations ) )
      {
        QgsFeatureRequest req = relation.getRelatedFeaturesRequest( feature );
        req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
        QgsVectorLayer *childLayer = relation.referencingLayer();
        json relatedFeatureAttributes;
        if ( childLayer )
        {
          QgsFeatureIterator it = childLayer->getFeatures( req );
          QVector<QVariant> attributeWidgetCaches;
          int fieldIndex = 0;
          const QgsFields fields { childLayer->fields() };
          for ( const QgsField &field : fields )
          {
            QgsEditorWidgetSetup setup = field.editorWidgetSetup();
            QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
            attributeWidgetCaches.append( fieldFormatter->createCache( childLayer, fieldIndex, setup.config() ) );
            fieldIndex++;
          }
          QgsFeature relatedFet;
          while ( it.nextFeature( relatedFet ) )
          {
            relatedFeatureAttributes += QgsJsonUtils::exportAttributesToJsonObject( relatedFet, childLayer, attributeWidgetCaches );
          }
        }
        properties[ relation.name().toStdString() ] = relatedFeatureAttributes;
      }
    }
  }
  featureJson[ "properties" ] = properties;
  return featureJson;
}

QString QgsJsonExporter::exportFeatures( const QgsFeatureList &features, int indent ) const
{
  return QString::fromStdString( exportFeaturesToJsonObject( features ).dump( indent ) );
}

json QgsJsonExporter::exportFeaturesToJsonObject( const QgsFeatureList &features ) const
{
  json data
  {
    { "type", "FeatureCollection" },
    { "features", json::array() }
  };

  QgsJsonUtils::addCrsInfo( data, mDestinationCrs );

  for ( const QgsFeature &feature : std::as_const( features ) )
  {
    data["features"].push_back( exportFeatureToJsonObject( feature ) );
  }
  return data;
}

void QgsJsonExporter::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mDestinationCrs = destinationCrs;
  mTransform.setDestinationCrs( mDestinationCrs );
}

//
// QgsJsonUtils
//

QgsFeatureList QgsJsonUtils::stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding )
{
  if ( !encoding )
    encoding = QTextCodec::codecForName( "UTF-8" );

  return QgsOgrUtils::stringToFeatureList( string, fields, encoding );
}

QgsFields QgsJsonUtils::stringToFields( const QString &string, QTextCodec *encoding )
{
  if ( !encoding )
    encoding = QTextCodec::codecForName( "UTF-8" );

  return QgsOgrUtils::stringToFields( string, encoding );
}

QString QgsJsonUtils::encodeValue( const QVariant &value )
{
  if ( QgsVariantUtils::isNull( value ) )
    return QStringLiteral( "null" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? "true" : "false";

    case QVariant::StringList:
    case QVariant::List:
    case QVariant::Map:
      return QString::fromUtf8( QJsonDocument::fromVariant( value ).toJson( QJsonDocument::Compact ) );

    default:
    case QVariant::String:
      QString v = value.toString()
                  .replace( '\\', QLatin1String( "\\\\" ) )
                  .replace( '"', QLatin1String( "\\\"" ) )
                  .replace( '\r', QLatin1String( "\\r" ) )
                  .replace( '\b', QLatin1String( "\\b" ) )
                  .replace( '\t', QLatin1String( "\\t" ) )
                  .replace( '/', QLatin1String( "\\/" ) )
                  .replace( '\n', QLatin1String( "\\n" ) );

      return v.prepend( '"' ).append( '"' );
  }
}

QString QgsJsonUtils::exportAttributes( const QgsFeature &feature, QgsVectorLayer *layer, const QVector<QVariant> &attributeWidgetCaches )
{
  QgsFields fields = feature.fields();
  QString attrs;
  for ( int i = 0; i < fields.count(); ++i )
  {
    if ( i > 0 )
      attrs += QLatin1String( ",\n" );

    QVariant val = feature.attributes().at( i );

    if ( layer )
    {
      QgsEditorWidgetSetup setup = layer->fields().at( i ).editorWidgetSetup();
      QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
      if ( fieldFormatter != QgsApplication::fieldFormatterRegistry()->fallbackFieldFormatter() )
        val = fieldFormatter->representValue( layer, i, setup.config(), attributeWidgetCaches.count() >= i ? attributeWidgetCaches.at( i ) : QVariant(), val );
    }

    attrs += encodeValue( fields.at( i ).name() ) + ':' + encodeValue( val );
  }
  return attrs.prepend( '{' ).append( '}' );
}

QVariantList QgsJsonUtils::parseArray( const QString &json, QVariant::Type type )
{
  QString errorMessage;
  QVariantList result;
  try
  {
    const auto jObj( json::parse( json.toStdString() ) );
    if ( ! jObj.is_array() )
    {
      throw json::parse_error::create( 0, 0, QStringLiteral( "JSON value must be an array" ).toStdString() );
    }
    for ( const auto &item : jObj )
    {
      // Create a QVariant from the array item
      QVariant v;
      if ( item.is_number_integer() )
      {
        v = item.get<int>();
      }
      else if ( item.is_number_unsigned() )
      {
        v = item.get<unsigned>();
      }
      else if ( item.is_number_float() )
      {
        // Note: it's a double and not a float on purpose
        v = item.get<double>();
      }
      else if ( item.is_string() )
      {
        v = QString::fromStdString( item.get<std::string>() );
      }
      else if ( item.is_boolean() )
      {
        v = item.get<bool>();
      }
      else if ( item.is_null() )
      {
        // Fallback to int
        v = QVariant( type == QVariant::Type::Invalid ? QVariant::Type::Int : type );
      }

      // If a destination type was specified (it's not invalid), try to convert
      if ( type != QVariant::Invalid )
      {
        if ( ! v.convert( static_cast<int>( type ) ) )
        {
          QgsLogger::warning( QStringLiteral( "Cannot convert json array element to specified type, ignoring: %1" ).arg( v.toString() ) );
        }
        else
        {
          result.push_back( v );
        }
      }
      else
      {
        result.push_back( v );
      }
    }
  }
  catch ( json::parse_error &ex )
  {
    errorMessage = ex.what();
    QgsLogger::warning( QStringLiteral( "Cannot parse json (%1): %2" ).arg( ex.what(), json ) );
  }

  return result;
}

std::unique_ptr< QgsPoint> parsePointFromGeoJson( const json &coords )
{
  if ( !coords.is_array() || coords.size() < 2 || coords.size() > 3 )
  {
    QgsDebugError( QStringLiteral( "JSON Point geometry coordinates must be an array of two or three numbers" ) );
    return nullptr;
  }

  const double x = coords[0].get< double >();
  const double y = coords[1].get< double >();
  if ( coords.size() == 2 )
  {
    return std::make_unique< QgsPoint >( x, y );
  }
  else
  {
    const double z = coords[2].get< double >();
    return std::make_unique< QgsPoint >( x, y, z );
  }
}

std::unique_ptr< QgsLineString> parseLineStringFromGeoJson( const json &coords )
{
  if ( !coords.is_array() || coords.size() < 2 )
  {
    QgsDebugError( QStringLiteral( "JSON LineString geometry coordinates must be an array of at least two points" ) );
    return nullptr;
  }

  const std::size_t coordsSize = coords.size();

  QVector< double > x;
  QVector< double > y;
  QVector< double > z;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  x.resize( static_cast< int >( coordsSize ) );
  y.resize( static_cast< int >( coordsSize ) );
  z.resize( static_cast< int >( coordsSize ) );
#else
  x.resize( coordsSize );
  y.resize( coordsSize );
  z.resize( coordsSize );
#endif
  double *xOut = x.data();
  double *yOut = y.data();
  double *zOut = z.data();
  bool hasZ = false;
  for ( const auto &coord : coords )
  {
    if ( !coord.is_array() || coord.size() < 2 || coord.size() > 3 )
    {
      QgsDebugError( QStringLiteral( "JSON LineString geometry coordinates must be an array of two or three numbers" ) );
      return nullptr;
    }

    *xOut++ = coord[0].get< double >();
    *yOut++ = coord[1].get< double >();
    if ( coord.size() == 3 )
    {
      *zOut++ = coord[2].get< double >();
      hasZ = true;
    }
    else
    {
      *zOut++ = std::numeric_limits< double >::quiet_NaN();
    }
  }

  return std::make_unique< QgsLineString >( x, y, hasZ ? z : QVector<double>() );
}

std::unique_ptr< QgsPolygon > parsePolygonFromGeoJson( const json &coords )
{
  if ( !coords.is_array() || coords.size() < 1 )
  {
    QgsDebugError( QStringLiteral( "JSON Polygon geometry coordinates must be an array" ) );
    return nullptr;
  }

  const std::size_t coordsSize = coords.size();
  std::unique_ptr< QgsLineString > exterior = parseLineStringFromGeoJson( coords[0] );
  if ( !exterior )
  {
    return nullptr;
  }

  std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >( exterior.release() );
  for ( std::size_t i = 1; i < coordsSize; ++i )
  {
    std::unique_ptr< QgsLineString > ring = parseLineStringFromGeoJson( coords[i] );
    if ( !ring )
    {
      return nullptr;
    }
    polygon->addInteriorRing( ring.release() );
  }
  return polygon;
}

std::unique_ptr< QgsAbstractGeometry > parseGeometryFromGeoJson( const json &geometry )
{
  if ( !geometry.is_object() )
  {
    QgsDebugError( QStringLiteral( "JSON geometry value must be an object" ) );
    return nullptr;
  }

  if ( !geometry.contains( "type" ) )
  {
    QgsDebugError( QStringLiteral( "JSON geometry must contain 'type'" ) );
    return nullptr;
  }

  const QString type = QString::fromStdString( geometry["type"].get< std::string >() );
  if ( type.compare( QLatin1String( "Point" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "coordinates" ) )
    {
      QgsDebugError( QStringLiteral( "JSON Point geometry must contain 'coordinates'" ) );
      return nullptr;
    }

    const json &coords = geometry["coordinates"];
    return parsePointFromGeoJson( coords );
  }
  else if ( type.compare( QLatin1String( "MultiPoint" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "coordinates" ) )
    {
      QgsDebugError( QStringLiteral( "JSON MultiPoint geometry must contain 'coordinates'" ) );
      return nullptr;
    }

    const json &coords = geometry["coordinates"];

    if ( !coords.is_array() )
    {
      QgsDebugError( QStringLiteral( "JSON MultiPoint geometry coordinates must be an array" ) );
      return nullptr;
    }

    std::unique_ptr< QgsMultiPoint > multiPoint = std::make_unique< QgsMultiPoint >();
    multiPoint->reserve( static_cast< int >( coords.size() ) );
    for ( const auto &pointCoords : coords )
    {
      std::unique_ptr< QgsPoint > point = parsePointFromGeoJson( pointCoords );
      if ( !point )
      {
        return nullptr;
      }
      multiPoint->addGeometry( point.release() );
    }

    return multiPoint;
  }
  else if ( type.compare( QLatin1String( "LineString" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "coordinates" ) )
    {
      QgsDebugError( QStringLiteral( "JSON LineString geometry must contain 'coordinates'" ) );
      return nullptr;
    }

    const json &coords = geometry["coordinates"];
    return parseLineStringFromGeoJson( coords );
  }
  else if ( type.compare( QLatin1String( "MultiLineString" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "coordinates" ) )
    {
      QgsDebugError( QStringLiteral( "JSON MultiLineString geometry must contain 'coordinates'" ) );
      return nullptr;
    }

    const json &coords = geometry["coordinates"];

    if ( !coords.is_array() )
    {
      QgsDebugError( QStringLiteral( "JSON MultiLineString geometry coordinates must be an array" ) );
      return nullptr;
    }

    std::unique_ptr< QgsMultiLineString > multiLineString = std::make_unique< QgsMultiLineString >();
    multiLineString->reserve( static_cast< int >( coords.size() ) );
    for ( const auto &lineCoords : coords )
    {
      std::unique_ptr< QgsLineString > line = parseLineStringFromGeoJson( lineCoords );
      if ( !line )
      {
        return nullptr;
      }
      multiLineString->addGeometry( line.release() );
    }

    return multiLineString;
  }
  else if ( type.compare( QLatin1String( "Polygon" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "coordinates" ) )
    {
      QgsDebugError( QStringLiteral( "JSON Polygon geometry must contain 'coordinates'" ) );
      return nullptr;
    }

    const json &coords = geometry["coordinates"];
    if ( !coords.is_array() || coords.size() < 1 )
    {
      QgsDebugError( QStringLiteral( "JSON Polygon geometry coordinates must be an array of at least one ring" ) );
      return nullptr;
    }

    return parsePolygonFromGeoJson( coords );
  }
  else if ( type.compare( QLatin1String( "MultiPolygon" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "coordinates" ) )
    {
      QgsDebugError( QStringLiteral( "JSON MultiPolygon geometry must contain 'coordinates'" ) );
      return nullptr;
    }

    const json &coords = geometry["coordinates"];

    if ( !coords.is_array() )
    {
      QgsDebugError( QStringLiteral( "JSON MultiPolygon geometry coordinates must be an array" ) );
      return nullptr;
    }

    std::unique_ptr< QgsMultiPolygon > multiPolygon = std::make_unique< QgsMultiPolygon >();
    multiPolygon->reserve( static_cast< int >( coords.size() ) );
    for ( const auto &polygonCoords : coords )
    {
      std::unique_ptr< QgsPolygon > polygon = parsePolygonFromGeoJson( polygonCoords );
      if ( !polygon )
      {
        return nullptr;
      }
      multiPolygon->addGeometry( polygon.release() );
    }

    return multiPolygon;
  }
  else if ( type.compare( QLatin1String( "GeometryCollection" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( !geometry.contains( "geometries" ) )
    {
      QgsDebugError( QStringLiteral( "JSON GeometryCollection geometry must contain 'geometries'" ) );
      return nullptr;
    }

    const json &geometries = geometry["geometries"];

    if ( !geometries.is_array() )
    {
      QgsDebugError( QStringLiteral( "JSON GeometryCollection geometries must be an array" ) );
      return nullptr;
    }

    std::unique_ptr< QgsGeometryCollection > collection = std::make_unique< QgsGeometryCollection >();
    collection->reserve( static_cast< int >( geometries.size() ) );
    for ( const auto &geometry : geometries )
    {
      std::unique_ptr< QgsAbstractGeometry > object = parseGeometryFromGeoJson( geometry );
      if ( !object )
      {
        return nullptr;
      }
      collection->addGeometry( object.release() );
    }

    return collection;
  }

  QgsDebugError( QStringLiteral( "Unhandled GeoJSON geometry type: %1" ).arg( type ) );
  return nullptr;
}

QgsGeometry QgsJsonUtils::geometryFromGeoJson( const json &geometry )
{
  if ( !geometry.is_object() )
  {
    QgsDebugError( QStringLiteral( "JSON geometry value must be an object" ) );
    return QgsGeometry();
  }

  return QgsGeometry( parseGeometryFromGeoJson( geometry ) );
}

QgsGeometry QgsJsonUtils::geometryFromGeoJson( const QString &geometry )
{
  try
  {
    const auto jObj( json::parse( geometry.toStdString() ) );
    return geometryFromGeoJson( jObj );
  }
  catch ( json::parse_error &ex )
  {
    QgsDebugError( QStringLiteral( "Cannot parse json (%1): %2" ).arg( geometry, ex.what() ) );
    return QgsGeometry();
  }
}

json QgsJsonUtils::jsonFromVariant( const QVariant &val )
{
  if ( QgsVariantUtils::isNull( val ) )
  {
    return nullptr;
  }
  json j;
  if ( val.type() == QVariant::Type::Map )
  {
    const QVariantMap &vMap = val.toMap();
    json jMap = json::object();
    for ( auto it = vMap.constBegin(); it != vMap.constEnd(); it++ )
    {
      jMap[ it.key().toStdString() ] = jsonFromVariant( it.value() );
    }
    j = jMap;
  }
  else if ( val.type() == QVariant::Type::List || val.type() == QVariant::Type::StringList )
  {
    const QVariantList &vList = val.toList();
    json jList = json::array();
    for ( const auto &v : vList )
    {
      jList.push_back( jsonFromVariant( v ) );
    }
    j = jList;
  }
  else
  {
    switch ( val.userType() )
    {
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
        j = val.toLongLong();
        break;
      case QMetaType::Double:
      case QMetaType::Float:
        j = val.toDouble();
        break;
      case QMetaType::Bool:
        j = val.toBool();
        break;
      case QMetaType::QByteArray:
        j = val.toByteArray().toBase64().toStdString();
        break;
      default:
        j = val.toString().toStdString();
        break;
    }
  }
  return j;
}

QVariant QgsJsonUtils::parseJson( const std::string &jsonString )
{
  QString error;
  const QVariant res = parseJson( jsonString, error );

  if ( !error.isEmpty() )
  {
    QgsLogger::warning( QStringLiteral( "Cannot parse json (%1): %2" ).arg( error,
                        QString::fromStdString( jsonString ) ) );
  }
  return res;
}

QVariant QgsJsonUtils::parseJson( const std::string &jsonString, QString &error )
{
  error.clear();
  try
  {
    const json j = json::parse( jsonString );
    return jsonToVariant( j );
  }
  catch ( json::parse_error &ex )
  {
    error = QString::fromStdString( ex.what() );
  }
  return QVariant();
}

QVariant QgsJsonUtils::jsonToVariant( const json &value )
{
  // tracks whether entire json string is a primitive
  bool isPrimitive = true;

  std::function<QVariant( json )> _parser { [ & ]( json jObj ) -> QVariant {
      QVariant result;
      if ( jObj.is_array() )
      {
        isPrimitive = false;
        QVariantList results;
        results.reserve( jObj.size() );
        for ( const auto &item : jObj )
        {
          results.push_back( _parser( item ) );
        }
        result = results;
      }
      else if ( jObj.is_object() )
      {
        isPrimitive = false;
        QVariantMap results;
        for ( const auto  &item : jObj.items() )
        {
          const auto key { QString::fromStdString( item.key() ) };
          const auto value {  _parser( item.value() ) };
          results[ key ] = value;
        }
        result = results;
      }
      else
      {
        if ( jObj.is_number_unsigned() )
        {
          // Try signed int and long long first, fall back
          // onto unsigned long long
          const qulonglong num { jObj.get<qulonglong>() };
          if ( num <= std::numeric_limits<int>::max() )
          {
            result = static_cast<int>( num );
          }
          else if ( num <= std::numeric_limits<qlonglong>::max() )
          {
            result = static_cast<qlonglong>( num );
          }
          else
          {
            result = num;
          }
        }
        else if ( jObj.is_number_integer() )
        {
          const qlonglong num { jObj.get<qlonglong>() };
          if ( num <= std::numeric_limits<int>::max() && num >= std::numeric_limits<int>::lowest() )
          {
            result = static_cast<int>( num );
          }
          else
          {
            result = num;
          }
        }
        else if ( jObj.is_boolean() )
        {
          result = jObj.get<bool>();
        }
        else if ( jObj.is_number_float() )
        {
          // Note: it's a double and not a float on purpose
          result = jObj.get<double>();
        }
        else if ( jObj.is_string() )
        {
          if ( isPrimitive && jObj.get<std::string>().length() == 0 )
          {
            result = QString::fromStdString( jObj.get<std::string>() ).append( "\"" ).insert( 0, "\"" );
          }
          else
          {
            result = QString::fromStdString( jObj.get<std::string>() );
          }
        }
        else if ( jObj.is_null() )
        {
          // Do nothing (leave invalid)
        }
      }
      return result;
    }
  };

  return _parser( value );
}

QVariant QgsJsonUtils::parseJson( const QString &jsonString )
{
  return parseJson( jsonString.toStdString() );
}

json QgsJsonUtils::exportAttributesToJsonObject( const QgsFeature &feature, QgsVectorLayer *layer, const QVector<QVariant> &attributeWidgetCaches )
{
  QgsFields fields = feature.fields();
  json attrs;
  for ( int i = 0; i < fields.count(); ++i )
  {
    QVariant val = feature.attributes().at( i );

    if ( layer )
    {
      QgsEditorWidgetSetup setup = layer->fields().at( i ).editorWidgetSetup();
      QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
      if ( fieldFormatter != QgsApplication::fieldFormatterRegistry()->fallbackFieldFormatter() )
        val = fieldFormatter->representValue( layer, i, setup.config(), attributeWidgetCaches.count() >= i ? attributeWidgetCaches.at( i ) : QVariant(), val );
    }
    attrs[fields.at( i ).name().toStdString()] = jsonFromVariant( val );
  }
  return attrs;
}

void QgsJsonUtils::addCrsInfo( json &value, const QgsCoordinateReferenceSystem &crs )
{
  // When user request EPSG:4326 we return a compliant CRS84 lon/lat GeoJSON
  // so no need to add CRS information
  if ( crs.authid() == "OGC:CRS84" || crs.authid() == "EPSG:4326" )
    return;

  value["crs"]["type"] = "name";
  value["crs"]["properties"]["name"] = crs.toOgcUrn().toStdString();
}
