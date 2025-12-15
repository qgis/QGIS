/***************************************************************************
  qgsvectorlayer3dhighlightfactory_p.h
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

#ifndef QGSVECTORLAYER3DHIGHLIGHTFACTORY_H
#define QGSVECTORLAYER3DHIGHLIGHTFACTORY_H

#define SIP_NO_FILE

///@cond PRIVATE

#include "qgsabstractvectorlayer3dhighlightfactory_p.h"
#include "qgsfeature.h"

class Qgs3DMapSettings;
class QgsVectorLayer;
class QgsAbstract3DSymbol;

namespace Qt3DCore
{
  class QEntity;
}

/**
 * \ingroup qgis_3d
 *
 * \brief 3D highlight factory based on a single 3D symbol.
 *
 * This factory creates 3D highlight entities for vector layer features
 * using a single 3D symbol by replacing the existing symbol with a symbol with highlight colors.
 *
 * \since QGIS 4.0
 */
class QgsVectorLayer3DHighlightFactory : public QgsAbstractVectorLayer3DHighlightFactory
{
  public:
    QgsVectorLayer3DHighlightFactory( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vLayer, QgsAbstract3DSymbol *symbol );

    QList<Qt3DCore::QEntity *> create( const QgsFeature &feature, QgsAbstract3DEngine *engine, Qt3DCore::QEntity *parent, const QColor &fillColor, const QColor &edgeColor ) override;

  private:
    QgsAbstract3DSymbol *mSymbol = nullptr;
};

/// @endcond

#endif // QGSVECTORLAYER3DHIGHLIGHTFACTORY_H
