/***************************************************************************
                         qgsvectorlayerselectionproperties.cpp
                         ---------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsvectorlayerselectionproperties.h"

QgsVectorLayerSelectionProperties::QgsVectorLayerSelectionProperties( QObject *parent )
  :  QgsMapLayerSelectionProperties( parent )
{
}


QDomElement QgsVectorLayerSelectionProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "selection" ) );

  parentElement.appendChild( element );
  return element;
}

bool QgsVectorLayerSelectionProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement selectionElement = element.firstChildElement( QStringLiteral( "selection" ) ).toElement();
  if ( selectionElement.isNull() )
    return false;

  return true;
}

QgsVectorLayerSelectionProperties *QgsVectorLayerSelectionProperties::clone() const
{
  std::unique_ptr< QgsVectorLayerSelectionProperties > res = std::make_unique< QgsVectorLayerSelectionProperties >( nullptr );
  return res.release();
}
