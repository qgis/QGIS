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


QgsJSONExporter::QgsJSONExporter( int precision, bool includeGeometry, bool includeAttributes )
    : mPrecision( precision )
    , mIncludeGeometry( includeGeometry )
    , mIncludeAttributes( includeAttributes )
{

}

QString QgsJSONExporter::exportFeature( const QgsFeature& feature, const QVariantMap& extraProperties,
                                        const QVariant& id ) const
{
  QString s = "{\n   \"type\":\"Feature\",\n";

  // ID
  s += QString( "   \"id\":%1" ).arg( !id.isValid() ? QString::number( feature.id() ) : QgsJSONUtils::encodeValue( id ) );

  if ( mIncludeAttributes || mIncludeGeometry || !extraProperties.isEmpty() )
    s += ",\n";
  else
    s += '\n';

  const QgsGeometry* geom = feature.constGeometry();
  if ( geom && !geom->isEmpty() && mIncludeGeometry )
  {
    QgsRectangle box = geom->boundingBox();

    if ( QgsWKBTypes::flatType( geom->geometry()->wkbType() ) != QgsWKBTypes::Point )
    {
      s += QString( "   \"bbox\":[%1, %2, %3, %4],\n" ).arg( qgsDoubleToString( box.xMinimum(), mPrecision ),
           qgsDoubleToString( box.yMinimum(), mPrecision ),
           qgsDoubleToString( box.xMaximum(), mPrecision ),
           qgsDoubleToString( box.yMaximum(), mPrecision ) );
    }
    s += "   \"geometry\":\n   ";
    s += geom->exportToGeoJSON( mPrecision );
    if ( mIncludeAttributes || !extraProperties.isEmpty() )
      s += ",\n";
    else
      s += '\n';
  }

  if ( mIncludeAttributes || !extraProperties.isEmpty() )
  {
    //read all attribute values from the feature
    s += "   \"properties\":{\n";
    int attributeCounter = 0;

    if ( mIncludeAttributes )
    {
      const QgsFields* fields = feature.fields();

      for ( int i = 0; i < fields->count(); ++i )
      {
        if ( !mAttributeIndexes.isEmpty() && !mAttributeIndexes.contains( i ) )
          continue;

        if ( attributeCounter > 0 )
          s += ",\n";
        QVariant val =  feature.attributes().at( i );

        s += QString( "      \"%1\":%2" ).arg( fields->at( i ).name(), QgsJSONUtils::encodeValue( val ) );

        ++attributeCounter;
      }
    }

    if ( !extraProperties.isEmpty() )
    {
      QVariantMap::const_iterator it = extraProperties.constBegin();
      for ( ; it != extraProperties.constEnd(); ++it )
      {
        if ( attributeCounter > 0 )
          s += ",\n";

        s += QString( "      \"%1\":%2" ).arg( it.key(), QgsJSONUtils::encodeValue( it.value() ) );

        ++attributeCounter;
      }
    }

    s += "\n   }\n";
  }

  s += "}";

  return s;
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
