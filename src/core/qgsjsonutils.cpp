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
                                        const QVariant &id ) const
{

  QString s = QStringLiteral( "{\n   \"type\":\"Feature\",\n" );

  // ID
  s += QStringLiteral( "   \"id\":%1,\n" ).arg( !id.isValid() ? QString::number( feature.id() ) : QgsJsonUtils::encodeValue( id ) );

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
      s += QStringLiteral( "   \"bbox\":[%1, %2, %3, %4],\n" ).arg( qgsDoubleToString( box.xMinimum(), mPrecision ),
           qgsDoubleToString( box.yMinimum(), mPrecision ),
           qgsDoubleToString( box.xMaximum(), mPrecision ),
           qgsDoubleToString( box.yMaximum(), mPrecision ) );
    }
    s += QLatin1String( "   \"geometry\":\n   " );
    s += geom.asJson( mPrecision );
    s += QLatin1String( ",\n" );
  }
  else
  {
    s += QLatin1String( "   \"geometry\":null,\n" );
  }

  // build up properties element
  QString properties;
  int attributeCounter = 0;
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

        if ( attributeCounter > 0 )
          properties += QLatin1String( ",\n" );
        QVariant val = feature.attributes().at( i );

        if ( mLayer )
        {
          QgsEditorWidgetSetup setup = fields.at( i ).editorWidgetSetup();
          QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
          if ( formattersWhiteList.contains( fieldFormatter->id() ) )
            val = fieldFormatter->representValue( mLayer.data(), i, setup.config(), QVariant(), val );
        }

        properties += QStringLiteral( "      \"%1\":%2" ).arg( fields.at( i ).name(), QgsJsonUtils::encodeValue( val ) );

        ++attributeCounter;
      }
    }

    if ( !extraProperties.isEmpty() )
    {
      QVariantMap::const_iterator it = extraProperties.constBegin();
      for ( ; it != extraProperties.constEnd(); ++it )
      {
        if ( attributeCounter > 0 )
          properties += QLatin1String( ",\n" );

        properties += QStringLiteral( "      \"%1\":%2" ).arg( it.key(), QgsJsonUtils::encodeValue( it.value() ) );

        ++attributeCounter;
      }
    }

    // related attributes
    if ( mLayer && mIncludeRelatedAttributes )
    {
      QList< QgsRelation > relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer.data() );
      Q_FOREACH ( const QgsRelation &relation, relations )
      {
        if ( attributeCounter > 0 )
          properties += QLatin1String( ",\n" );

        QgsFeatureRequest req = relation.getRelatedFeaturesRequest( feature );
        req.setFlags( QgsFeatureRequest::NoGeometry );
        QgsVectorLayer *childLayer = relation.referencingLayer();
        QString relatedFeatureAttributes;
        if ( childLayer )
        {
          QgsFeatureIterator it = childLayer->getFeatures( req );
          QVector<QVariant> attributeWidgetCaches;
          int fieldIndex = 0;
          const QgsFields fields = childLayer->fields();
          for ( const QgsField &field : fields )
          {
            QgsEditorWidgetSetup setup = field.editorWidgetSetup();
            QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
            attributeWidgetCaches.append( fieldFormatter->createCache( childLayer, fieldIndex, setup.config() ) );
            fieldIndex++;
          }

          QgsFeature relatedFet;
          int relationFeatures = 0;
          while ( it.nextFeature( relatedFet ) )
          {
            if ( relationFeatures > 0 )
              relatedFeatureAttributes += QLatin1String( ",\n" );

            relatedFeatureAttributes += QgsJsonUtils::exportAttributes( relatedFet, childLayer, attributeWidgetCaches );
            relationFeatures++;
          }
        }
        relatedFeatureAttributes.prepend( '[' ).append( ']' );

        properties += QStringLiteral( "      \"%1\":%2" ).arg( relation.name(), relatedFeatureAttributes );
        attributeCounter++;
      }
    }
  }

  bool hasProperties = attributeCounter > 0;

  s += QLatin1String( "   \"properties\":" );
  if ( hasProperties )
  {
    //read all attribute values from the feature
    s += "{\n" + properties + "\n   }\n";
  }
  else
  {
    s += QLatin1String( "null\n" );
  }

  s += '}';

  return s;
}

QString QgsJsonExporter::exportFeatures( const QgsFeatureList &features ) const
{
  QStringList featureJSON;
  Q_FOREACH ( const QgsFeature &feature, features )
  {
    featureJSON << exportFeature( feature );
  }

  return QStringLiteral( "{ \"type\": \"FeatureCollection\",\n    \"features\":[\n%1\n]}" ).arg( featureJSON.join( QStringLiteral( ",\n" ) ) );
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
  Q_FOREACH ( const QJsonValue &cur, jsonDoc.array() )
  {
    QVariant curVariant = cur.toVariant();
    if ( curVariant.convert( type ) )
      result.append( curVariant );
    else
      QgsLogger::warning( QStringLiteral( "Cannot convert json array element: %1" ).arg( cur.toString() ) );
  }
  return result;
}
