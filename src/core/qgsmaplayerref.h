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
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "raster/qgsrasterlayer.h"
#include "raster/qgsrasterdataprovider.h"

/** Internal structure to keep weak pointer to QgsMapLayer or layerId
 *  if the layer is not available yet.
 *  \note not available in Python bindings
 */
template<typename TYPE>
struct _LayerRef
{
  _LayerRef( TYPE *l = nullptr ): layer( l ), layerId( l ? l->id() : QString() ) {}

  /**
   * Constructor for a weak layer reference, using a combination of layer ID,
   * \a name, public \a source and \a provider key.
   */
  _LayerRef( const QString &id, const QString &name = QString(), const QString &source = QString(), const QString &provider = QString() )
    : layer()
    , layerId( id )
    , source( source )
    , name( name )
    , provider( provider )
  {}

  QPointer<TYPE> layer;
  QString layerId;

  //! Weak reference to layer public source
  QString source;
  //! Weak reference to layer name
  QString name;
  //! Weak reference to layer provider
  QString provider;

  /**
   * Returns true if a layer matches the weak references to layer public source,
   * layer name and data provider contained in this layer reference.
   */
  bool layerMatchesSource( QgsMapLayer *layer ) const
  {
    if ( layer->publicSource() != source ||
         layer->name() != name )
      return false;

    if ( layer->dataProvider()->name() != provider )
      return false;

    return true;
  }
};

typedef _LayerRef<QgsMapLayer> QgsMapLayerRef;

#endif // QGSMAPLAYERREF_H
