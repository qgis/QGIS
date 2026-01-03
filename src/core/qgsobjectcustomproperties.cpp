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
  const QDomNode propsNode = parentNode.namedItem( u"customproperties"_s );
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
  if ( newProps.userType() == QMetaType::Type::QVariantMap )
  {
    mMap.insert( newProps.toMap() );
  }
  else
  {
    // backward compatibility code for QGIS < 3.20
    const QDomNodeList nodes = propsNode.childNodes();

    for ( int i = 0; i < nodes.size(); i++ )
    {
      const QDomNode propNode = nodes.at( i );
      if ( propNode.isNull() || propNode.nodeName() != "property"_L1 )
        continue;
      const QDomElement propElement = propNode.toElement();

      const QString key = propElement.attribute( u"key"_s );
      if ( key.isEmpty() || key.startsWith( keyStartsWith ) )
      {
        if ( propElement.hasAttribute( u"value"_s ) )
        {
          const QString value = propElement.attribute( u"value"_s );
          mMap[key] = QVariant( value );
        }
        else
        {
          QStringList list;

          for ( QDomElement itemElement = propElement.firstChildElement( u"value"_s );
                !itemElement.isNull();
                itemElement = itemElement.nextSiblingElement( u"value"_s ) )
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
  const QDomNodeList propertyList = parentNode.toElement().elementsByTagName( u"customproperties"_s );
  for ( int i = 0; i < propertyList.size(); ++i )
  {
    parentNode.removeChild( propertyList.at( i ) );
  }

  QDomElement propsElement = doc.createElement( u"customproperties"_s );
  propsElement.appendChild( QgsXmlUtils::writeVariant( mMap, doc ) );
  parentNode.appendChild( propsElement );
}
