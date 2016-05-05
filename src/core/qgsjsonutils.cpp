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

QgsFeatureList QgsJSONUtils::stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding )
{
  return QgsOgrUtils::stringToFeatureList( string, fields, encoding );
}

QgsFields QgsJSONUtils::stringToFields( const QString &string, QTextCodec *encoding )
{
  return QgsOgrUtils::stringToFields( string, encoding );
}

QString QgsJSONUtils::featureToGeoJSON( const QgsFeature& feature,
                                        int precision,
                                        const QgsAttributeList& attrIndexes,
                                        bool includeGeom,
                                        bool includeAttributes,
                                        const QVariant& id )
{
  QString s = "{\n   \"type\":\"Feature\",\n";

  // ID
  s += QString( "   \"id\":%1" ).arg( !id.isValid() ? QString::number( feature.id() ) : encodeValue( id ) );

  if ( includeAttributes || includeGeom )
    s += ",\n";
  else
    s += '\n';

  const QgsGeometry* geom = feature.constGeometry();
  if ( geom && !geom->isEmpty() && includeGeom )
  {
    QgsRectangle box = geom->boundingBox();

    if ( QgsWKBTypes::flatType( geom->geometry()->wkbType() ) != QgsWKBTypes::Point )
    {
      s += QString( "   \"bbox\":[%1, %2, %3, %4],\n" ).arg( qgsDoubleToString( box.xMinimum(), precision ),
           qgsDoubleToString( box.yMinimum(), precision ),
           qgsDoubleToString( box.xMaximum(), precision ),
           qgsDoubleToString( box.yMaximum(), precision ) );
    }
    s += "   \"geometry\":\n   ";
    s += geom->exportToGeoJSON( precision );
    if ( includeAttributes )
      s += ",\n";
    else
      s += '\n';
  }

  if ( includeAttributes )
  {
    //read all attribute values from the feature
    s += "   \"properties\":{\n";

    const QgsFields* fields = feature.fields();
    int attributeCounter = 0;

    for ( int i = 0; i < fields->count(); ++i )
    {
      if ( !attrIndexes.isEmpty() && !attrIndexes.contains( i ) )
        continue;

      if ( attributeCounter > 0 )
        s += ",\n";
      QVariant val =  feature.attributes().at( i );

      s += QString( "      \"%1\":%2" ).arg( fields->at( i ).name(), encodeValue( val ) );

      ++attributeCounter;
    }

    s += "\n   }\n";
  }

  s += "}";

  return s;
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
