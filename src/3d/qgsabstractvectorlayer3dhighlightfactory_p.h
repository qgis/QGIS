/***************************************************************************
  qgsabstractvectorlayer3dhighlightfactory_p.h
  --------------------------------------
  Date                 : December 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTVECTORLAYERH3DHIGHLIGHTFACTORY_H
#define QGSABSTRACTVECTORLAYERH3DHIGHLIGHTFACTORY_H

#define SIP_NO_FILE

///@cond PRIVATE

#include "qgsabstract3dsymbol.h"
#include "qgsfeature.h"

class Qgs3DMapSettings;
class QgsVectorLayer;
class QgsAbstract3DEngine;

namespace Qt3DCore
{
  class QEntity;
}

/**
 * \ingroup qgis_3d
 *
 * \brief Abstract factory for creating 3D highlight entities for vector layer features.
 *
 * It is responsible for generating Qt3D entities used to highlight
 * vector features in a 3D scene. Subclasses must implement create() to define
 * how highlight entities are built for a given feature.
 *
 * The prepareSymbol() method must be called to replace the feature symbol with a new symbol
 * which highlights the feature.
 * The finalizeEntities() method must be called at the end of the create() function to configure
 * the components of the newly created entities.
 *
 * \since QGIS 4.0
 */
class QgsAbstractVectorLayer3DHighlightFactory
{
  public:
    //! Constructs the factory for a layer
    QgsAbstractVectorLayer3DHighlightFactory( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vLayer );

    virtual ~QgsAbstractVectorLayer3DHighlightFactory() = default;

    /**
     * Creates 3D highlight entities for a specific feature.
     *
     * Implementations must create and return one or more entities
     * representing the highlighted version of the given feature.
     *
     * \param feature Feature to be highlighted. Its geometry must be in the 3D scene CRS.
     * \param engine 3D engine
     * \param parent Parent for the created highlight entities.
     * \param fillColor Color used to fill the highlighted geometry.
     * \param edgeColor Color used to render edges of the highlighted geometry.
     *
     * \return List of created Qt3D entities representing the highlight.
     */
    virtual QList<Qt3DCore::QEntity *> create( const QgsFeature &feature, QgsAbstract3DEngine *engine, Qt3DCore::QEntity *parent, const QColor &fillColor, const QColor &edgeColor ) = 0;

    /**
     * Prepares a 3D symbol for highlight rendering.
     *
     * This clones the given symbol and replaces its material
     * settings by a phong material with fill and edge colors.
     *
     * \param symbol Original 3D symbol used by the feature.
     * \param fillColor Color used to fill the highlighted symbol.
     * \param edgeColor Color used to render edges of the highlighted geometry.
     *
     * \return A newly prepared 3D symbol suitable for highlight rendering.
     */
    std::unique_ptr<QgsAbstract3DSymbol> prepareSymbol( const QgsAbstract3DSymbol *symbol, const QColor &fillColor, const QColor &edgeColor );

    /**
     * Finalizes newly created highlight entities.
     *
     * It configures material listeners and updates the QgsGeoTransform
     * based on the provided data origin.
     *
     * \param newEntities List of newly created Qt3D entities.
     * \param engine 3D engine
     * \param dataOrigin Origin used to transform entity coordinates.
     */
    void finalizeEntities( QList<Qt3DCore::QEntity *> newEntities, QgsAbstract3DEngine *engine, const QgsVector3D &dataOrigin );

  protected:
    Qgs3DMapSettings *mMapSettings = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

/// @endcond

#endif // QGSABSTRACTVECTORLAYERH3DHIGHLIGHTFACTORY_H
