/***************************************************************************
    qgsxmlutils.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsxmlutils.h"

#include "qgscolorutils.h"
#include "qgsprocessingparameters.h"
#include "qgsproperty.h"
#include "qgsrectangle.h"
#include "qgsremappingproxyfeaturesink.h"
#include "qgsunittypes.h"

#include <QDomElement>

Qgis::DistanceUnit QgsXmlUtils::readMapUnits( const QDomElement &element )
{
  if ( "unknown" == element.text() )
  {
    return Qgis::DistanceUnit::Unknown;
  }
  else
  {
    const Qgis::DistanceUnit unit = QgsUnitTypes::decodeDistanceUnit( element.text() );
    return unit == Qgis::DistanceUnit::Unknown ? Qgis::DistanceUnit::Degrees : unit;
  }
}

QgsBox3D QgsXmlUtils::readBox3D( const QDomElement &element )
{
  QgsBox3D aoi;

  const double xmin = element.attribute( u"xmin"_s ).toDouble();
  aoi.setXMinimum( xmin );

  const double ymin = element.attribute( u"ymin"_s ).toDouble();
  aoi.setYMinimum( ymin );

  const double zmin = element.attribute( u"zmin"_s ).toDouble();
  aoi.setZMinimum( zmin );

  const double xmax = element.attribute( u"xmax"_s ).toDouble();
  aoi.setXMaximum( xmax );

  const double ymax = element.attribute( u"ymax"_s ).toDouble();
  aoi.setYMaximum( ymax );

  const double zmax = element.attribute( u"zmax"_s ).toDouble();
  aoi.setZMaximum( zmax );

  return aoi;
}

QgsRectangle QgsXmlUtils::readRectangle( const QDomElement &element )
{
  QgsRectangle aoi;

  const QDomNode xminNode = element.namedItem( u"xmin"_s );
  const QDomNode yminNode = element.namedItem( u"ymin"_s );
  const QDomNode xmaxNode = element.namedItem( u"xmax"_s );
  const QDomNode ymaxNode = element.namedItem( u"ymax"_s );

  QDomElement exElement = xminNode.toElement();
  const double xmin = exElement.text().toDouble();
  aoi.setXMinimum( xmin );

  exElement = yminNode.toElement();
  const double ymin = exElement.text().toDouble();
  aoi.setYMinimum( ymin );

  exElement = xmaxNode.toElement();
  const double xmax = exElement.text().toDouble();
  aoi.setXMaximum( xmax );

  exElement = ymaxNode.toElement();
  const double ymax = exElement.text().toDouble();
  aoi.setYMaximum( ymax );

  return aoi;
}



QDomElement QgsXmlUtils::writeMapUnits( Qgis::DistanceUnit units, QDomDocument &doc )
{
  QString unitsString = QgsUnitTypes::encodeUnit( units );
  // maintain compatibility with old projects
  if ( units == Qgis::DistanceUnit::Unknown )
    unitsString = u"unknown"_s;

  QDomElement unitsNode = doc.createElement( u"units"_s );
  unitsNode.appendChild( doc.createTextNode( unitsString ) );
  return unitsNode;
}

QDomElement QgsXmlUtils::writeBox3D( const QgsBox3D &box, QDomDocument &doc, const QString &elementName )
{
  QDomElement elemExtent3D = doc.createElement( elementName );
  elemExtent3D.setAttribute( u"xMin"_s, box.xMinimum() );
  elemExtent3D.setAttribute( u"yMin"_s, box.yMinimum() );
  elemExtent3D.setAttribute( u"zMin"_s, box.zMinimum() );
  elemExtent3D.setAttribute( u"xMax"_s, box.xMaximum() );
  elemExtent3D.setAttribute( u"yMax"_s, box.yMaximum() );
  elemExtent3D.setAttribute( u"zMax"_s, box.zMaximum() );

  return elemExtent3D;
}

QDomElement QgsXmlUtils::writeRectangle( const QgsRectangle &rect, QDomDocument &doc, const QString &elementName )
{
  QDomElement xMin = doc.createElement( u"xmin"_s );
  QDomElement yMin = doc.createElement( u"ymin"_s );
  QDomElement xMax = doc.createElement( u"xmax"_s );
  QDomElement yMax = doc.createElement( u"ymax"_s );

  const QDomText xMinText = doc.createTextNode( qgsDoubleToString( rect.xMinimum() ) );
  const QDomText yMinText = doc.createTextNode( qgsDoubleToString( rect.yMinimum() ) );
  const QDomText xMaxText = doc.createTextNode( qgsDoubleToString( rect.xMaximum() ) );
  const QDomText yMaxText = doc.createTextNode( qgsDoubleToString( rect.yMaximum() ) );

  xMin.appendChild( xMinText );
  yMin.appendChild( yMinText );
  xMax.appendChild( xMaxText );
  yMax.appendChild( yMaxText );

  QDomElement extentNode = doc.createElement( elementName );
  extentNode.appendChild( xMin );
  extentNode.appendChild( yMin );
  extentNode.appendChild( xMax );
  extentNode.appendChild( yMax );
  return extentNode;
}

QDomElement QgsXmlUtils::writeVariant( const QVariant &value, QDomDocument &doc )
{
  QDomElement element = doc.createElement( u"Option"_s );
  switch ( value.userType() )
  {
    case QMetaType::Type::UnknownType:
    {
      element.setAttribute( u"type"_s, u"invalid"_s );
      break;
    }

    case QMetaType::Type::QVariantMap:
    {
      const QVariantMap map = value.toMap();

      for ( auto option = map.constBegin(); option != map.constEnd(); ++option )
      {
        QDomElement optionElement = writeVariant( option.value(), doc );
        optionElement.setAttribute( u"name"_s, option.key() );
        element.appendChild( optionElement );
        element.setAttribute( u"type"_s, u"Map"_s );
      }
      break;
    }

    case QMetaType::Type::QVariantList:
    {
      const QVariantList list = value.toList();

      const auto constList = list;
      for ( const QVariant &value : constList )
      {
        const QDomElement valueElement = writeVariant( value, doc );
        element.appendChild( valueElement );
        element.setAttribute( u"type"_s, u"List"_s );
      }
      break;
    }

    case QMetaType::Type::QStringList:
    {
      const QStringList list = value.toStringList();

      const auto constList = list;
      for ( const QString &value : constList )
      {
        const QDomElement valueElement = writeVariant( value, doc );
        element.appendChild( valueElement );
        element.setAttribute( u"type"_s, u"StringList"_s );
      }
      break;
    }

    case QMetaType::Type::QRect:
    {
      element.setAttribute( u"type"_s, "QRect" );

      const QRect rect = value.toRect();
      element.setAttribute( u"x"_s, rect.x() );
      element.setAttribute( u"y"_s, rect.y() );
      element.setAttribute( u"width"_s, rect.width() );
      element.setAttribute( u"height"_s, rect.height() );
      break;
    }

    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::Bool:
    case QMetaType::Type::Double:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::QString:
      element.setAttribute( u"type"_s, QVariant::typeToName( static_cast<QMetaType::Type>( value.userType() ) ) );
      element.setAttribute( u"value"_s, value.toString() );
      break;

    case QMetaType::Type::QChar:
      element.setAttribute( u"type"_s, QVariant::typeToName( static_cast<QMetaType::Type>( value.userType() ) ) );
      element.setAttribute( u"value"_s, QgsVariantUtils::isNull( value ) ? QString() : QString( value.toChar() ) );
      break;

    case QMetaType::Type::QColor:
      element.setAttribute( u"type"_s, u"color"_s );
      element.setAttribute( u"value"_s, value.value< QColor >().isValid() ? QgsColorUtils::colorToString( value.value< QColor >() ) : QString() );
      break;

    case QMetaType::Type::QDateTime:
      element.setAttribute( u"type"_s, u"datetime"_s );
      element.setAttribute( u"value"_s, value.value< QDateTime >().isValid() ? value.toDateTime().toString( Qt::ISODate ) : QString() );
      break;

    case QMetaType::Type::QDate:
      element.setAttribute( u"type"_s, u"date"_s );
      element.setAttribute( u"value"_s, value.value< QDate >().isValid() ? value.toDate().toString( Qt::ISODate ) : QString() );
      break;

    case QMetaType::Type::QTime:
      element.setAttribute( u"type"_s, u"time"_s );
      element.setAttribute( u"value"_s, value.value< QTime >().isValid() ? value.toTime().toString( Qt::ISODate ) : QString() );
      break;

    default:

      if ( value.userType() == qMetaTypeId<QgsProperty>() )
      {
        element.setAttribute( u"type"_s, u"QgsProperty"_s );
        const QDomElement propertyElem = QgsXmlUtils::writeVariant( value.value< QgsProperty >().toVariant(), doc );
        element.appendChild( propertyElem );
        break;
      }
      else if ( value.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
      {
        element.setAttribute( u"type"_s, u"QgsCoordinateReferenceSystem"_s );
        const QgsCoordinateReferenceSystem crs = value.value< QgsCoordinateReferenceSystem >();
        crs.writeXml( element, doc );
        break;
      }
      else if ( value.userType() == qMetaTypeId< QgsGeometry>() )
      {
        element.setAttribute( u"type"_s, u"QgsGeometry"_s );
        const QgsGeometry geom = value.value< QgsGeometry >();
        element.setAttribute( u"value"_s, geom.asWkt() );
        break;
      }
      else if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
      {
        const QDomElement valueElement = writeVariant( value.value< QgsProcessingOutputLayerDefinition >().toVariant(), doc );
        element.appendChild( valueElement );
        element.setAttribute( u"type"_s, u"QgsProcessingOutputLayerDefinition"_s );
        break;
      }
      else if ( value.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
      {
        const QDomElement valueElement = writeVariant( value.value< QgsProcessingFeatureSourceDefinition >().toVariant(), doc );
        element.appendChild( valueElement );
        element.setAttribute( u"type"_s, u"QgsProcessingFeatureSourceDefinition"_s );
        break;
      }
      else if ( value.userType() == qMetaTypeId<QgsRemappingSinkDefinition>() )
      {
        const QDomElement valueElement = writeVariant( value.value< QgsRemappingSinkDefinition >().toVariant(), doc );
        element.appendChild( valueElement );
        element.setAttribute( u"type"_s, u"QgsRemappingSinkDefinition"_s );
        break;
      }
      else
      {
        Q_ASSERT_X( false, "QgsXmlUtils::writeVariant", u"unsupported %1variant type %2"_s
                    .arg( value.userType() >= QMetaType::Type::User ? "user " : QString() ).arg( value.metaType().name() ).toLocal8Bit() );
      }
      break;
  }

  return element;
}

QVariant QgsXmlUtils::readVariant( const QDomElement &element )
{
  const QString type = element.attribute( u"type"_s );

  if ( type == "invalid"_L1 )
  {
    return QVariant();
  }
  else if ( type == "int"_L1 )
  {
    return element.attribute( u"value"_s ).toInt();
  }
  else if ( type == "uint"_L1 )
  {
    return element.attribute( u"value"_s ).toUInt();
  }
  else if ( type == "qlonglong"_L1 )
  {
    return element.attribute( u"value"_s ).toLongLong();
  }
  else if ( type == "qulonglong"_L1 )
  {
    return element.attribute( u"value"_s ).toULongLong();
  }
  else if ( type == "double"_L1 )
  {
    return element.attribute( u"value"_s ).toDouble();
  }
  else if ( type == "QString"_L1 )
  {
    const QString res = element.attribute( u"value"_s );
    return res.isEmpty() ? QVariant() : res;
  }
  else if ( type == "QChar"_L1 )
  {
    const QString res = element.attribute( u"value"_s );
    return res.isEmpty() ? QVariant() : res.at( 0 );
  }
  else if ( type == "bool"_L1 )
  {
    return element.attribute( u"value"_s ) == "true"_L1;
  }
  else if ( type == "color"_L1 )
  {
    return element.attribute( u"value"_s ).isEmpty() ? QVariant() : QgsColorUtils::colorFromString( element.attribute( u"value"_s ) );
  }
  else if ( type == "datetime"_L1 )
  {
    return element.attribute( u"value"_s ).isEmpty() ? QVariant() : QDateTime::fromString( element.attribute( u"value"_s ), Qt::ISODate );
  }
  else if ( type == "date"_L1 )
  {
    return element.attribute( u"value"_s ).isEmpty() ? QVariant() : QDate::fromString( element.attribute( u"value"_s ), Qt::ISODate );
  }
  else if ( type == "time"_L1 )
  {
    return element.attribute( u"value"_s ).isEmpty() ? QVariant() : QTime::fromString( element.attribute( u"value"_s ), Qt::ISODate );
  }
  else if ( type == "Map"_L1 )
  {
    QVariantMap map;
    const QDomNodeList options = element.childNodes();

    for ( int i = 0; i < options.count(); ++i )
    {
      const QDomElement elem = options.at( i ).toElement();
      if ( elem.tagName() == "Option"_L1 )
        map.insert( elem.attribute( u"name"_s ), readVariant( elem ) );
    }
    return map;
  }
  else if ( type == "List"_L1 )
  {
    QVariantList list;
    const QDomNodeList values = element.childNodes();
    for ( int i = 0; i < values.count(); ++i )
    {
      const QDomElement elem = values.at( i ).toElement();
      list.append( readVariant( elem ) );
    }
    return list;
  }
  else if ( type == "StringList"_L1 )
  {
    QStringList list;
    const QDomNodeList values = element.childNodes();
    for ( int i = 0; i < values.count(); ++i )
    {
      const QDomElement elem = values.at( i ).toElement();
      list.append( readVariant( elem ).toString() );
    }
    return list;
  }
  else if ( type == "QRect"_L1 )
  {
    const int x = element.attribute( "x" ).toInt();
    const int y = element.attribute( "y" ).toInt();
    const int width = element.attribute( "width" ).toInt();
    const int height = element.attribute( "height" ).toInt();

    return QRect( x, y, width, height );
  }
  else if ( type == "QgsProperty"_L1 )
  {
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    QgsProperty p;
    if ( p.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ) ) )
      return p;

    return QVariant();
  }
  else if ( type == "QgsCoordinateReferenceSystem"_L1 )
  {
    QgsCoordinateReferenceSystem crs;
    crs.readXml( element );
    return crs.isValid() ? crs : QVariant();
  }
  else if ( type == "QgsGeometry"_L1 )
  {
    const QgsGeometry g = QgsGeometry::fromWkt( element.attribute( "value" ) );
    return !g.isNull() ? g : QVariant();
  }
  else if ( type == "QgsProcessingOutputLayerDefinition"_L1 )
  {
    QgsProcessingOutputLayerDefinition res;
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    if ( res.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ).toMap() ) )
      return res;

    return QVariant();
  }
  else if ( type == "QgsProcessingFeatureSourceDefinition"_L1 )
  {
    QgsProcessingFeatureSourceDefinition res;
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    if ( res.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ).toMap() ) )
      return res;

    return QVariant();
  }
  else if ( type == "QgsRemappingSinkDefinition"_L1 )
  {
    QgsRemappingSinkDefinition res;
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    if ( res.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ).toMap() ) )
      return QVariant::fromValue( res );

    return QVariant();
  }
  else
  {
    return QVariant();
  }
}
