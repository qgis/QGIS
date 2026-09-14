/***************************************************************************
                         qgsmeshtracerenderer.h
                         -------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHTRACERENDERER_H
#define QGSMESHTRACERENDERER_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsvectorfieldtraceanimationgenerator.h"

#include <QSize>
#include <QVector>

/**
 * \ingroup core
 *
 * \brief A wrapper for QgsMeshParticuleTracesField used to render the particles.
 *
 * Available for Python binding
 *
 * \since QGIS 3.12
 * \deprecated QGIS 4.4. Use QgsVectorFieldTraceAnimationGenerator instead.
 */
class CORE_EXPORT QgsMeshVectorTraceAnimationGenerator : public QgsVectorFieldTraceAnimationGenerator SIP_NODEFAULTCTORS
{
  public:
    /**
     * Constructor to use with Python binding
     *
     * \deprecated QGIS 4.4. Use QgsVectorFieldTraceAnimationGenerator::fromMeshLayer() instead.
     */
    Q_DECL_DEPRECATED QgsMeshVectorTraceAnimationGenerator( QgsMeshLayer *layer, const QgsRenderContext &rendererContext ) SIP_DEPRECATED;
};

#endif // QGSMESHTRACERENDERER_H
