/***************************************************************************
                         qgspointcloudrenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDRENDERER_H
#define QGSPOINTCLOUDRENDERER_H

#include "qgsrendercontext.h"

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsPointCloudBlock;


/**
 * \ingroup core
 * \class QgsPointCloudRenderContext
 *
 * Encapsulates the render context for a 2D point cloud rendering operation.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRenderContext
{
  public:

    /**
     * Constructor for QgsPointCloudRenderContext.
     */
    QgsPointCloudRenderContext( QgsRenderContext &context );

    //! QgsPointCloudRenderContext cannot be copied.
    QgsPointCloudRenderContext( const QgsPointCloudRenderContext &rh ) = delete;

    //! QgsPointCloudRenderContext cannot be copied.
    QgsPointCloudRenderContext &operator=( const QgsPointCloudRenderContext & ) = delete;

    /**
     * Returns a reference to the context's render context.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns a reference to the context's render context.
     * \note Not available in Python bindings.
     */
    const QgsRenderContext &renderContext() const { return mRenderContext; } SIP_SKIP

  private:
#ifdef SIP_RUN
    QgsPointCloudRenderContext( const QgsPointCloudRenderContext &rh );
#endif

    QgsRenderContext &mRenderContext;

};


/**
 * \ingroup core
 * \class QgsPointCloudRenderer
 *
 * Abstract base class for 2d point cloud renderers.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRenderer
{
  public:

    /**
     * Returns a new default point cloud renderer.
     *
     * Caller takes ownership of the returned renderer.
     */
    static QgsPointCloudRenderer *defaultRenderer() SIP_FACTORY;


    virtual ~QgsPointCloudRenderer() = default;

    /**
     * Create a deep copy of this renderer. Should be implemented by all subclasses
     * and generate a proper subclass.
     */
    virtual QgsPointCloudRenderer *clone() const = 0 SIP_FACTORY;

    /**
     * Renders a \a block of point cloud data using the specified render \a context.
     */
    virtual void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) = 0;

};


#endif // QGSPOINTCLOUDRENDERER_H
