/***************************************************************************
                             qgscoreenums.h
                             ----------
    begin                : May 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOREENUMS_H
#define QGSCOREENUMS_H

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \brief Types of layers that can be added to a map
 * \since QGIS 3.8
 */
enum class QgsMapLayerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsMapLayer, LayerType ) : int
  {
  VectorLayer,
  RasterLayer,
  PluginLayer,
  MeshLayer,      //!< Added in 3.2
  VectorTileLayer, //!< Added in 3.14
  AnnotationLayer, //!< Contains freeform, georeferenced annotations. Added in QGIS 3.16
  PointCloudLayer, //!< Added in 3.18
};

/**
 * \ingroup core
 * \brief Enumeration of feature count states
 * \since QGIS 3.20
 */
enum class FeatureCountState SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorDataProvider, FeatureCountState ) : int
  {
  Uncounted = -2, //!< Feature count not yet computed
  UnknownCount = -1, //!< Provider returned an unknown feature count
};

#endif // QGSCOREENUMS_H
