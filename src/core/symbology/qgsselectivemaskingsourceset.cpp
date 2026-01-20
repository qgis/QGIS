/***************************************************************************
    qgsselectivemaskingsourceset.cpp
    ---------------
    begin                : January 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsselectivemaskingsourceset.h"

#include "qgsreadwritecontext.h"
#include "qgsselectivemaskingsource.h"

#include <QDomDocument>
#include <QDomElement>
#include <QUuid>

QgsSelectiveMaskingSourceSet::QgsSelectiveMaskingSourceSet()
  : mId( QUuid::createUuid().toString() )
{

}

QVector<QgsSelectiveMaskSource> QgsSelectiveMaskingSourceSet::sources() const
{
  return mSources;
}

void QgsSelectiveMaskingSourceSet::setSources( const QVector<QgsSelectiveMaskSource> &sources )
{
  mSources = sources;
  mIsValid = true;
}

void QgsSelectiveMaskingSourceSet::append( const QgsSelectiveMaskSource &source )
{
  mSources.append( source );
}

QDomElement QgsSelectiveMaskingSourceSet::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement setElem = document.createElement( u"selectiveMaskingSourceSet"_s );
  setElem.setAttribute( u"id"_s, mId );
  setElem.setAttribute( u"name"_s, mName );

  for ( const QgsSelectiveMaskSource &source : mSources )
  {
    QDomElement sourceElem = document.createElement( u"source"_s );
    sourceElem.setAttribute( u"layerId"_s, source.layerId() );
    sourceElem.setAttribute( u"sourceType"_s, qgsEnumValueToKey( source.sourceType() ) );
    sourceElem.setAttribute( u"sourceId"_s, source.sourceId() );
    setElem.appendChild( sourceElem );
  }

  return setElem;
}

bool QgsSelectiveMaskingSourceSet::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext & )
{
  if ( element.nodeName() != "selectiveMaskingSourceSet"_L1 )
  {
    return false;
  }

  setId( element.attribute( u"id"_s ) );
  setName( element.attribute( u"name"_s ) );

  mSources.clear();
  const QDomNodeList sourceNodes = element.elementsByTagName( u"source"_s );
  mSources.reserve( sourceNodes.count() );
  for ( int i = 0; i < sourceNodes.count(); ++i )
  {
    const QDomElement sourceElem = sourceNodes.at( i ).toElement();
    QgsSelectiveMaskSource source;
    source.setLayerId( sourceElem.attribute( u"layerId"_s ) );
    source.setSourceType( qgsEnumKeyToValue( sourceElem.attribute( u"sourceType"_s ), Qgis::SelectiveMaskSourceType::SymbolLayer ) );
    source.setSourceId( sourceElem.attribute( u"sourceId"_s ) );
    mSources.append( source );
  }

  return true;
}

QgsSelectiveMaskSource &QgsSelectiveMaskingSourceSet::operator[]( int index )
{
  return mSources[index];
}

int QgsSelectiveMaskingSourceSet::size() const
{
  return mSources.size();
}

bool QgsSelectiveMaskingSourceSet::isEmpty() const
{
  return mSources.empty();
}
