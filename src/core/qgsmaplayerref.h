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

#define SIP_NO_FILE

#include <QPointer>

#include "qgsmaplayer.h"
#include "qgsdataprovider.h"
#include "qgsproject.h"
#include <utility>

/**
 * Internal structure to keep weak pointer to QgsMapLayer or layerId
 *  if the layer is not available yet.
 *  \note not available in Python bindings
 */
template<typename TYPE>
struct _LayerRef
{

  /**
   * Flag for match type in weak resolution
   * \since QGIS 3.12
   */
  enum MatchType
  {
    Name = 1 << 2, //! Match layer name
    Provider = 1 << 3, //! Match layer provider name
    Source = 1 << 4, //! Match layer source
    All = Provider | Source //!< Match all
  };


  /**
   * Constructor for a layer reference from an existing map layer.
   * The layerId, source, name and provider members will automatically
   * be populated from this layer.
   */
  _LayerRef( TYPE *l = nullptr )
    : layer( l )
    , layerId( l ? l->id() : QString() )
    , source( l ? l->publicSource() : QString() )
    , name( l ? l->name() : QString() )
    , provider( l && l->dataProvider() ? l->dataProvider()->name() : QString() )
  {}

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

  /**
   * Sets the reference to point to a specified layer.
   */
  void setLayer( TYPE *l )
  {
    layer = l;
    layerId = l ? l->id() : QString();
    source = l ? l->publicSource() : QString();
    name = l ? l->name() : QString();
    provider = l && l->dataProvider() ? l->dataProvider()->name() : QString();
  }

  /**
   * Returns TRUE if the layer reference is resolved and contains a reference to an existing
   * map layer.
   */
  operator bool() const
  {
    return static_cast< bool >( layer.data() );
  }

  /**
   * Forwards the to map layer.
   */
  TYPE *operator->() const
  {
    return layer.data();
  }

  /**
   * Returns a pointer to the layer, or NULLPTR if the reference has not yet been matched
   * to a layer.
   */
  TYPE *get() const
  {
    return layer.data();
  }

  //! Weak pointer to map layer
  QPointer<TYPE> layer;

  //! Original layer ID
  QString layerId;

  //! Weak reference to layer public source
  QString source;
  //! Weak reference to layer name
  QString name;
  //! Weak reference to layer provider
  QString provider;

  /**
   * Returns TRUE if a layer matches the weak references to layer public source,
   * layer name and data provider contained in this layer reference.
   * \see resolveWeakly()
   */
  bool layerMatchesSource( QgsMapLayer *layer ) const
  {
    if ( layer->publicSource() != source ||
         layer->name() != name )
      return false;

    if ( layer->providerType() != provider )
      return false;

    return true;
  }

  /**
   * Resolves the map layer by attempting to find a layer with matching ID
   * within a \a project. If found, this reference will be updated to match
   * the found layer and the layer will be returned. If no matching layer is
   * found, NULLPTR is returned.
   * \see resolveWeakly()
   */
  TYPE *resolve( const QgsProject *project )
  {
    if ( project && !layerId.isEmpty() )
    {
      if ( TYPE *l = qobject_cast<TYPE *>( project->mapLayer( layerId ) ) )
      {
        setLayer( l );
        return l;
      }
    }
    return nullptr;
  }

  bool layerMatchesWeakly( QgsMapLayer *layer, MatchType matchType = MatchType::All ) const
  {
    // First match the name
    if ( matchType & MatchType::Name && ( layer->name().isEmpty() || layer->name() != name ) )
    {
      return false;
    }
    else
    {
      // We have found a match by name, now check the other
      // criteria
      if ( matchType & MatchType::Provider && layer->providerType() != provider )
      {
        return false;
      }
      if ( matchType & MatchType::Source && layer->publicSource() != source )
      {
        return false;
      }
      // All tests passed
      return true;
    }
  }

  /**
   * Resolves the map layer by attempting to find a matching layer
   * in a \a project using a weak match.
   *
   * First, the layer is attempted to match to project layers using the
   * layer's ID (calling this method implicitly calls resolve()).
   *
   * Failing a match by layer ID, the layer will be matched by using
   * the weak references to layer public source, layer name and data
   * provider contained in this layer reference.
   *
   * The \a matchType enum can used to refine the matching criteria
   * when using the weak reference and include layer name,
   * provider name and layer source in the equality test,
   * by default they are all checked.
   *
   * If a matching layer is found, this reference will be updated to match
   * the found layer and the layer will be returned. If no matching layer is
   * found, NULLPTR is returned.
   * \see resolve()
   * \see layerMatchesSource()
   * \see layerMatchesWeakly()
   * \see resolveByIdOrNameOnly()
   */
  TYPE *resolveWeakly( const QgsProject *project, MatchType matchType = MatchType::All )
  {
    // first try matching by layer ID
    if ( resolve( project ) )
      return layer;

    if ( project )
    {
      QList<QgsMapLayer *> layers;
      // If matching by name ...
      if ( matchType & MatchType::Name )
      {
        if ( name.isEmpty() )
        {
          return nullptr;
        }
        layers = project->mapLayersByName( name );
      }
      else // ... we need all layers
      {
        layers = project->mapLayers().values();
      }
      for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
      {
        if ( TYPE *tl = qobject_cast< TYPE *>( *it ) )
        {
          if ( layerMatchesWeakly( tl, matchType ) )
          {
            setLayer( tl );
            return tl;
          }
        }
      }
    }
    return nullptr;
  }

  /**
   * Resolves the map layer by attempting to find a matching layer
   * in a \a project using a weak match.
   *
   * First, the layer is attempted to match to project layers using the
   * layer's ID (calling this method implicitly calls resolve()).
   *
   * Failing a match by layer ID, the layer will be matched by using
   * the weak references to layer public source, layer name and data
   * provider contained in this layer reference.
   *
   * Failing a match by weak reference, the layer will be matched by using
   * the name only.
   *
   * If a matching layer is found, this reference will be updated to match
   * the found layer and the layer will be returned. If no matching layer is
   * found, NULLPTR is returned.
   * \see resolve()
   * \see layerMatchesSource()
   * \see resolveWeakly()
   * \since QGIS 3.8
   */
  TYPE *resolveByIdOrNameOnly( const QgsProject *project )
  {
    // first try by matching by layer ID, or weakly by source, name and provider
    if ( resolveWeakly( project ) )
      return layer;

    // fallback to checking by name only
    if ( project && !name.isEmpty() )
    {
      const QList<QgsMapLayer *> layers = project->mapLayersByName( name );
      for ( QgsMapLayer *l : layers )
      {
        if ( TYPE *tl = qobject_cast< TYPE *>( l ) )
        {
          setLayer( tl );
          return tl;
        }
      }
    }
    return nullptr;
  }


};

typedef _LayerRef<QgsMapLayer> QgsMapLayerRef;

#endif // QGSMAPLAYERREF_H
