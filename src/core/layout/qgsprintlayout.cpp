/***************************************************************************
                              qgsprintlayout.cpp
                             -------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"

QgsPrintLayout::QgsPrintLayout( QgsProject *project )
  : QgsLayout( project )
  , mAtlas( new QgsLayoutAtlas( this ) )
{
}

QgsLayoutAtlas *QgsPrintLayout::atlas()
{
  return mAtlas;
}

QDomElement QgsPrintLayout::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement layoutElem = QgsLayout::writeXml( document, context );
  mAtlas->writeXml( layoutElem, document, context );
  return layoutElem;
}

bool QgsPrintLayout::readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context )
{
  if ( !QgsLayout::readXml( layoutElement, document, context ) )
    return false;

  QDomElement atlasElem = layoutElement.firstChildElement( QStringLiteral( "Atlas" ) );
  mAtlas->readXml( atlasElem, document, context );
  return true;
}
