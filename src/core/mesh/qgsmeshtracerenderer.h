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

    //! Sets the number of frames per seconds that will be rendered
    void setFPS( int FPS ) { setFramesPerSecond( FPS ); }

    //! Sets the max number of pixels that can be go through by the particles in 1 second
    void setMaxSpeedPixel( int max ) { setMaximumSpeedPixel( max ); }
};

#endif // QGSMESHTRACERENDERER_H
