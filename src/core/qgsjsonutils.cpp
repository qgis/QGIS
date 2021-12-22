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
  mTransform.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
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

    if ( QgsWkbTypes::flatType( geom.wkbType() ) != QgsWkbTypes::Point )
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
  int attributeCounter { 0 };
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
        attributeCounter++;
      }
    }

    if ( !extraProperties.isEmpty() )
    {
      QVariantMap::const_iterator it = extraProperties.constBegin();
      for ( ; it != extraProperties.constEnd(); ++it )
      {
        properties[ it.key().toStdString() ] = QgsJsonUtils::jsonFromVariant( it.value() );
        attributeCounter++;
      }
    }

    // related attributes
    if ( mLayer && mIncludeRelatedAttributes )
    {
      QList< QgsRelation > relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer.data() );
      for ( const auto &relation : std::as_const( relations ) )
      {
        QgsFeatureRequest req = relation.getRelatedFeaturesRequest( feature );
        req.setFlags( QgsFeatureRequest::NoGeometry );
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
        attributeCounter++;
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
  for ( const QgsFeature &feature : std::as_const( features ) )
  {
    data["features"].push_back( exportFeatureToJsonObject( feature ) );
  }
  return data;
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
  if ( value.isNull() )
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

json QgsJsonUtils::jsonFromVariant( const QVariant &val )
{
  if ( val.isNull() || ! val.isValid() )
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
        if ( jObj.is_number_integer() )
        {
          result = jObj.get<int>();
        }
        else if ( jObj.is_number_unsigned() )
        {
          result = jObj.get<unsigned>();
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

  error.clear();
  try
  {
    const json j = json::parse( jsonString );
    return _parser( j );
  }
  catch ( json::parse_error &ex )
  {
    error = QString::fromStdString( ex.what() );
  }
  return QVariant();
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
