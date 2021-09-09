/***************************************************************************
  qgsattributeeditorhtmlelement.cpp - QgsAttributeEditorHtmlelement

 ---------------------
 begin                : 12.01.2021
 copyright            : (C) 2021 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditorhtmlelement.h"


QgsAttributeEditorElement *QgsAttributeEditorHtmlElement::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorHtmlElement *element = new QgsAttributeEditorHtmlElement( name(), parent );
  element->setHtmlCode( mHtmlCode );

  return element;
}

QString QgsAttributeEditorHtmlElement::htmlCode() const
{
  return mHtmlCode;
}

void QgsAttributeEditorHtmlElement::setHtmlCode( const QString &htmlCode )
{
  mHtmlCode = htmlCode;
}

void QgsAttributeEditorHtmlElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  const QDomText codeElem = doc.createTextNode( mHtmlCode );
  elem.appendChild( codeElem );
}

void QgsAttributeEditorHtmlElement::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )
  setHtmlCode( element.text() );
}

QString QgsAttributeEditorHtmlElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorHtmlElement" );
}

