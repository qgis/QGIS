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

#include <QDomNode>
#include <QStringList>


QgsObjectCustomProperties::QgsObjectCustomProperties()
{
}

QStringList QgsObjectCustomProperties::keys() const
{
  return mMap.keys();
}

void QgsObjectCustomProperties::setValue( const QString& key, const QVariant& value )
{
  mMap[key] = value;
}

QVariant QgsObjectCustomProperties::value( const QString& key, const QVariant& defaultValue ) const
{
  return mMap.value( key, defaultValue );
}

void QgsObjectCustomProperties::remove( const QString& key )
{
  mMap.remove( key );
}


void QgsObjectCustomProperties::readXml( const QDomNode& parentNode, const QString& keyStartsWith )
{
  QDomNode propsNode = parentNode.namedItem( "customproperties" );
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

  QDomNodeList nodes = propsNode.childNodes();

  for ( int i = 0; i < nodes.size(); i++ )
  {
    QDomNode propNode = nodes.at( i );
    if ( propNode.isNull() || propNode.nodeName() != "property" )
      continue;
    QDomElement propElement = propNode.toElement();

    QString key = propElement.attribute( "key" );
    if ( key.isEmpty() || key.startsWith( keyStartsWith ) )
    {
      if ( propElement.hasAttribute( "value" ) )
      {
        QString value = propElement.attribute( "value" );
        mMap[key] = QVariant( value );
      }
      else
      {
        QStringList list;

        for ( QDomElement itemElement = propElement.firstChildElement( "value" );
              !itemElement.isNull();
              itemElement = itemElement.nextSiblingElement( "value" ) )
        {
          list << itemElement.text();
        }

        mMap[key] = QVariant( list );
      }
    }
  }

}

void QgsObjectCustomProperties::writeXml( QDomNode& parentNode, QDomDocument& doc ) const
{
  //remove already existing <customproperties> tags
  QDomNodeList propertyList = parentNode.toElement().elementsByTagName( "customproperties" );
  for ( int i = 0; i < propertyList.size(); ++i )
  {
    parentNode.removeChild( propertyList.at( i ) );
  }

  QDomElement propsElement = doc.createElement( "customproperties" );

  for ( QMap<QString, QVariant>::const_iterator it = mMap.constBegin(); it != mMap.constEnd(); ++it )
  {
    QDomElement propElement = doc.createElement( "property" );
    propElement.setAttribute( "key", it.key() );
    if ( it.value().canConvert<QString>() )
    {
      propElement.setAttribute( "value", it.value().toString() );
    }
    else if ( it.value().canConvert<QStringList>() )
    {
      foreach ( QString value, it.value().toStringList() )
      {
        QDomElement itemElement = doc.createElement( "value" );
        itemElement.appendChild( doc.createTextNode( value ) );
        propElement.appendChild( itemElement );
      }
    }
    propsElement.appendChild( propElement );
  }

  parentNode.appendChild( propsElement );
}
