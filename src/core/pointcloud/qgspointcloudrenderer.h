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
#include "qgsvector3d.h"

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
     *
     * The \a scale and \a offset arguments specify the scale and offset of the layer's int32 coordinates
     * compared to CRS coordinates respectively.
     */
    QgsPointCloudRenderContext( QgsRenderContext &context, const QgsVector3D &scale, const QgsVector3D &offset );

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

    /**
     * Returns the scale of the layer's int32 coordinates compared to CRS coords.
     */
    QgsVector3D scale() const { return mScale; }

    /**
     * Returns the offset of the layer's int32 coordinates compared to CRS coords.
     */
    QgsVector3D offset() const { return mOffset; }

    /**
     * Returns the total number of points rendered.
     */
    long pointsRendered() const;

    /**
     * Increments the count of points rendered by the specified amount.
     *
     * It is a point cloud renderer's responsibility to correctly call this after
     * rendering a block of points.
    */
    void incrementPointsRendered( long count );

  private:
#ifdef SIP_RUN
    QgsPointCloudRenderContext( const QgsPointCloudRenderContext &rh );
#endif

    QgsRenderContext &mRenderContext;
    QgsVector3D mScale;
    QgsVector3D mOffset;
    long mPointsRendered = 0;
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

    /**
     * Creates a renderer from an XML \a element.
     *
     * Caller takes ownership of the returned renderer.
     *
     * \see save()
     */
    static QgsPointCloudRenderer *load( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Saves the renderer configuration to an XML element.
     * \see load()
     */
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    /**
     * Returns a list of attributes required by this renderer. Attributes not listed in here may
     * not be requested from the provider at rendering time.
     *
     * \note the "X" and "Y" attributes will always be fetched and do not need to be explicitly
     * returned here.
     */
    virtual QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const;

    /**
     * Must be called when a new render cycle is started. A call to startRender() must always
     * be followed by a corresponding call to stopRender() after all features have been rendered.
     *
     * \see stopRender()
     *
     * \warning This method is not thread safe. Before calling startRender() in a non-main thread,
     * the renderer should instead be cloned and startRender()/stopRender() called on the clone.
     */
    virtual void startRender( QgsPointCloudRenderContext &context );

    /**
     * Must be called when a render cycle has finished, to allow the renderer to clean up.
     *
     * Calls to stopRender() must always be preceded by a call to startRender().
     *
     * \warning This method is not thread safe. Before calling startRender() in a non-main thread,
     * the renderer should instead be cloned and startRender()/stopRender() called on the clone.
     *
     * \see startRender()
     */
    virtual void stopRender( QgsPointCloudRenderContext &context );

  private:
#ifdef QGISDEBUG
    //! Pointer to thread in which startRender was first called
    QThread *mThread = nullptr;
#endif
};

#ifndef SIP_RUN

class QgsColorRamp;

class CORE_EXPORT QgsDummyPointCloudRenderer : public QgsPointCloudRenderer
{
  public:

    QgsPointCloudRenderer *clone() const override;
    void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    void startRender( QgsPointCloudRenderContext &context ) override;
    void stopRender( QgsPointCloudRenderContext &context ) override;
    QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const override;

    //! Returns z min
    double zMin() const;
    //! Sets z min
    void setZMin( double value );

    //! Returns z max
    double zMax() const;

    //! Sets z max
    void setZMax( double value );

    //! Returns pen width
    int penWidth() const;

    //! Sets pen width
    void setPenWidth( int value );

    //! Returns color ramp
    QgsColorRamp *colorRamp() const;

    //! Sets color ramp (ownership is transferrred)
    void setColorRamp( QgsColorRamp *value SIP_TRANSFER );

    //! Returns maximum allowed screen error in pixels
    float maximumScreenError() const;

    QString attribute() const;
    void setAttribute( const QString &attribute );

  private:
    double mZMin = 0, mZMax = 0;
    QString mAttribute;
    int mPenWidth = 1;
    int mPainterPenWidth = 1;
    std::unique_ptr<QgsColorRamp> mColorRamp;
    float mMaximumScreenError = 5;

};
#endif
#endif // QGSPOINTCLOUDRENDERER_H
