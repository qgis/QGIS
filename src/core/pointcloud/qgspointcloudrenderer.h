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
class QgsLayerTreeLayer;
class QgsLayerTreeModelLegendNode;
class QgsPointCloudLayer;
class QgsElevationMap;

/**
 * \ingroup core
 * \class QgsPointCloudRenderContext
 *
 * \brief Encapsulates the render context for a 2D point cloud rendering operation.
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
     *
     * The \a zValueScale argument specifies any constant scaling factor which must be applied to z values
     * taken from the point cloud index.
     *
     * The \a zValueFixedOffset argument specifies any constant offset value which must be added to z values
     * taken from the point cloud index.
     */
    QgsPointCloudRenderContext( QgsRenderContext &context, const QgsVector3D &scale, const QgsVector3D &offset,
                                double zValueScale, double zValueFixedOffset, QgsFeedback *feedback = nullptr );

    QgsPointCloudRenderContext( const QgsPointCloudRenderContext &rh ) = delete;
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
     * Sets the scale of the layer's int32 coordinates compared to CRS coords.
     * \since QGIS 3.20
     */
    void setScale( const QgsVector3D &scale ) { mScale = scale; }

    /**
     * Returns the offset of the layer's int32 coordinates compared to CRS coords.
     */
    QgsVector3D offset() const { return mOffset; }

    /**
     * Sets the offset of the layer's int32 coordinates compared to CRS coords.
     * \since QGIS 3.20
     */
    void setOffset( const QgsVector3D &offset ) { mOffset = offset; }

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

    /**
     * Returns any constant scaling factor which must be applied to z values taken from the point cloud index.
     *
     * \note Scaling of z values should be applied before the zValueFixedOffset().
     */
    double zValueScale() const { return mZValueScale; }

    /**
     * Returns any constant offset which must be applied to z values taken from the point cloud index.
     *
     * \note Scaling of z values via zValueScale() should be applied before the zValueFixedOffset().
     */
    double zValueFixedOffset() const { return mZValueFixedOffset; }

    /**
     * Returns the feedback object used to cancel rendering
     *
     * \since QGIS 3.20
     */
    QgsFeedback *feedback() const { return mFeedback; }

#ifndef SIP_RUN

    /**
     * Retrieves the attribute \a value from \a data at the specified \a offset, where
     * \a type indicates the original data type for the attribute.
     */
    template <typename T>
    static void getAttribute( const char *data, std::size_t offset, QgsPointCloudAttribute::DataType type, T &value )
    {
      switch ( type )
      {
        case QgsPointCloudAttribute::UChar:
          value = *reinterpret_cast< const unsigned char * >( data + offset );
          return;
        case QgsPointCloudAttribute::Char:
          value = *( data + offset );
          return;

        case QgsPointCloudAttribute::UInt32:
          value = *reinterpret_cast< const quint32 * >( data + offset );
          return;
        case QgsPointCloudAttribute::Int32:
          value = *reinterpret_cast< const qint32 * >( data + offset );
          return;

        case QgsPointCloudAttribute::UInt64:
          value = *reinterpret_cast< const quint64 * >( data + offset );
          return;
        case QgsPointCloudAttribute::Int64:
          value = *reinterpret_cast< const qint64 * >( data + offset );
          return;

        case QgsPointCloudAttribute::Short:
          value = *reinterpret_cast< const short * >( data + offset );
          return;

        case QgsPointCloudAttribute::UShort:
          value = *reinterpret_cast< const unsigned short * >( data + offset );
          return;

        case QgsPointCloudAttribute::Float:
          value = *reinterpret_cast< const float * >( data + offset );
          return;

        case QgsPointCloudAttribute::Double:
          value = *reinterpret_cast< const double * >( data + offset );
          return;
      }
    }
#endif

#ifndef SIP_RUN    // this is only meant for low-level rendering in C++ code

    /**
     * Helper data structure used when rendering points as triangulated surface.
     * We populate the structure as we traverse the nodes and then run Delaunay
     * triangulation at the end + draw the triangles.
     * \since QGIS 3.36
     */
    struct TriangulationData
    {
      std::vector<double> points;     //!< X,Y for each point - kept in this structure so that we can use it without further conversions in Delaunator-cpp
      std::vector<QRgb> colors;       //!< RGB color for each point
      std::vector<float> elevations;  //!< Z value for each point (only used when global map shading is enabled)
    };

    /**
     * Returns reference to the triangulation data structure (only used when rendering as triangles is enabled)
     * \since QGIS 3.36
     */
    TriangulationData &triangulationData() { return mTriangulationData; }
#endif

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
    double mZValueScale = 1.0;
    double mZValueFixedOffset = 0;

    QgsFeedback *mFeedback = nullptr;

    TriangulationData mTriangulationData;
};

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsPreparedPointCloudRendererData
 *
 * \brief Base class for 2d point cloud renderer prepared data containers.
 * \note Not available in Python bindings
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPreparedPointCloudRendererData
{
  public:

    virtual ~QgsPreparedPointCloudRendererData();

    /**
     * Returns the set of attributes used by the prepared point cloud renderer.
     */
    virtual QSet< QString > usedAttributes() const = 0;

    /**
     * Prepares the renderer for using the specified \a block.
     *
     * Returns FALSE if preparation failed.
     */
    virtual bool prepareBlock( const QgsPointCloudBlock *block ) = 0;

    /**
     * An optimised method of retrieving the color of a point from a point cloud block.
     *
     * Before calling this method prepareBlock() must be called for each incoming point cloud block.
     *
     * \since QGIS 3.26
     */
    virtual QColor pointColor( const QgsPointCloudBlock *block, int i, double z ) = 0;

};

#endif


/**
 * \ingroup core
 * \class QgsPointCloudRenderer
 *
 * \brief Abstract base class for 2d point cloud renderers.
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
    else if ( type == QLatin1String( "ramp" ) )
      sipType = sipType_QgsPointCloudAttributeByRampRenderer;
    else if ( type == QLatin1String( "classified" ) )
      sipType = sipType_QgsPointCloudClassifiedRenderer;
    else if ( type == QLatin1String( "extent" ) )
      sipType = sipType_QgsPointCloudExtentRenderer;
    else
      sipType = 0;
    SIP_END
#endif

  public:

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
     * Returns the list of visible points of the point cloud layer \a layer and an extent defined by
     * a geometry in the 2D plane \a geometry.
     *
     * The \a toleranceForPointIdentification argument can be used to specify a minimum tolerance allowable when
     * identify from a point \a geometry value. This must be specified in the map units associated with the render \a context.
     *
     * \warning The \a geometry value must be specified in the render context's destination CRS, not the layer's native CRS!
     */
    QVector<QVariantMap> identify( QgsPointCloudLayer *layer, const QgsRenderContext &context, const QgsGeometry &geometry, double toleranceForPointIdentification = 0 ) SIP_SKIP;

    /**
     * Checks whether the point holding \a pointAttributes attributes will be rendered
     * By default if not overridden in the subclass renderer will return true
     * ( the renderer is responsible for the filtering behavior )
     */
    virtual bool willRenderPoint( const QMap<QString, QVariant> &pointAttributes )
    {
      Q_UNUSED( pointAttributes );
      return true;
    }

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
     * Returns prepared data container for bulk point color retrieval.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.26
     */
    virtual std::unique_ptr< QgsPreparedPointCloudRendererData > prepare() SIP_SKIP;

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
     * Returns TRUE if the legend item with the specified \a key is checked.
     *
     * \see checkLegendItem()
     */
    virtual bool legendItemChecked( const QString &key );

    /**
     * Called when the check state of the legend item with the specified \a key is changed.
     *
     * \see legendItemChecked()
     */
    virtual void checkLegendItem( const QString &key, bool state = true );

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
    void setPointSizeUnit( const Qgis::RenderUnit units ) { mPointSizeUnit = units; }

    /**
     * Returns the units used for the point size.
     * \see setPointSizeUnit()
     * \see pointSize()
     * \see pointSizeMapUnitScale()
     */
    Qgis::RenderUnit pointSizeUnit() const { return mPointSizeUnit; }

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
     * Returns the drawing order used by the renderer for drawing points.
     *
     * \see setDrawOrder2d()
     * \since QGIS 3.24
     */
    Qgis::PointCloudDrawOrder drawOrder2d() const;

    /**
     * Sets the drawing \a order used by the renderer for drawing points.
     *
     * \see drawOrder2d()
     * \since QGIS 3.24
     */
    void setDrawOrder2d( Qgis::PointCloudDrawOrder order );

    /**
     * Returns the symbol used by the renderer for drawing points.
     *
     * \see setPointSymbol()
     */
    Qgis::PointCloudSymbol pointSymbol() const;

    /**
     * Sets the \a symbol used by the renderer for drawing points.
     *
     * \see pointSymbol()
     */
    void setPointSymbol( Qgis::PointCloudSymbol symbol );

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
    Qgis::RenderUnit maximumScreenErrorUnit() const;

    /**
     * Sets the \a unit for the maximum screen error allowed when rendering the point cloud.
     *
     * \see setMaximumScreenError()
     * \see maximumScreenErrorUnit()
     */
    void setMaximumScreenErrorUnit( Qgis::RenderUnit unit );

    /**
     * Returns whether points are triangulated to render solid surface
     *
     * \since QGIS 3.36
     */
    bool renderAsTriangles() const { return mRenderAsTriangles; }

    /**
     * Sets whether points are triangulated to render solid surface
     *
     * \since QGIS 3.36
     */
    void setRenderAsTriangles( bool asTriangles ) { mRenderAsTriangles = asTriangles; }

    /**
     * Returns whether large triangles will get rendered. This only applies when renderAsTriangles()
     * is enabled. When the triangle filtering is enabled, triangles where at least one side is
     * horizontally longer than the threshold in horizontalTriangleFilterThreshold() do not get rendered.
     *
     * \see horizontalTriangleFilterThreshold()
     * \see horizontalTriangleFilterUnit()
     * \see setHorizontalTriangleFilter()
     * \since QGIS 3.36
     */
    bool horizontalTriangleFilter() const { return mHorizontalTriangleFilter; }

    /**
     * Sets whether large triangles will get rendered. This only applies when renderAsTriangles()
     * is enabled. When the triangle filtering is enabled, triangles where at least one side is
     * horizontally longer than the threshold in horizontalTriangleFilterThreshold() do not get rendered.
     *
     * \see setHorizontalTriangleFilterThreshold()
     * \see setHorizontalTriangleFilterUnit()
     * \see horizontalTriangleFilter()
     * \since QGIS 3.36
     */
    void setHorizontalTriangleFilter( bool enabled ) { mHorizontalTriangleFilter = enabled; }

    /**
     * Returns threshold for filtering of triangles. This only applies when renderAsTriangles() and
     * horizontalTriangleFilter() are both enabled. If any edge of a triangle is horizontally longer
     * than the threshold, such triangle will not get rendered. Units of the threshold value are
     * given by horizontalTriangleFilterUnits().
     *
     * \see horizontalTriangleFilter()
     * \see horizontalTriangleFilterUnit()
     * \see setHorizontalTriangleFilterThreshold()
     * \since QGIS 3.36
     */
    double horizontalTriangleFilterThreshold() const { return mHorizontalTriangleFilterThreshold; }

    /**
     * Sets threshold for filtering of triangles. This only applies when renderAsTriangles() and
     * horizontalTriangleFilter() are both enabled. If any edge of a triangle is horizontally longer
     * than the threshold, such triangle will not get rendered. Units of the threshold value are
     * given by horizontalTriangleFilterUnits().
     *
     * \see horizontalTriangleFilter()
     * \see horizontalTriangleFilterUnit()
     * \see horizontalTriangleFilterThreshold()
     * \since QGIS 3.36
     */
    void setHorizontalTriangleFilterThreshold( double threshold ) { mHorizontalTriangleFilterThreshold = threshold; }

    /**
     * Returns units of the threshold for filtering of triangles. This only applies when renderAsTriangles() and
     * horizontalTriangleFilter() are both enabled.
     *
     * \see horizontalTriangleFilter()
     * \see horizontalTriangleFilterThreshold()
     * \see setHorizontalTriangleFilterUnit()
     * \since QGIS 3.36
     */
    Qgis::RenderUnit horizontalTriangleFilterUnit() const { return mHorizontalTriangleFilterUnit; }

    /**
     * Sets units of the threshold for filtering of triangles. This only applies when renderAsTriangles() and
     * horizontalTriangleFilter() are both enabled.
     *
     * \see horizontalTriangleFilter()
     * \see horizontalTriangleFilterThreshold()
     * \see horizontalTriangleFilterUnit()
     * \since QGIS 3.36
     */
    void setHorizontalTriangleFilterUnit( Qgis::RenderUnit unit ) { mHorizontalTriangleFilterUnit = unit; }

    /**
     * Creates a set of legend nodes representing the renderer.
     */
    virtual QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY;

    /**
     * Returns a list of all rule keys for legend nodes created by the renderer.
     */
    virtual QStringList legendRuleKeys() const;

  protected:

    /**
     * Retrieves the x and y coordinate for the point at index \a i.
     */
    static void pointXY( QgsPointCloudRenderContext &context, const char *ptr, int i, double &x, double &y )
    {
      // be wary when copying this code!! In the renderer we explicitly request x/y/z as qint32 values, but in other
      // situations these may be floats or doubles!
      const qint32 ix = *reinterpret_cast< const qint32 * >( ptr + i * context.pointRecordSize() + context.xOffset() );
      const qint32 iy = *reinterpret_cast< const qint32 * >( ptr + i * context.pointRecordSize() + context.yOffset() );
      x = context.offset().x() + context.scale().x() * ix;
      y = context.offset().y() + context.scale().y() * iy;
    }

    /**
     * Retrieves the z value for the point at index \a i.
     */
    static double pointZ( QgsPointCloudRenderContext &context, const char *ptr, int i )
    {
      // be wary when copying this code!! In the renderer we explicitly request x/y/z as qint32 values, but in other
      // situations these may be floats or doubles!
      const qint32 iz = *reinterpret_cast<const qint32 * >( ptr + i * context.pointRecordSize() + context.zOffset() );
      return ( context.offset().z() + context.scale().z() * iz ) * context.zValueScale() + context.zValueFixedOffset();
    }

    /**
     * Draws a point using a \a color at the specified \a x and \a y (in map coordinates).
     */
    void drawPoint( double x, double y, const QColor &color, QgsPointCloudRenderContext &context ) const
    {
      drawPoint( x, y, color, mDefaultPainterPenWidth, context );
    }

    /**
     * Draws a point using a \a color and painter \a width at the specified \a x and \a y (in map coordinates).
     *
     * \since QGIS 3.36
     */
    void drawPoint( double x, double y, const QColor &color, int width, QgsPointCloudRenderContext &context ) const
    {
      const QPointF originalXY( x, y );
      context.renderContext().mapToPixel().transformInPlace( x, y );
      QPainter *painter = context.renderContext().painter();
      switch ( mPointSymbol )
      {
        case Qgis::PointCloudSymbol::Square:
          painter->fillRect( QRectF( x - width * 0.5,
                                     y - width * 0.5,
                                     width, width ), color );
          break;

        case Qgis::PointCloudSymbol::Circle:
          painter->setBrush( QBrush( color ) );
          painter->setPen( Qt::NoPen );
          painter->drawEllipse( QRectF( x - width * 0.5,
                                        y - width * 0.5,
                                        width, width ) );
          break;
      };
    }

#ifndef SIP_RUN   // intentionally left out from SIP to avoid API breaks in future when we move elevation post-processing elsewhere

    /**
     * Draws a point at the elevation \a z using at the specified \a x and \a y (in map coordinates) on the elevation map.
     * \since QGIS 3.28
     */
    void drawPointToElevationMap( double x, double y, double z, QgsPointCloudRenderContext &context ) const;

    /**
     * Draws a point at the elevation \a z using at the specified \a x and \a y (in map coordinates) and painter \a width on the elevation map.
     * \since QGIS 3.36
     */
    void drawPointToElevationMap( double x, double y, double z, int width, QgsPointCloudRenderContext &context ) const;
#endif

    /**
     * Adds a point to the list of points to be triangulated (only used when renderAsTriangles() is enabled)
     * \since QGIS 3.36
     */
    void addPointToTriangulation( double x, double y, double z, const QColor &color, QgsPointCloudRenderContext &context )
    {
      QgsPointXY p = context.renderContext().mapToPixel().transform( x, y ) * context.renderContext().devicePixelRatio();
      QgsPointCloudRenderContext::TriangulationData &triangulation = context.triangulationData();
      triangulation.points.push_back( p.x() );
      triangulation.points.push_back( p.y() );
      triangulation.colors.push_back( color.rgb() );
      if ( context.renderContext().elevationMap() )
        triangulation.elevations.push_back( static_cast<float>( z ) );
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

    double mMaximumScreenError = 0.3;
    Qgis::RenderUnit mMaximumScreenErrorUnit = Qgis::RenderUnit::Millimeters;

    double mPointSize = 1;
    Qgis::RenderUnit mPointSizeUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mPointSizeMapUnitScale;

    Qgis::PointCloudSymbol mPointSymbol = Qgis::PointCloudSymbol::Square;
    int mDefaultPainterPenWidth = 1;
    Qgis::PointCloudDrawOrder mDrawOrder2d = Qgis::PointCloudDrawOrder::Default;

    bool mRenderAsTriangles = false;
    bool mHorizontalTriangleFilter = false;
    double mHorizontalTriangleFilterThreshold = 5.0;
    Qgis::RenderUnit mHorizontalTriangleFilterUnit = Qgis::RenderUnit::Millimeters;
};

#endif // QGSPOINTCLOUDRENDERER_H
