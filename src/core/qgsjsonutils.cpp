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
#include "nlohmann/json.hpp"

#include <QJsonDocument>
#include <QJsonArray>

QgsJsonExporter::QgsJsonExporter( QgsVectorLayer *vectorLayer, int precision )
  : mPrecision( precision )
  , mLayer( vectorLayer )
{
  if ( vectorLayer )
  {
    mCrs = vectorLayer->crs();
    mTransform.setSourceCrs( mCrs );
  }
  mTransform.setDestinationCrs( QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId ) );
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
        if ( transformed.transform( mTransform ) == 0 )
          geom = transformed;
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
      }
    }
    QgsRectangle box = geom.boundingBox();

    if ( QgsWkbTypes::flatType( geom.wkbType() ) != QgsWkbTypes::Point )
    {
      featureJson[ "bbox" ] = { {
          box.xMinimum(),
          box.yMinimum(),
          box.xMaximum(),
          box.yMaximum()
        }
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
      QStringList formattersWhiteList;
      formattersWhiteList << QStringLiteral( "KeyValue" )
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
          if ( formattersWhiteList.contains( fieldFormatter->id() ) )
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
      for ( const auto &relation : qgis::as_const( relations ) )
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
  json data
  {
    { "type", "FeatureCollection" },
    { "features", json::array() }
  };
  const auto constFeatures = features;
  for ( const QgsFeature &feature : constFeatures )
  {
    data["features"].push_back( exportFeatureToJsonObject( feature ) );
  }
  return QString::fromStdString( data.dump( indent ) );
}

//
// QgsJsonUtils
//

QgsFeatureList QgsJsonUtils::stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding )
{
  return QgsOgrUtils::stringToFeatureList( string, fields, encoding );
}

QgsFields QgsJsonUtils::stringToFields( const QString &string, QTextCodec *encoding )
{
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
  QJsonParseError error;
  const QJsonDocument jsonDoc = QJsonDocument::fromJson( json.toUtf8(), &error );
  QVariantList result;
  if ( error.error != QJsonParseError::NoError )
  {
    QgsLogger::warning( QStringLiteral( "Cannot parse json (%1): %2" ).arg( error.errorString(), json ) );
    return result;
  }
  if ( !jsonDoc.isArray() )
  {
    QgsLogger::warning( QStringLiteral( "Cannot parse json (%1) as array: %2" ).arg( error.errorString(), json ) );
    return result;
  }
  const auto constArray = jsonDoc.array();
  for ( const QJsonValue &cur : constArray )
  {
    QVariant curVariant = cur.toVariant();
    if ( curVariant.convert( type ) )
      result.append( curVariant );
    else
      QgsLogger::warning( QStringLiteral( "Cannot convert json array element: %1" ).arg( cur.toString() ) );
  }
  return result;
}

json QgsJsonUtils::jsonFromVariant( const QVariant &val )
{
  if ( val.type() == QVariant::Type::Map )
  {
    const auto vMap { val.toMap() };
    auto jMap { json::object() };
    for ( auto it = vMap.constBegin(); it != vMap.constEnd(); it++ )
    {
      jMap[ it.key().toStdString() ] = jsonFromVariant( it.value() );
    }
    return jMap;
  }
  else if ( val.type() == QVariant::Type::List )
  {
    const auto vList{ val.toList() };
    auto jList { json::array() };
    for ( const auto &v : vList )
    {
      jList.push_back( jsonFromVariant( v ) );
    }
    return jList;
  }
  else
  {
    switch ( val.userType() )
    {
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
        return val.toLongLong();
      case QMetaType::Double:
      case QMetaType::Float:
        return val.toDouble();
      default:
        return  val.toString().toStdString();
    }
  }
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
