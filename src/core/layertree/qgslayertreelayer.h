/***************************************************************************
  qgslayertreelayer.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREELAYER_H
#define QGSLAYERTREELAYER_H

#include "qgis_core.h"
#include "qgslayertreenode.h"
#include "qgsmaplayerref.h"

class QgsMapLayer;

/** \ingroup core
 * Layer tree node points to a map layer.
 *
 * The node can exist also without a valid instance of a layer (just ID). That
 * means the referenced layer does not need to be loaded in order to use it
 * in layer tree. In such case, resolveReferences() method can be called
 * once the layer is loaded.
 *
 * A map layer is supposed to be present in one layer tree just once. It is
 * however possible that temporarily a layer exists in one tree more than just
 * once, e.g. while reordering items with drag and drop.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsLayerTreeLayer : public QgsLayerTreeNode
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeLayer( QgsMapLayer* layer );
    QgsLayerTreeLayer( const QgsLayerTreeLayer& other );

    explicit QgsLayerTreeLayer( const QString& layerId, const QString& name = QString() );

    QString layerId() const { return mRef.layerId; }

    QgsMapLayer* layer() const { return mRef.layer.data(); }

    //! Get layer's name
    //! @note added in 3.0
    QString name() const override;
    //! Set layer's name
    //! @note added in 3.0
    void setName( const QString& n ) override;

    //! Read layer node from XML. Returns new instance.
    //! Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
    static QgsLayerTreeLayer* readXml( QDomElement& element );
    //! Read layer node from XML. Returns new instance.
    //! Also resolves textual references to layers from the project (calls resolveReferences() internally).
    //! @note added in 3.0
    static QgsLayerTreeLayer* readXml( QDomElement& element, const QgsProject* project );

    virtual void writeXml( QDomElement& parentElement ) override;

    virtual QString dump() const override;

    virtual QgsLayerTreeLayer* clone() const override;

    //! Resolves reference to layer from stored layer ID (if it has not been resolved already)
    //! @note added in 3.0
    virtual void resolveReferences( const QgsProject* project ) override;

  private slots:
    //! Emits a nameChanged() signal if layer's name has changed
    //! @note added in 3.0
    void layerNameChanged();
    //! Handles the event of deletion of the referenced layer
    //! @note added in 3.0
    void layerWillBeDeleted();

  signals:
    //! emitted when a previously unavailable layer got loaded
    void layerLoaded();
    //! emitted when a previously available layer got unloaded (from layer registry)
    //! @note added in 2.6
    void layerWillBeUnloaded();

  protected:
    void attachToLayer();

    //! Weak reference to the layer (or just it's ID if the reference is not resolved yet)
    QgsMapLayerRef mRef;
    //! Layer name - only used if layer does not exist
    QString mLayerName;
};



#endif // QGSLAYERTREELAYER_H
