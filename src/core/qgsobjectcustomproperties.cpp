/***************************************************************************
                          qgsobjectcustomproperties.cpp
                             -------------------
    begin                : April 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder.sk at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsobjectcustomproperties.h"
#include "qgis.h"
#include "qgsxmlutils.h"

#include <QDomNode>
#include <QStringList>


QStringList QgsObjectCustomProperties::keys() const
{
  return mMap.keys();
}

void QgsObjectCustomProperties::setValue( const QString &key, const QVariant &value )
{
  mMap[key] = value;
}

QVariant QgsObjectCustomProperties::value( const QString &key, const QVariant &defaultValue ) const
{
  return mMap.value( key, defaultValue );
}

void QgsObjectCustomProperties::remove( const QString &key )
{
  mMap.remove( key );
}

bool QgsObjectCustomProperties::contains( const QString &key ) const
{
  return mMap.contains( key );
}

void QgsObjectCustomProperties::readXml( const QDomNode &parentNode, const QString &keyStartsWith )
{
  const QDomNode propsNode = parentNode.namedItem( QStringLiteral( "customproperties" ) );
  if ( propsNode.isNull() ) // no properties stored...
    return;

  if ( !keyStartsWith.isEmpty() )
  {
    //remove old keys
    QStringList keysToRemove;
    QMap<QString, QVariant>::const_iterator pIt = mMap.constBegin();
    for ( ; pIt != mMap.constEnd(); ++pIt )
    {
      if ( pIt.key().startsWith( keyStartsWith ) )
      {
        keysToRemove.push_back( pIt.key() );
      }
    }

    QStringList::const_iterator sIt = keysToRemove.constBegin();
    for ( ; sIt != keysToRemove.constEnd(); ++sIt )
    {
      mMap.remove( *sIt );
    }
  }
  else
  {
    mMap.clear();
  }

  const QVariant newProps = QgsXmlUtils::readVariant( propsNode.firstChildElement() );
  if ( newProps.type() == QVariant::Map )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QVariantMap propsMap = newProps.toMap();
    for ( auto it = propsMap.constBegin(); it != propsMap.constEnd(); ++it )
      mMap.insert( it.key(), it.value() );
#else
    mMap.insert( newProps.toMap() );
#endif
  }
  else
  {
    // backward compatibility code for QGIS < 3.20
    const QDomNodeList nodes = propsNode.childNodes();

    for ( int i = 0; i < nodes.size(); i++ )
    {
      const QDomNode propNode = nodes.at( i );
      if ( propNode.isNull() || propNode.nodeName() != QLatin1String( "property" ) )
        continue;
      const QDomElement propElement = propNode.toElement();

      const QString key = propElement.attribute( QStringLiteral( "key" ) );
      if ( key.isEmpty() || key.startsWith( keyStartsWith ) )
      {
        if ( propElement.hasAttribute( QStringLiteral( "value" ) ) )
        {
          const QString value = propElement.attribute( QStringLiteral( "value" ) );
          mMap[key] = QVariant( value );
        }
        else
        {
          QStringList list;

          for ( QDomElement itemElement = propElement.firstChildElement( QStringLiteral( "value" ) );
                !itemElement.isNull();
                itemElement = itemElement.nextSiblingElement( QStringLiteral( "value" ) ) )
          {
            list << itemElement.text();
          }

          mMap[key] = QVariant( list );
        }
      }
    }
  }
}

void QgsObjectCustomProperties::writeXml( QDomNode &parentNode, QDomDocument &doc ) const
{
  //remove already existing <customproperties> tags
  const QDomNodeList propertyList = parentNode.toElement().elementsByTagName( QStringLiteral( "customproperties" ) );
  for ( int i = 0; i < propertyList.size(); ++i )
  {
    parentNode.removeChild( propertyList.at( i ) );
  }

  QDomElement propsElement = doc.createElement( QStringLiteral( "customproperties" ) );
  propsElement.appendChild( QgsXmlUtils::writeVariant( mMap, doc ) );
  parentNode.appendChild( propsElement );
}
