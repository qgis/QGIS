/***************************************************************************
  qgsconfigurationmap.cpp - QgsConfigurationMap

 ---------------------
 begin                : 11.11.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsconfigurationmap.h"
#include <QMetaEnum>
#include "qgis.h"

QgsConfigurationMap::QgsConfigurationMap()
{

}

QgsConfigurationMap::QgsConfigurationMap( const QVariantMap& value )
    : mValue( value )
{
}

QVariantMap QgsConfigurationMap::get() const
{
  return mValue;
}

void QgsConfigurationMap::set( const QVariantMap& value )
{
  mValue = value;
}

void QgsConfigurationMap::toXml( QDomElement& parentElement ) const
{
  return toXml( parentElement, mValue );
}

void QgsConfigurationMap::fromXml( const QDomElement& element )
{
  mValue = fromXmlHelper( element ).toMap();
}

void QgsConfigurationMap::toXml( QDomElement& parentElement, const QVariant& value ) const
{
  switch ( value.type() )
  {
    case QVariant::Map:
    {
      QVariantMap map = value.toMap();

      for ( auto option = map.constBegin(); option != map.constEnd(); ++option )
      {
        QDomElement optionElement = parentElement.ownerDocument().createElement( "Option" );
        optionElement.setAttribute( "name", option.key() );
        toXml( optionElement, option.value() );
        parentElement.appendChild( optionElement );
        parentElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "Map" ) );
      }
      break;
    }

    case QVariant::Int:
      parentElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "Int" ) );
      parentElement.setAttribute( QStringLiteral( "value" ), value.toString() );
      break;

    case QVariant::Bool:
      parentElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "Bool" ) );
      parentElement.setAttribute( QStringLiteral( "value" ), value.toString() );
      break;

    case QVariant::Double:
      parentElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "Double" ) );
      parentElement.setAttribute( QStringLiteral( "value" ), value.toString() );
      break;

    case QVariant::String:
      parentElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "String" ) );
      parentElement.setAttribute( QStringLiteral( "value" ), value.toString() );
      break;

    default:
      parentElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "Unknown" ) );
      parentElement.setAttribute( QStringLiteral( "value" ), value.toString() );
      break;
  }
}

QVariant QgsConfigurationMap::fromXmlHelper( const QDomElement& element ) const
{
  QString type = element.attribute( QStringLiteral( "type" ) );

  if ( type == QLatin1String( "Int" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toInt();
  }
  else if ( type == QLatin1String( "Double" ) )
  {
    return element.attribute( QStringLiteral( "value" ) ).toDouble();
  }
  else if ( type == QLatin1String( "String" ) )
  {
    return element.attribute( QStringLiteral( "value" ) );
  }
  else if ( type == QLatin1String( "Bool" ) )
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
        map.insert( elem.attribute( QStringLiteral( "name" ) ), fromXmlHelper( elem ) );
    }
    return map;
  }
  else
  {
    return QVariant();
  }
}
