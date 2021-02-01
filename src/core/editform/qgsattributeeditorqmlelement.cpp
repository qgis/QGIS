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
  element->setResize( mResize );

  return element;
}

bool QgsAttributeEditorQmlElement::resize() const
{
  return mResize;
}

void QgsAttributeEditorQmlElement::setResize( const bool resize )
{
  mResize = resize;
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
  QDomText codeElem = doc.createTextNode( mQmlCode );  
  elem.setAttribute( QStringLiteral( "resize" ), mResize ? 1 : 0 );
  elem.appendChild( codeElem );
}

void QgsAttributeEditorQmlElement::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )
  bool ok;
  setQmlCode( element.text() );
  bool resizable = element.attribute( QStringLiteral( "resize" ) ).toInt( &ok );
  if ( ok )
    setResize( resizable );
  else
    setResize( false );
}

QString QgsAttributeEditorQmlElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorQmlElement" );
}

