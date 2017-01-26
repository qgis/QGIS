/***************************************************************************
  qgsmaplayerref.h
  --------------------------------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERREF_H
#define QGSMAPLAYERREF_H

#include <QPointer>

#include "qgsmaplayer.h"

/** Internal structure to keep weak pointer to QgsMapLayer or layerId
 *  if the layer is not available yet.
 *  @note not available in python bindings
 */
struct QgsMapLayerRef
{
  QgsMapLayerRef( QgsMapLayer* l = nullptr ): layer( l ), layerId( l ? l->id() : QString() ) {}
  QgsMapLayerRef( const QString& id ): layer(), layerId( id ) {}

  QPointer<QgsMapLayer> layer;
  QString layerId;
};

#endif // QGSMAPLAYERREF_H
