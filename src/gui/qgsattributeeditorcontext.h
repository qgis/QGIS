/***************************************************************************
    qgsattributeeditorcontext.h
     --------------------------------------
    Date                 : 30.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEEDITORCONTEXT_H
#define QGSATTRIBUTEEDITORCONTEXT_H

#include <QMap>
#include <QWidget>

#include <qgsdistancearea.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayertools.h>


/**
 * This class contains context information for attribute editor widgets.
 * It will be passed to embedded widgets whenever this occurs (e.g. when
 * showing an embedded form due to relations)
 */

class GUI_EXPORT QgsAttributeEditorContext
{
  public:
    QgsAttributeEditorContext();

    QWidget* proxyWidget( QgsVectorLayer* vl, int fieldIdx );
    //! @note not available in python bindings
    void addProxyWidgets( QgsVectorLayer* vl, QMap<int, QWidget*> proxyWidgets );
    void addProxyWidget( QgsVectorLayer* vl, int idx, QWidget* widget );

    void setDistanceArea( const QgsDistanceArea& distanceArea ) { mDistanceArea = distanceArea; }
    inline const QgsDistanceArea& distanceArea() { return mDistanceArea; }

    void setVectorLayerTools( QgsVectorLayerTools* vlTools ) { mVectorLayerTools = vlTools; }
    QgsVectorLayerTools* vectorLayerTools() { return mVectorLayerTools; }

    /**
     * When copying the context for another layer,  call this.
     * Will adjast the distance area for this layer
     *
     * @param layer The layer to adjust for.
     */
    void adjustForLayer( QgsVectorLayer* layer );

  private:
    QgsVectorLayerTools* mVectorLayerTools;

    //! vectorlayer => ( fieldIdx, proxyWidget )
    QMap<QgsVectorLayer*, QMap<int, QWidget*> > mProxyWidgets;

    QgsDistanceArea mDistanceArea;
};

#endif // QGSATTRIBUTEEDITORCONTEXT_H
