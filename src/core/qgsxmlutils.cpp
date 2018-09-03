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

QgsUnitTypes::DistanceUnit QgsXmlUtils::readMapUnits( const QDomElement &element )
{
  if ( "unknown" == element.text() )
  {
    return QgsUnitTypes::DistanceUnknownUnit;
  }
  else
  {
    QgsUnitTypes::DistanceUnit unit = QgsUnitTypes::decodeDistanceUnit( element.text() );
    return unit == QgsUnitTypes::DistanceUnknownUnit ? QgsUnitTypes::DistanceDegrees : unit;
  }
}

QgsRectangle QgsXmlUtils::readRectangle( const QDomElement &element )
{
  QgsRectangle aoi;

  QDomNode xminNode = element.namedItem( QStringLiteral( "xmin" ) );
  QDomNode yminNode = element.namedItem( QStringLiteral( "ymin" ) );
  QDomNode xmaxNode = element.namedItem( QStringLiteral( "xmax" ) );
  QDomNode ymaxNode = element.namedItem( QStringLiteral( "ymax" ) );

  QDomElement exElement = xminNode.toElement();
  double xmin = exElement.text().toDouble();
  aoi.setXMinimum( xmin );

  exElement = yminNode.toElement();
  double ymin = exElement.text().toDouble();
  aoi.setYMinimum( ymin );

  exElement = xmaxNode.toElement();
  double xmax = exElement.text().toDouble();
  aoi.setXMaximum( xmax );

  exElement = ymaxNode.toElement();
  double ymax = exElement.text().toDouble();
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

QDomElement QgsXmlUtils::writeRectangle( const QgsRectangle &rect, QDomDocument &doc )
{
  QDomElement xMin = doc.createElement( QStringLiteral( "xmin" ) );
  QDomElement yMin = doc.createElement( QStringLiteral( "ymin" ) );
  QDomElement xMax = doc.createElement( QStringLiteral( "xmax" ) );
  QDomElement yMax = doc.createElement( QStringLiteral( "ymax" ) );

  QDomText xMinText = doc.createTextNode( qgsDoubleToString( rect.xMinimum() ) );
  QDomText yMinText = doc.createTextNode( qgsDoubleToString( rect.yMinimum() ) );
  QDomText xMaxText = doc.createTextNode( qgsDoubleToString( rect.xMaximum() ) );
  QDomText yMaxText = doc.createTextNode( qgsDoubleToString( rect.yMaximum() ) );

  xMin.appendChild( xMinText );
  yMin.appendChild( yMinText );
  xMax.appendChild( xMaxText );
  yMax.appendChild( yMaxText );

  QDomElement extentNode = doc.createElement( QStringLiteral( "extent" ) );
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
    case QVariant::Map:
    {
      QVariantMap map = value.toMap();

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
      QVariantList list = value.toList();

      Q_FOREACH ( const QVariant &value, list )
      {
        QDomElement valueElement = writeVariant( value, doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "List" ) );
      }
      break;
    }

    case QVariant::StringList:
    {
      QStringList list = value.toStringList();

      Q_FOREACH ( const QString &value, list )
      {
        QDomElement valueElement = writeVariant( value, doc );
        element.appendChild( valueElement );
        element.setAttribute( QStringLiteral( "type" ), QStringLiteral( "StringList" ) );
      }
      break;
    }

    case QVariant::Int:
    case QVariant::Bool:
    case QVariant::Double:
    case QVariant::String:
      element.setAttribute( QStringLiteral( "type" ), QVariant::typeToName( value.type() ) );
      element.setAttribute( QStringLiteral( "value" ), value.toString() );
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
      FALLTHROUGH
    }

    default:
      Q_ASSERT_X( false, "QgsXmlUtils::writeVariant", "unsupported variant type" );
      break;
  }

  return element;
}

QVariant QgsXmlUtils::readVariant( const QDomElement &element )
{
  QString type = element.attribute( QStringLiteral( "type" ) );

  if ( type == QLatin1String( "int" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toInt();
  }
  else if ( type == QLatin1String( "double" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toDouble();
  }
  else if ( type == QLatin1String( "QString" ) )
  {
    return element.attribute( QStringLiteral( "value" ) );
  }
  else if ( type == QLatin1String( "bool" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ) == QLatin1String( "true" );
  }
  else if ( type == QLatin1String( "Map" ) )
  {
    QVariantMap map;
    QDomNodeList options = element.childNodes();

    for ( int i = 0; i < options.count(); ++i )
    {
      QDomElement elem = options.at( i ).toElement();
      if ( elem.tagName() == QLatin1String( "Option" ) )
        map.insert( elem.attribute( QStringLiteral( "name" ) ), readVariant( elem ) );
    }
    return map;
  }
  else if ( type == QLatin1String( "List" ) )
  {
    QVariantList list;
    QDomNodeList values = element.childNodes();
    for ( int i = 0; i < values.count(); ++i )
    {
      QDomElement elem = values.at( i ).toElement();
      list.append( readVariant( elem ) );
    }
    return list;
  }
  else if ( type == QLatin1String( "StringList" ) )
  {
    QStringList list;
    QDomNodeList values = element.childNodes();
    for ( int i = 0; i < values.count(); ++i )
    {
      QDomElement elem = values.at( i ).toElement();
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
  else
  {
    return QVariant();
  }
}
