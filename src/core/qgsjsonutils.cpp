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
#include "qgsogrutils.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgsproject.h"

QgsJSONExporter::QgsJSONExporter( const QgsVectorLayer* vectorLayer, int precision )
    : mPrecision( precision )
    , mIncludeGeometry( true )
    , mIncludeAttributes( true )
    , mIncludeRelatedAttributes( false )
    , mLayerId( vectorLayer ? vectorLayer->id() : QString() )
{
  if ( vectorLayer )
  {
    mCrs = vectorLayer->crs();
    mTransform.setSourceCrs( mCrs );
  }
  mTransform.setDestCRS( QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId ) );
}

void QgsJSONExporter::setVectorLayer( const QgsVectorLayer* vectorLayer )
{
  mLayerId = vectorLayer ? vectorLayer->id() : QString();
  if ( vectorLayer )
  {
    mCrs = vectorLayer->crs();
    mTransform.setSourceCrs( mCrs );
  }
}

QgsVectorLayer *QgsJSONExporter::vectorLayer() const
{
  return qobject_cast< QgsVectorLayer* >( QgsMapLayerRegistry::instance()->mapLayer( mLayerId ) );
}

void QgsJSONExporter::setSourceCrs( const QgsCoordinateReferenceSystem& crs )
{
  mCrs = crs;
  mTransform.setSourceCrs( mCrs );
}

const QgsCoordinateReferenceSystem& QgsJSONExporter::sourceCrs() const
{
  return mCrs;
}

QString QgsJSONExporter::exportFeature( const QgsFeature& feature, const QVariantMap& extraProperties,
                                        const QVariant& id ) const
{
  QString s = "{\n   \"type\":\"Feature\",\n";

  // ID
  s += QString( "   \"id\":%1,\n" ).arg( !id.isValid() ? QString::number( feature.id() ) : QgsJSONUtils::encodeValue( id ) );

  const QgsGeometry* geom = feature.constGeometry();
  if ( geom && !geom->isEmpty() && mIncludeGeometry )
  {
    const QgsGeometry* exportGeom = geom;
    if ( mCrs.isValid() )
    {
      QgsGeometry* clone = new QgsGeometry( *geom );
      try
      {
        if ( clone->transform( mTransform ) == 0 )
          exportGeom = clone;
        else
          delete clone;
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        delete clone;
      }
    }
    QgsRectangle box = exportGeom->boundingBox();

    if ( QgsWKBTypes::flatType( exportGeom->geometry()->wkbType() ) != QgsWKBTypes::Point )
    {
      s += QString( "   \"bbox\":[%1, %2, %3, %4],\n" ).arg( qgsDoubleToString( box.xMinimum(), mPrecision ),
           qgsDoubleToString( box.yMinimum(), mPrecision ),
           qgsDoubleToString( box.xMaximum(), mPrecision ),
           qgsDoubleToString( box.yMaximum(), mPrecision ) );
    }
    s += "   \"geometry\":\n   ";
    s += exportGeom->exportToGeoJSON( mPrecision );
    s += ",\n";

    if ( exportGeom != geom )
      delete exportGeom;
  }
  else
  {
    s += "   \"geometry\":null,\n";
  }

  // build up properties element
  QString properties;
  int attributeCounter = 0;
  if ( mIncludeAttributes || !extraProperties.isEmpty() )
  {
    //read all attribute values from the feature

    if ( mIncludeAttributes )
    {
      const QgsFields* fields = feature.fields();

      for ( int i = 0; i < fields->count(); ++i )
      {
        if (( !mAttributeIndexes.isEmpty() && !mAttributeIndexes.contains( i ) ) || mExcludedAttributeIndexes.contains( i ) )
          continue;

        if ( attributeCounter > 0 )
          properties += ",\n";
        QVariant val =  feature.attributes().at( i );

        properties += QString( "      \"%1\":%2" ).arg( fields->at( i ).name(), QgsJSONUtils::encodeValue( val ) );

        ++attributeCounter;
      }
    }

    if ( !extraProperties.isEmpty() )
    {
      QVariantMap::const_iterator it = extraProperties.constBegin();
      for ( ; it != extraProperties.constEnd(); ++it )
      {
        if ( attributeCounter > 0 )
          properties += ",\n";

        properties += QString( "      \"%1\":%2" ).arg( it.key(), QgsJSONUtils::encodeValue( it.value() ) );

        ++attributeCounter;
      }
    }

    // related attributes
    QgsVectorLayer* vl = vectorLayer();
    if ( vl && mIncludeRelatedAttributes )
    {
      QList< QgsRelation > relations = QgsProject::instance()->relationManager()->referencedRelations( vl );
      Q_FOREACH ( const QgsRelation& relation, relations )
      {
        if ( attributeCounter > 0 )
          properties += ",\n";

        QgsFeatureRequest req = relation.getRelatedFeaturesRequest( feature );
        req.setFlags( QgsFeatureRequest::NoGeometry );
        QgsVectorLayer* childLayer = relation.referencingLayer();
        QString relatedFeatureAttributes;
        if ( childLayer )
        {
          QgsFeatureIterator it = childLayer->getFeatures( req );
          QgsFeature relatedFet;
          int relationFeatures = 0;
          while ( it.nextFeature( relatedFet ) )
          {
            if ( relationFeatures > 0 )
              relatedFeatureAttributes += ",\n";

            relatedFeatureAttributes += QgsJSONUtils::exportAttributes( relatedFet );
            relationFeatures++;
          }
        }
        relatedFeatureAttributes.prepend( '[' ).append( ']' );

        properties += QString( "      \"%1\":%2" ).arg( relation.name(), relatedFeatureAttributes );
        attributeCounter++;
      }
    }
  }

  bool hasProperties = attributeCounter > 0;

  s += "   \"properties\":";
  if ( hasProperties )
  {
    //read all attribute values from the feature
    s += "{\n" + properties + "\n   }\n";
  }
  else
  {
    s += "null\n";
  }

  s += '}';

  return s;
}

QString QgsJSONExporter::exportFeatures( const QgsFeatureList& features ) const
{
  QStringList featureJSON;
  Q_FOREACH ( const QgsFeature& feature, features )
  {
    featureJSON << exportFeature( feature );
  }

  return QString( "{ \"type\": \"FeatureCollection\",\n    \"features\":[\n%1\n]}" ).arg( featureJSON.join( ",\n" ) );
}



//
// QgsJSONUtils
//

QgsFeatureList QgsJSONUtils::stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding )
{
  return QgsOgrUtils::stringToFeatureList( string, fields, encoding );
}

QgsFields QgsJSONUtils::stringToFields( const QString &string, QTextCodec *encoding )
{
  return QgsOgrUtils::stringToFields( string, encoding );
}

QString QgsJSONUtils::encodeValue( const QVariant &value )
{
  if ( value.isNull() )
    return "null";

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
    {
      QStringList input = value.toStringList();
      QStringList output;
      Q_FOREACH ( const QString& string, input )
      {
        output << encodeValue( string );
      }
      return output.join( "," ).prepend( '[' ).append( ']' );
    }

    case QVariant::List:
    {
      QVariantList input = value.toList();
      QStringList output;
      Q_FOREACH ( const QVariant& v, input )
      {
        output << encodeValue( v );
      }
      return output.join( "," ).prepend( '[' ).append( ']' );
    }

    case QVariant::Map:
    {
      QMap< QString, QVariant > input = value.toMap();
      QStringList output;
      QMap< QString, QVariant >::const_iterator it = input.constBegin();
      for ( ; it != input.constEnd(); ++it )
      {
        output << encodeValue( it.key() ) + ':' + encodeValue( it.value() );
      }
      return output.join( ",\n" ).prepend( '{' ).append( '}' );
    }

    default:
    case QVariant::String:
      QString v = value.toString()
                  .replace( '\\', "\\\\" )
                  .replace( '"', "\\\"" )
                  .replace( '\r', "\\r" )
                  .replace( '\b', "\\b" )
                  .replace( '\t', "\\t" )
                  .replace( '/', "\\/" )
                  .replace( '\n', "\\n" );

      return v.prepend( '"' ).append( '"' );
  }
}

QString QgsJSONUtils::exportAttributes( const QgsFeature& feature )
{
  const QgsFields* fields = feature.fields();
  QString attrs;
  for ( int i = 0; i < fields->count(); ++i )
  {
    if ( i > 0 )
      attrs += ",\n";

    QVariant val = feature.attributes().at( i );
    attrs += encodeValue( fields->at( i ).name() ) + ':' + encodeValue( val );
  }
  return attrs.prepend( '{' ).append( '}' );
}

