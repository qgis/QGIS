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

  return layer->customProperty( u"userNotes"_s ).toString();
}

void QgsLayerNotesUtils::setLayerNotes( QgsMapLayer *layer, const QString &notes )
{
  if ( !layer )
    return;

  if ( notes.isEmpty() )
    layer->removeCustomProperty( u"userNotes"_s );
  else
    layer->setCustomProperty( u"userNotes"_s, notes );
}

bool QgsLayerNotesUtils::layerHasNotes( const QgsMapLayer *layer )
{
  if ( !layer )
    return false;

  return !layer->customProperty( u"userNotes"_s ).toString().isEmpty();
}

void QgsLayerNotesUtils::removeNotes( QgsMapLayer *layer )
{
  if ( layer )
    layer->removeCustomProperty( u"userNotes"_s );
}
