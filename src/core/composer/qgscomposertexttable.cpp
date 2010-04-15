/***************************************************************************
                         qgscomposertexttable.h
                         ----------------------
    begin                : April 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposertexttable.h"

QgsComposerTextTable::QgsComposerTextTable( QgsComposition* c ): QgsComposerTable( c )
{

}

QgsComposerTextTable::~QgsComposerTextTable()
{

}

bool QgsComposerTextTable::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerTableElem = doc.createElement( "ComposerTextTable" );
  //todo: write headers and text entries
  bool ok = _writeXML( composerTableElem , doc );
  elem.appendChild( composerTableElem );
  return ok;
}

bool QgsComposerTextTable::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  //todo: read headers and text entries
  return tableReadXML( itemElem, doc );
}

bool QgsComposerTextTable::getFeatureAttributes( QList<QgsAttributeMap>& attributes )
{
  attributes.clear();

  QList< QStringList >::const_iterator rowIt = mRowText.constBegin();
  QStringList currentStringList;
  for ( ; rowIt != mRowText.constEnd(); ++rowIt )
  {
    currentStringList = *rowIt;
    QgsAttributeMap map;
    for ( int i = 0; i < currentStringList.size(); ++i )
    {
      map.insert( i, QVariant( currentStringList.at( i ) ) );
    }
    attributes.append( map );
  }
  return true;
}

QMap<int, QString> QgsComposerTextTable::getHeaderLabels() const
{
  QMap<int, QString> header;
  QStringList::const_iterator it = mHeaderLabels.constBegin();
  int index = 0;
  for ( ; it != mHeaderLabels.constEnd(); ++it )
  {
    header.insert( index, *it );
    ++index;
  }
  return header;
}


