/***************************************************************************
  qgsattributeeditortextelement.cpp - QgsAttributeEditorTextelement

 ---------------------
 begin                : 28.12.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditortextelement.h"
#include <QDomText>

QgsAttributeEditorElement *QgsAttributeEditorTextElement::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorTextElement *element = new QgsAttributeEditorTextElement( name(), parent );
  element->setText( mText );

  return element;
}

QString QgsAttributeEditorTextElement::text() const
{
  return mText;
}

void QgsAttributeEditorTextElement::setText( const QString &text )
{
  mText = text;
}

void QgsAttributeEditorTextElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  const QDomText textElem = doc.createTextNode( mText );
  elem.appendChild( textElem );
}

void QgsAttributeEditorTextElement::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )
  setText( element.text() );
}

QString QgsAttributeEditorTextElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorTextElement" );
}

