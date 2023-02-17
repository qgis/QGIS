/***************************************************************************
  qgsattributeeditorfield.cpp - QgsAttributeEditorField

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
#include "qgsattributeeditorfield.h"

QgsAttributeEditorElement *QgsAttributeEditorField::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorField *element = new QgsAttributeEditorField( name(), mIdx, parent );
  element->mLabelStyle = mLabelStyle;
  return element;
}


void QgsAttributeEditorField::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  elem.setAttribute( QStringLiteral( "index" ), mIdx );
}

void QgsAttributeEditorField::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( element )
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )
}

QString QgsAttributeEditorField::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorField" );
}
