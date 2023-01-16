/***************************************************************************
  qgsattributeeditorspacerelement.cpp - QgsAttributeEditorSpacerElement

 ---------------------
 begin                : 16.1.2023
 copyright            : (C) 2023 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditorspacerelement.h"


QgsAttributeEditorElement *QgsAttributeEditorSpacerElement::clone( QgsAttributeEditorElement *parent ) const
{
  return new QgsAttributeEditorSpacerElement( name(), parent );
}

void QgsAttributeEditorSpacerElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  // Nothing to save
  Q_UNUSED( elem );
  Q_UNUSED( doc );
}

void QgsAttributeEditorSpacerElement::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  // Nothing to load
  Q_UNUSED( element );
  Q_UNUSED( layerId );
  Q_UNUSED( context );
  Q_UNUSED( fields );
}

QString QgsAttributeEditorSpacerElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorSpacerElement" );
}
