/***************************************************************************
  qgsattributeeditorqmlelement.cpp - QgsAttributeEditorQmlelement

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
#include "qgsattributeeditorqmlelement.h"



QgsAttributeEditorElement *QgsAttributeEditorQmlElement::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorQmlElement *element = new QgsAttributeEditorQmlElement( name(), parent );
  element->setQmlCode( mQmlCode );

  return element;
}

QString QgsAttributeEditorQmlElement::qmlCode() const
{
  return mQmlCode;
}

void QgsAttributeEditorQmlElement::setQmlCode( const QString &qmlCode )
{
  mQmlCode = qmlCode;
}

void QgsAttributeEditorQmlElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  const QDomText codeElem = doc.createTextNode( mQmlCode );
  elem.appendChild( codeElem );
}

void QgsAttributeEditorQmlElement::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )
  setQmlCode( element.text() );
}

QString QgsAttributeEditorQmlElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorQmlElement" );
}

