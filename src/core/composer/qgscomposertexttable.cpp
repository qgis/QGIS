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
#include "qgscomposertablecolumn.h"

QgsComposerTextTable::QgsComposerTextTable( QgsComposition* c ): QgsComposerTable( c )
{

}

QgsComposerTextTable::~QgsComposerTextTable()
{

}

void QgsComposerTextTable::setHeaderLabels( const QStringList& labels )
{
  //update existing column headings, or add new columns if required
  QStringList::const_iterator labelIt = labels.constBegin();
  int idx = 0;
  for ( ; labelIt != labels.constEnd(); ++labelIt )
  {
    QgsComposerTableColumn* col;
    if ( idx < mColumns.count() )
    {
      col = mColumns.at( idx );
    }
    else
    {
      col = new QgsComposerTableColumn;
      mColumns.append( col );
    }
    col->setHeading(( *labelIt ) );
    idx++;
  }
}

bool QgsComposerTextTable::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerTableElem = doc.createElement( "ComposerTextTable" );
  //todo: write headers and text entries
  bool ok = _writeXML( composerTableElem, doc );
  elem.appendChild( composerTableElem );
  return ok;
}

bool QgsComposerTextTable::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  //todo: read headers and text entries
  return tableReadXML( itemElem, doc );
}

bool QgsComposerTextTable::getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps )
{
  attributeMaps.clear();

  QList< QStringList >::const_iterator rowIt = mRowText.constBegin();
  QStringList currentStringList;
  for ( ; rowIt != mRowText.constEnd(); ++rowIt )
  {
    currentStringList = *rowIt;

    attributeMaps.push_back( QgsAttributeMap() );
    for ( int i = 0; i < currentStringList.size(); ++i )
    {
      attributeMaps.last().insert( i, QVariant( currentStringList.at( i ) ) );
    }
  }

  return true;
}
