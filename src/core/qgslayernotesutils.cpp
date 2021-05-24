/***************************************************************************
  qgslayernotesutils.cpp
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayernotesutils.h"
#include "qgsmaplayer.h"

QString QgsLayerNotesUtils::layerNotes( const QgsMapLayer *layer )
{
  if ( !layer )
    return nullptr;

  return layer->customProperty( QStringLiteral( "userNotes" ) ).toString();
}

void QgsLayerNotesUtils::setLayerNotes( QgsMapLayer *layer, const QString &notes )
{
  if ( !layer )
    return;

  if ( notes.isEmpty() )
    layer->removeCustomProperty( QStringLiteral( "userNotes" ) );
  else
    layer->setCustomProperty( QStringLiteral( "userNotes" ), notes );
}

bool QgsLayerNotesUtils::layerHasNotes( const QgsMapLayer *layer )
{
  if ( !layer )
    return false;

  return !layer->customProperty( QStringLiteral( "userNotes" ) ).toString().isEmpty();
}

void QgsLayerNotesUtils::removeNotes( QgsMapLayer *layer )
{
  if ( layer )
    layer->removeCustomProperty( QStringLiteral( "userNotes" ) );
}
