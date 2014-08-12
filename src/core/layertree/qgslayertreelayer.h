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

#include "qgslayertreenode.h"

class QgsMapLayer;

/**
 * Layer tree node points to a map layer.
 *
 * When using with existing QgsMapLayer instance, it is expected that the layer
 * has been registered in QgsMapLayerRegistry earlier.
 *
 * The node can exist also without a valid instance of a layer (just ID). That
 * means the referenced layer does not need to be loaded in order to use it
 * in layer tree. In such case, the node will start listening to map layer
 * registry updates in expectation that the layer (identified by its ID) will
 * be loaded later.
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

    explicit QgsLayerTreeLayer( QString layerId, QString name = QString() );

    QString layerId() const { return mLayerId; }

    QgsMapLayer* layer() const { return mLayer; }

    QString layerName() const;
    void setLayerName( const QString& n );

    Qt::CheckState isVisible() const { return mVisible; }
    void setVisible( Qt::CheckState visible );

    static QgsLayerTreeLayer* readXML( QDomElement& element );
    virtual void writeXML( QDomElement& parentElement );

    virtual QString dump() const;

    virtual QgsLayerTreeNode* clone() const;

  protected slots:
    void registryLayersAdded( QList<QgsMapLayer*> layers );
    void registryLayersWillBeRemoved( const QStringList& layerIds );

  signals:
    //! emitted when a previously unavailable layer got loaded
    void layerLoaded();
    //! emitted when a previously available layer got unloaded (from layer registry)
    //! @note added in 2.6
    void layerWillBeUnloaded();

  protected:
    void attachToLayer();

    QString mLayerId;
    QString mLayerName; // only used if layer does not exist
    QgsMapLayer* mLayer; // not owned! may be null
    Qt::CheckState mVisible;
};



#endif // QGSLAYERTREELAYER_H
