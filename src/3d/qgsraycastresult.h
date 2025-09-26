/***************************************************************************
    qgsraycastresult.h
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRAYCASTRESULT_H
#define QGSRAYCASTRESULT_H

#include "qgis_3d.h"
#include "qgsraycasthit.h"

#include <QHash>
#include <QList>
#include <QPointer>

class QgsMapLayer;


/**
 * \ingroup qgis_3d
 *
 * \brief Contains the results of ray casting operations in a 3D map canvas.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsRayCastResult
{
  public:
    //! Ctor
    QgsRayCastResult();

    //! Returns TRUE is ray did not intersect any layer or terrain entity
    bool isEmpty() const;

    //! Returns TRUE is ray hit at least one entity from a layer
    bool hasLayerHits() const;

    //! Returns pointers to the map layers of entities that were intersected by the ray
    QList<QgsMapLayer *> layers() const;

    //! Returns all hits from entities of the specific \a layer
    QList<QgsRayCastHit> layerHits( QgsMapLayer *layer ) const;

    //! Returns TRUE is the ray intersected the terrain
    bool hasTerrainHits() const;

    //! Returns all terrain intersection hits
    QList<QgsRayCastHit> terrainHits() const;

    //! Returns all the hits from both layer and terrain intersections
    QList<QgsRayCastHit> allHits() const;

    //! Adds all \a hits from \a layer to the result
    void addLayerHits( QgsMapLayer *layer, const QList<QgsRayCastHit> &hits );

    //! Adds all terrain \a hits to the result
    void addTerrainHits( const QList<QgsRayCastHit> &hits );

  private:
    QHash<QgsMapLayer *, QList<QgsRayCastHit>> mLayerResults;
    QHash<QgsMapLayer *, QPointer<QgsMapLayer>> mLayerPointers;
    QList<QgsRayCastHit> mTerrainResults;
};

#endif // QGSRAYCASTRESULT_H
