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

/**
 * Layer Node
 *
 * It is expected that the layer is registered in QgsMapLayerRegistry.
 *
 * One layer is supposed to be present in one layer tree just once. It is possible that temporarily a layer
 * temporarily exists in one tree more than once, e.g. while reordering items.
 *
 * Can exist also without a valid instance of a layer (just ID),
 * so that referenced layer does not need to be loaded in order to use it in layer tree.
 */
class QgsLayerTreeLayer : public QgsLayerTreeNode
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeLayer( QgsMapLayer* layer );
    QgsLayerTreeLayer( const QgsLayerTreeLayer& other );

    explicit QgsLayerTreeLayer( QString layerId, QString name = QString() );

    QString layerId() const { return mLayerId; }

    QgsMapLayer* layer() const { return mLayer; }

    QString layerName() const { return mLayer ? mLayer->name() : mLayerName; }
    void setLayerName( const QString& n ) { if ( mLayer ) mLayer->setLayerName( n ); else mLayerName = n; }

    Qt::CheckState isVisible() const { return mVisible; }
    void setVisible( Qt::CheckState visible );

    static QgsLayerTreeLayer* readXML( QDomElement& element );
    virtual void writeXML( QDomElement& parentElement );

    virtual QString dump() const;

    virtual QgsLayerTreeNode* clone() const;

  protected slots:
    void registryLayersAdded( QList<QgsMapLayer*> layers );

  signals:
    //! emitted when a previously unavailable layer got loaded
    void layerLoaded();

  protected:
    void attachToLayer();

    QString mLayerId;
    QString mLayerName; // only used if layer does not exist
    QgsMapLayer* mLayer; // not owned! may be null
    Qt::CheckState mVisible;
};



#endif // QGSLAYERTREELAYER_H
