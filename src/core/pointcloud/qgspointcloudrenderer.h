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
#include "qgspointcloudattribute.h"

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

    /**
     * Returns the attributes associated with the rendered block.
     *
     * \see setAttributes()
     */
    QgsPointCloudAttributeCollection attributes() const { return mAttributes; }

    /**
     * Sets the \a attributes associated with the rendered block.
     *
     * \see attributes()
     */
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

    /**
     * Returns the size of a single point record.
     */
    int pointRecordSize() const { return mPointRecordSize; }

    /**
     * Returns the offset for the x value in a point record.
     *
     * \see yOffset()
     * \see zOffset()
     */
    int xOffset() const { return mXOffset; }

    /**
     * Returns the offset for the y value in a point record.
     *
     * \see xOffset()
     * \see zOffset()
     */
    int yOffset() const { return mYOffset; }

    /**
     * Returns the offset for the y value in a point record.
     *
     * \see xOffset()
     * \see yOffset()
     */
    int zOffset() const { return mZOffset; }

  private:
#ifdef SIP_RUN
    QgsPointCloudRenderContext( const QgsPointCloudRenderContext &rh );
#endif

    QgsRenderContext &mRenderContext;
    QgsVector3D mScale;
    QgsVector3D mOffset;
    long mPointsRendered = 0;
    QgsPointCloudAttributeCollection mAttributes;
    int mPointRecordSize = 0;
    int mXOffset = 0;
    int mYOffset = 0;
    int mZOffset = 0;
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

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE

    const QString type = sipCpp->type();

    if ( type == QLatin1String( "rgb" ) )
      sipType = sipType_QgsPointCloudRgbRenderer;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsPointCloudRenderer.
     */
    QgsPointCloudRenderer() = default;

    virtual ~QgsPointCloudRenderer() = default;

    /**
     * Returns the identifier of the renderer type.
     */
    virtual QString type() const = 0;

    /**
     * Create a deep copy of this renderer. Should be implemented by all subclasses
     * and generate a proper subclass.
     */
    virtual QgsPointCloudRenderer *clone() const = 0 SIP_FACTORY;

    //! QgsPointCloudRenderer cannot be copied -- use clone() instead
    QgsPointCloudRenderer( const QgsPointCloudRenderer &other ) = delete;

    //! QgsPointCloudRenderer cannot be copied -- use clone() instead
    QgsPointCloudRenderer &operator=( const QgsPointCloudRenderer &other ) = delete;

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

    /**
     * Sets the point \a size. Point size units are specified via setPointSizeUnit().
     * \see pointSize()
     * \see setPointSizeUnit()
     * \see setPointSizeMapUnitScale()
     */
    void setPointSize( double size ) { mPointSize = size; }

    /**
     * Returns the point size.
     *
     * The point size units are retrieved by calling pointSizeUnit().
     *
     * \see setPointSize()
     * \see pointSizeUnit()
     * \see pointSizeMapUnitScale()
     */
    double pointSize() const { return mPointSize; }

    /**
     * Sets the \a units used for the point size.
     *
     * \see setPointSize()
     * \see pointSizeUnit()
     * \see setPointSizeMapUnitScale()
     */
    void setPointSizeUnit( const QgsUnitTypes::RenderUnit units ) { mPointSizeUnit = units; }

    /**
     * Returns the units used for the point size.
     * \see setPointSizeUnit()
     * \see pointSize()
     * \see pointSizeMapUnitScale()
     */
    QgsUnitTypes::RenderUnit pointSizeUnit() const { return mPointSizeUnit; }

    /**
     * Sets the map unit \a scale used for the point size.
     * \see pointSizeMapUnitScale()
     * \see setPointSize()
     * \see setPointSizeUnit()
     */
    void setPointSizeMapUnitScale( const QgsMapUnitScale &scale ) { mPointSizeMapUnitScale = scale; }

    /**
     * Returns the map unit scale used for the point size.
     * \see setPointSizeMapUnitScale()
     * \see pointSizeUnit()
     * \see pointSize()
     */
    const QgsMapUnitScale &pointSizeMapUnitScale() const { return mPointSizeMapUnitScale; }

    /**
     * Returns the maximum screen error allowed when rendering the point cloud.
     *
     * Larger values result in a faster render with less points rendered.
     *
     * Units are retrieved via maximumScreenErrorUnit().
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     */
    double maximumScreenError() const;

    /**
     * Sets the maximum screen \a error allowed when rendering the point cloud.
     *
     * Larger values result in a faster render with less points rendered.
     *
     * Units are set via setMaximumScreenErrorUnit().
     *
     * \see maximumScreenError()
     * \see setMaximumScreenErrorUnit()
     */
    void setMaximumScreenError( double error );

    /**
     * Returns the unit for the maximum screen error allowed when rendering the point cloud.
     *
     * \see maximumScreenError()
     * \see setMaximumScreenErrorUnit()
     */
    QgsUnitTypes::RenderUnit maximumScreenErrorUnit() const;

    /**
     * Sets the \a unit for the maximum screen error allowed when rendering the point cloud.
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     */
    void setMaximumScreenErrorUnit( QgsUnitTypes::RenderUnit unit );

  protected:

    /**
     * Retrieves the x and y coordinate for the point at index \a i.
     */
    static void pointXY( QgsPointCloudRenderContext &context, const char *ptr, int i, double &x, double &y )
    {
      qint32 ix = *( qint32 * )( ptr + i * context.pointRecordSize() + context.xOffset() );
      qint32 iy = *( qint32 * )( ptr + i * context.pointRecordSize() + context.yOffset() );

      x = context.offset().x() + context.scale().x() * ix;
      y = context.offset().y() + context.scale().y() * iy;
    }

    /**
     * Copies common point cloud properties (such as point size and screen error) to the \a destination renderer.
     */
    void copyCommonProperties( QgsPointCloudRenderer *destination ) const;

    /**
     * Restores common renderer properties (such as point size and screen error) from the
     * specified DOM \a element.
     *
     * \see saveCommonProperties()
     */
    void restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Saves common renderer properties (such as point size and screen error) to the
     * specified DOM \a element.
     *
     * \see restoreCommonProperties()
     */
    void saveCommonProperties( QDomElement &element, const QgsReadWriteContext &context ) const;

  private:
#ifdef SIP_RUN
    QgsPointCloudRenderer( const QgsPointCloudRenderer &other );
#endif

#ifdef QGISDEBUG
    //! Pointer to thread in which startRender was first called
    QThread *mThread = nullptr;
#endif

    double mMaximumScreenError = 5.0;
    QgsUnitTypes::RenderUnit mMaximumScreenErrorUnit = QgsUnitTypes::RenderMillimeters;

    double mPointSize = 1;
    QgsUnitTypes::RenderUnit mPointSizeUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mPointSizeMapUnitScale;

};

#ifndef SIP_RUN

#include "qgscolorramp.h"


///@cond PRIVATE

class CORE_EXPORT QgsDummyPointCloudRenderer : public QgsPointCloudRenderer
{
  public:

    QgsDummyPointCloudRenderer();

    QgsPointCloudRenderer *clone() const override;
    void renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context ) override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    void startRender( QgsPointCloudRenderContext &context ) override;
    void stopRender( QgsPointCloudRenderContext &context ) override;
    QSet< QString > usedAttributes( const QgsPointCloudRenderContext &context ) const override;

    QString type() const override { return QStringLiteral( "dummy" ); }

    /**
     * Creates a dummy renderer from an XML \a element.
     */
    static QgsPointCloudRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

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
    double mZMin = 0, mZMax = 100;
    QString mAttribute = "Z";
    int mPenWidth = 1;
    int mPainterPenWidth = 1;
    std::unique_ptr<QgsColorRamp> mColorRamp;
    float mMaximumScreenError = 5;

};

///@endcond
#endif
#endif // QGSPOINTCLOUDRENDERER_H
