/***************************************************************************
    qgsvectorfieldtraceanimationgenerator.h
    ---------------------------------------
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

#ifndef QGSVECTORFIELDTRACEANIMATIONGENERATOR_H
#define QGSVECTORFIELDTRACEANIMATIONGENERATOR_H

#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrendercontext.h"

#include <QImage>

class QgsMeshLayer;
class QgsVectorFieldParticleTracesField;
class QgsVectorFieldSettings;
class QgsVectorFieldValueSource;

/**
 * \ingroup core
 *
 * \brief Renders animated particle traces over a vector field.
 *
 * Unlike the trace symbology of a layer renderer, which draws a single static frame, this class
 * keeps the particles alive between calls so that successive imageRendered() calls produce the
 * frames of an animation.
 *
 * The vector field can come from any source, so the same generator serves mesh layers and any
 * other layer type able to expose a vector field.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldTraceAnimationGenerator SIP_NODEFAULTCTORS
{
  public:
    /**
     * Constructs a generator sampling the vector field from \a source, which is taken ownership of.
     *
     * If \a minimizeFieldSize is TRUE the animated field only covers the part of the data which is
     * currently on the device, otherwise it covers the whole rendered extent.
     */
    QgsVectorFieldTraceAnimationGenerator(
      std::unique_ptr<QgsVectorFieldValueSource> source, const QgsRenderContext &rendererContext, const QgsVectorFieldSettings &vectorSettings, bool minimizeFieldSize = true
    ) SIP_SKIP;

    /**
     * Returns a generator for the active vector dataset of \a layer, or NULLPTR if the layer has
     * no vector dataset to animate.
     *
     * The caller takes ownership of the returned generator.
     */
    static QgsVectorFieldTraceAnimationGenerator *fromMeshLayer( QgsMeshLayer *layer, const QgsRenderContext &rendererContext ) SIP_FACTORY;

    QgsVectorFieldTraceAnimationGenerator( const QgsVectorFieldTraceAnimationGenerator &other );

    ~QgsVectorFieldTraceAnimationGenerator();

    //! Seeds particles in the vector field
    void seedRandomParticles( int count );

    //! Moves all the particles using frame per second (fps) to calculate the displacement and return the rendered frame
    QImage imageRendered();

    //! Sets the number of frames per seconds that will be rendered
    void setFPS( int FPS );

    //! Sets the max number of pixels that can be go through by the particles in 1 second
    void setMaxSpeedPixel( int max );

    //! Sets maximum life time of particles in seconds
    void setParticlesLifeTime( double particleLifeTime );

    //! Sets colors of particle
    void setParticlesColor( const QColor &c );

    //! Sets particle size in px
    void setParticlesSize( double width );

    //! Sets the tail factor, used to adjust the length of the tail. 0 : minimum length, >1 increase the tail
    void setTailFactor( double fct );

    //! Sets the minimum tail length
    void setMinimumTailLength( int l );

    //! Sets the visual persistence of the tail
    void setTailPersitence( double p );

    QgsVectorFieldTraceAnimationGenerator &operator=( const QgsVectorFieldTraceAnimationGenerator &other );

  private:
    std::unique_ptr<QgsVectorFieldParticleTracesField> mParticleField;
    const QgsRenderContext &mRendererContext;
    int mFPS = 15;       //frame per second of the output, used to calculate orher parameters of the field
    int mVpixMax = 2000; //is the number of pixels that are going through for 1 s
    double mParticleLifeTime = 5;

    void updateFieldParameter();

  protected:
    /**
     * Constructs a generator for the active vector dataset of \a layer.
     *
     * Only call this when QgsVectorFieldTraceAnimationGenerator::fromMeshLayer() would return a
     * generator for \a layer, that is when the layer actually has a vector dataset to animate.
     */
    QgsVectorFieldTraceAnimationGenerator( QgsMeshLayer *layer, const QgsRenderContext &rendererContext ) SIP_SKIP;
};

#endif // QGSVECTORFIELDTRACEANIMATIONGENERATOR_H
