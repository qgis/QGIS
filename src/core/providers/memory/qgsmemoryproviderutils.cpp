/***************************************************************************
                         qgsmemoryproviderutils.cpp
                         --------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmemoryproviderutils.h"

#include "qgsfields.h"
#include "qgsvectorlayer.h"

#include <QUrl>

QString memoryLayerFieldType( QMetaType::Type type, const QString &typeString )
{
  switch ( type )
  {
    case QMetaType::Type::Int:
      return u"integer"_s;

    case QMetaType::Type::LongLong:
      return u"long"_s;

    case QMetaType::Type::Double:
      return u"double"_s;

    case QMetaType::Type::QString:
      return u"string"_s;

    case QMetaType::Type::QDate:
      return u"date"_s;

    case QMetaType::Type::QTime:
      return u"time"_s;

    case QMetaType::Type::QDateTime:
      return u"datetime"_s;

    case QMetaType::Type::QByteArray:
      return u"binary"_s;

    case QMetaType::Type::Bool:
      return u"boolean"_s;

    case QMetaType::Type::QVariantMap:
      return u"map"_s;

    case QMetaType::Type::User:
      if ( typeString.compare( "geometry"_L1, Qt::CaseInsensitive ) == 0 )
      {
        return u"geometry"_s;
      }
      break;

    default:
      break;
  }
  return u"string"_s;
}

QgsVectorLayer *QgsMemoryProviderUtils::createMemoryLayer( const QString &name, const QgsFields &fields, Qgis::WkbType geometryType, const QgsCoordinateReferenceSystem &crs, bool loadDefaultStyle )
{
  QString geomType = QgsWkbTypes::displayString( geometryType );
  if ( geomType.isNull() )
    geomType = u"none"_s;

  QStringList parts;
  if ( crs.isValid() )
  {
    if ( !crs.authid().isEmpty() )
      parts << u"crs=%1"_s.arg( crs.authid() );
    else
      parts << u"crs=wkt:%1"_s.arg( crs.toWkt( Qgis::CrsWktVariant::Preferred ) );
  }
  else
  {
    parts << u"crs="_s;
  }
  for ( const QgsField &field : fields )
  {
    const QString lengthPrecision = u"(%1,%2)"_s.arg( field.length() ).arg( field.precision() );
    parts << u"field=%1:%2%3%4"_s.arg( QString( QUrl::toPercentEncoding( field.name() ) ),
                                       memoryLayerFieldType( field.type() == QMetaType::Type::QVariantList || field.type() == QMetaType::Type::QStringList ? field.subType() : field.type(), field.typeName() ),
                                       lengthPrecision,
                                       field.type() == QMetaType::Type::QVariantList || field.type() == QMetaType::Type::QStringList ? u"[]"_s : QString() );
  }

  const QString uri = geomType + '?' + parts.join( '&' );
  QgsVectorLayer::LayerOptions options{ QgsCoordinateTransformContext() };
  options.skipCrsValidation = true;
  options.loadDefaultStyle = loadDefaultStyle;
  return new QgsVectorLayer( uri, name, u"memory"_s, options );
}
