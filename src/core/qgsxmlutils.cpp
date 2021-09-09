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

#include <QDomElement>

#include "qgslogger.h"
#include "qgsrectangle.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgsprocessingparameters.h"
#include "qgsremappingproxyfeaturesink.h"

QgsUnitTypes::DistanceUnit QgsXmlUtils::readMapUnits( const QDomElement &element )
{
  if ( "unknown" == element.text() )
  {
    return QgsUnitTypes::DistanceUnknownUnit;
  }
  else
  {
    const QgsUnitTypes::DistanceUnit unit = QgsUnitTypes::decodeDistanceUnit( element.text() );
    return unit == QgsUnitTypes::DistanceUnknownUnit ? QgsUnitTypes::DistanceDegrees : unit;
  }
}

QgsRectangle QgsXmlUtils::readRectangle( const QDomElement &element )
{
  QgsRectangle aoi;

  const QDomNode xminNode = element.namedItem( QStringLiteral( "xmin" ) );
  const QDomNode yminNode = element.namedItem( QStringLiteral( "ymin" ) );
  const QDomNode xmaxNode = element.namedItem( QStringLiteral( "xmax" ) );
  const QDomNode ymaxNode = element.namedItem( QStringLiteral( "ymax" ) );

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



QDomElement QgsXmlUtils::writeMapUnits( QgsUnitTypes::DistanceUnit units, QDomDocument &doc )
{
  QString unitsString = QgsUnitTypes::encodeUnit( units );
  // maintain compatibility with old projects
  if ( units == QgsUnitTypes::DistanceUnknownUnit )
    unitsString = QStringLiteral( "unknown" );

  QDomElement unitsNode = doc.createElement( QStringLiteral( "units" ) );
  unitsNode.appendChild( doc.createTextNode( unitsString ) );
  return unitsNode;
}

QDomElement QgsXmlUtils::writeRectangle( const QgsRectangle &rect, QDomDocument &doc, const QString &elementName )
{
  QDomElement xMin = doc.createElement( QStringLiteral( "xmin" ) );
  QDomElement yMin = doc.createElement( QStringLiteral( "ymin" ) );
  QDomElement xMax = doc.createElement( QStringLiteral( "xmax" ) );
  QDomElement yMax = doc.createElement( QStringLiteral( "ymax" ) );

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
  QDomElement element = doc.createElement( QStringLiteral( "Option" ) );
  switch ( value.type() )
  {
    case QVariant::Invalid:
    {
      element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "invalid" ) );
      break;
    }

    case QVariant::Map:
    {
      const QVariantMap map = value.toMap();

      for ( auto option = map.constBegin(); option != map.constEnd(); ++option )
      {
        QDomElement optionElement = writeVariant( option.value(), doc );
        optionElement.setAttribute( QStringLiteral( "name" ), option.key() );
        element.appendChild( optionElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "Map" ) );
      }
      break;
    }

    case QVariant::List:
    {
      const QVariantList list = value.toList();

      const auto constList = list;
      for ( const QVariant &value : constList )
      {
        const QDomElement valueElement = writeVariant( value, doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "List" ) );
      }
      break;
    }

    case QVariant::StringList:
    {
      const QStringList list = value.toStringList();

      const auto constList = list;
      for ( const QString &value : constList )
      {
        const QDomElement valueElement = writeVariant( value, doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "StringList" ) );
      }
      break;
    }

    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Bool:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::String:
      element.setAttribute( QStringLiteral( "type" ), QVariant::typeToName( value.type() ) );
      element.setAttribute( QStringLiteral( "value" ), value.toString() );
      break;

    case QVariant::Char:
      element.setAttribute( QStringLiteral( "type" ), QVariant::typeToName( value.type() ) );
      element.setAttribute( QStringLiteral( "value" ), value.isNull() ? QString() : value.toString() );
      break;

    case QVariant::Color:
      element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "color" ) );
      element.setAttribute( QStringLiteral( "value" ), value.value< QColor >().isValid() ? QgsSymbolLayerUtils::encodeColor( value.value< QColor >() ) : QString() );
      break;

    case QVariant::DateTime:
      element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "datetime" ) );
      element.setAttribute( QStringLiteral( "value" ), value.value< QDateTime >().isValid() ? value.toDateTime().toString( Qt::ISODate ) : QString() );
      break;

    case QVariant::Date:
      element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "date" ) );
      element.setAttribute( QStringLiteral( "value" ), value.value< QDate >().isValid() ? value.toDate().toString( Qt::ISODate ) : QString() );
      break;

    case QVariant::Time:
      element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "time" ) );
      element.setAttribute( QStringLiteral( "value" ), value.value< QTime >().isValid() ? value.toTime().toString( Qt::ISODate ) : QString() );
      break;

    case QVariant::UserType:
    {
      if ( value.canConvert< QgsProperty >() )
      {
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QgsProperty" ) );
        const QDomElement propertyElem = QgsXmlUtils::writeVariant( value.value< QgsProperty >().toVariant(), doc );
        element.appendChild( propertyElem );
        break;
      }
      else if ( value.canConvert< QgsCoordinateReferenceSystem >() )
      {
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QgsCoordinateReferenceSystem" ) );
        const QgsCoordinateReferenceSystem crs = value.value< QgsCoordinateReferenceSystem >();
        crs.writeXml( element, doc );
        break;
      }
      else if ( value.canConvert< QgsGeometry >() )
      {
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QgsGeometry" ) );
        const QgsGeometry geom = value.value< QgsGeometry >();
        element.setAttribute( QStringLiteral( "value" ), geom.asWkt() );
        break;
      }
      else if ( value.canConvert< QgsProcessingOutputLayerDefinition >() )
      {
        const QDomElement valueElement = writeVariant( value.value< QgsProcessingOutputLayerDefinition >().toVariant(), doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QgsProcessingOutputLayerDefinition" ) );
        break;
      }
      else if ( value.canConvert< QgsProcessingFeatureSourceDefinition >() )
      {
        const QDomElement valueElement = writeVariant( value.value< QgsProcessingFeatureSourceDefinition >().toVariant(), doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QgsProcessingFeatureSourceDefinition" ) );
        break;
      }
      else if ( value.canConvert< QgsRemappingSinkDefinition >() )
      {
        const QDomElement valueElement = writeVariant( value.value< QgsRemappingSinkDefinition >().toVariant(), doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "QgsRemappingSinkDefinition" ) );
        break;
      }
      Q_ASSERT_X( false, "QgsXmlUtils::writeVariant", QStringLiteral( "unsupported user variant type %1" ).arg( QMetaType::typeName( value.userType() ) ).toLocal8Bit() );
      break;
    }

    default:
      Q_ASSERT_X( false, "QgsXmlUtils::writeVariant", QStringLiteral( "unsupported variant type %1" ).arg( QVariant::typeToName( value.type() ) ).toLocal8Bit() );
      break;
  }

  return element;
}

QVariant QgsXmlUtils::readVariant( const QDomElement &element )
{
  const QString type = element.attribute( QStringLiteral( "type" ) );

  if ( type == QLatin1String( "invalid" ) )
  {
    return QVariant();
  }
  else if ( type == QLatin1String( "int" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toInt();
  }
  else if ( type == QLatin1String( "uint" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toUInt();
  }
  else if ( type == QLatin1String( "qlonglong" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toLongLong();
  }
  else if ( type == QLatin1String( "qulonglong" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toULongLong();
  }
  else if ( type == QLatin1String( "double" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toDouble();
  }
  else if ( type == QLatin1String( "QString" ) )
  {
    return element.attribute( QStringLiteral( "value" ) );
  }
  else if ( type == QLatin1String( "QChar" ) )
  {
    const QString res = element.attribute( QStringLiteral( "value" ) );
    return res.isEmpty() ? QChar() : res.at( 0 );
  }
  else if ( type == QLatin1String( "bool" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ) == QLatin1String( "true" );
  }
  else if ( type == QLatin1String( "color" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).isEmpty() ? QColor() : QgsSymbolLayerUtils::decodeColor( element.attribute( QStringLiteral( "value" ) ) );
  }
  else if ( type == QLatin1String( "datetime" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).isEmpty() ? QDateTime() : QDateTime::fromString( element.attribute( QStringLiteral( "value" ) ), Qt::ISODate );
  }
  else if ( type == QLatin1String( "date" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).isEmpty() ? QDate() : QDate::fromString( element.attribute( QStringLiteral( "value" ) ), Qt::ISODate );
  }
  else if ( type == QLatin1String( "time" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).isEmpty() ? QTime() : QTime::fromString( element.attribute( QStringLiteral( "value" ) ), Qt::ISODate );
  }
  else if ( type == QLatin1String( "Map" ) )
  {
    QVariantMap map;
    const QDomNodeList options = element.childNodes();

    for ( int i = 0; i < options.count(); ++i )
    {
      const QDomElement elem = options.at( i ).toElement();
      if ( elem.tagName() == QLatin1String( "Option" ) )
        map.insert( elem.attribute( QStringLiteral( "name" ) ), readVariant( elem ) );
    }
    return map;
  }
  else if ( type == QLatin1String( "List" ) )
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
  else if ( type == QLatin1String( "StringList" ) )
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
  else if ( type == QLatin1String( "QgsProperty" ) )
  {
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    QgsProperty p;
    if ( p.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ) ) )
      return p;

    return QVariant();
  }
  else if ( type == QLatin1String( "QgsCoordinateReferenceSystem" ) )
  {
    QgsCoordinateReferenceSystem crs;
    crs.readXml( element );
    return crs;
  }
  else if ( type == QLatin1String( "QgsGeometry" ) )
  {
    return QgsGeometry::fromWkt( element.attribute( "value" ) );
  }
  else if ( type == QLatin1String( "QgsProcessingOutputLayerDefinition" ) )
  {
    QgsProcessingOutputLayerDefinition res;
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    if ( res.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ).toMap() ) )
      return res;

    return QVariant();
  }
  else if ( type == QLatin1String( "QgsProcessingFeatureSourceDefinition" ) )
  {
    QgsProcessingFeatureSourceDefinition res;
    const QDomNodeList values = element.childNodes();
    if ( values.isEmpty() )
      return QVariant();

    if ( res.loadVariant( QgsXmlUtils::readVariant( values.at( 0 ).toElement() ).toMap() ) )
      return res;

    return QVariant();
  }
  else if ( type == QLatin1String( "QgsRemappingSinkDefinition" ) )
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
