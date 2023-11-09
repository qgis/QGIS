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
#include <QDomElement>

QgsAttributeEditorElement *QgsAttributeEditorSpacerElement::clone( QgsAttributeEditorElement *parent ) const
{
  return new QgsAttributeEditorSpacerElement( name(), parent );
}

void QgsAttributeEditorSpacerElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc );
  elem.setAttribute( QStringLiteral( "drawLine" ), mDrawLine ? 1 : 0 );
}

void QgsAttributeEditorSpacerElement::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  // Nothing to load
  Q_UNUSED( layerId );
  Q_UNUSED( context );
  Q_UNUSED( fields );

  bool ok;
  const bool drawLine = element.attribute( QStringLiteral( "drawLine" ) ).toInt( &ok );
  if ( ok )
    mDrawLine = drawLine;
}

QString QgsAttributeEditorSpacerElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorSpacerElement" );
}

bool QgsAttributeEditorSpacerElement::drawLine() const
{
  return mDrawLine;
}

void QgsAttributeEditorSpacerElement::setDrawLine( bool drawLine )
{
  mDrawLine = drawLine;
}
