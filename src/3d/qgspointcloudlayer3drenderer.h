/***************************************************************************
  qgspointcloudlayer3drenderer.h
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYER3DRENDERER_H
#define QGSPOINTCLOUDLAYER3DRENDERER_H

#include "qgis_3d.h"
#include "qgis_sip.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsmaplayerref.h"
#include "qgsfeedback.h"
#include <QObject>

class QgsPointCloudLayer;
#include "qgspointcloud3dsymbol.h"
#include "qgsfeature3dhandler_p.h"

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsPointCloud3DRenderContext
 *
 * \brief Encapsulates the render context for a 3D point cloud rendering operation.
 *
 * \since QGIS 3.18
 */
class _3D_NO_EXPORT QgsPointCloud3DRenderContext : public Qgs3DRenderContext
{
  public:

    /**
     * Constructor for QgsPointCloud3DRenderContext.
     *
     * The \a zValueScale argument specifies any constant scaling factor which must be applied to z values
     * taken from the point cloud index.
     *
     * The \a zValueFixedOffset argument specifies any constant offset value which must be added to z values
     * taken from the point cloud index.
     */
    QgsPointCloud3DRenderContext( const Qgs3DMapSettings &map, const QgsCoordinateTransform &coordinateTransform, std::unique_ptr< QgsPointCloud3DSymbol > symbol,
                                  double zValueScale, double zValueFixedOffset );

    //! QgsPointCloudRenderContext cannot be copied.
    QgsPointCloud3DRenderContext( const QgsPointCloud3DRenderContext &rh ) = delete;

    //! QgsPointCloudRenderContext cannot be copied.
    QgsPointCloud3DRenderContext &operator=( const QgsPointCloud3DRenderContext & ) = delete;

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
     * Returns the symbol used for rendering the point cloud
     *
     * \see setSymbol()
     */
    QgsPointCloud3DSymbol *symbol() const { return mSymbol.get(); }

    /**
     * Sets the \a symbol used for rendering the point cloud
     * Takes ownership over the passed symbol
     * \see symbol()
     */
    void setSymbol( QgsPointCloud3DSymbol *symbol );

    /**
     * Sets the list of categories of the classification that won't be rendered
     * \see getFilteredOutValues()
     */
    void setFilteredOutCategories( const QgsPointCloudCategoryList &categories );

    /**
     * Returns a set containing the filtered out values
     * \see setFilteredOutCategories()
     */
    QSet<int> getFilteredOutValues() const;

    /**
     * Retrieves the attribute \a value from \a data at the specified \a offset, where
     * \a type indicates the original data type for the attribute.
     */
    template <typename T>
    void getAttribute( const char *data, std::size_t offset, QgsPointCloudAttribute::DataType type, T &value ) const
    {
      switch ( type )
      {
        case QgsPointCloudAttribute::UChar:
        case QgsPointCloudAttribute::Char:
          value = *( data + offset );
          return;

        case QgsPointCloudAttribute::Int32:
          value = *reinterpret_cast< const qint32 * >( data + offset );
          return;
        case QgsPointCloudAttribute::UInt32:
          value = *reinterpret_cast< const quint32 * >( data + offset );
          return;

        case QgsPointCloudAttribute::Int64:
          value = *reinterpret_cast< const qint64 * >( data + offset );
          return;
        case QgsPointCloudAttribute::UInt64:
          value = *reinterpret_cast< const quint64 * >( data + offset );
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
     * Returns TRUE if the rendering is canceled.
     */
    bool isCanceled() const;

    /**
     * Cancels rendering.
     * \see isCanceled()
     */
    void cancelRendering() const;

    /**
     * Sets the coordinate transform used to transform points from layer CRS to the map CRS
     */
    void setCoordinateTransform( const QgsCoordinateTransform &coordinateTransform );

    /**
     * Returns the coordinate transform used to transform points from layer CRS to the map CRS
     */
    QgsCoordinateTransform coordinateTransform() const { return mCoordinateTransform; }

    /**
     * Returns the feedback object used to cancel rendering and check if rendering was canceled.
     */
    QgsFeedback *feedback() const { return mFeedback.get(); }
  private:
#ifdef SIP_RUN
    QgsPointCloudRenderContext( const QgsPointCloudRenderContext &rh );
#endif
    QgsPointCloudAttributeCollection mAttributes;
    std::unique_ptr<QgsPointCloud3DSymbol> mSymbol;
    QgsPointCloudCategoryList mFilteredOutCategories;
    double mZValueScale = 1.0;
    double mZValueFixedOffset = 0;
    QgsCoordinateTransform mCoordinateTransform;
    std::unique_ptr<QgsFeedback> mFeedback;
};


/**
 * \ingroup core
 * \brief Metadata for point cloud layer 3D renderer to allow creation of its instances from XML
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloudLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata SIP_SKIP
{
  public:
    QgsPointCloudLayer3DRendererMetadata();

    //! Creates an instance of a 3D renderer based on a DOM element with renderer configuration
    QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY;
};

#endif

/**
 * \ingroup core
 * \brief 3D renderer that renders all points from a point cloud layer
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloudLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit QgsPointCloudLayer3DRenderer();

    //! Sets point cloud layer associated with the renderer
    void setLayer( QgsPointCloudLayer *layer );
    //! Returns point cloud layer associated with the renderer
    QgsPointCloudLayer *layer() const;

    QString type() const override;
    QgsPointCloudLayer3DRenderer *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map ) const override SIP_SKIP;

    /**
     * Sets the 3D \a symbol associated with the renderer.
      * Ownership of \a symbol is transferred to the renderer.
      * \see symbol()
      */
    void setSymbol( QgsPointCloud3DSymbol *symbol SIP_TRANSFER );
    //! Returns 3D symbol associated with the renderer
    const QgsPointCloud3DSymbol *symbol() const { return mSymbol.get(); }

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject &project ) override;

    /**
     * Returns the maximum screen error allowed when rendering the point cloud.
     *
     * Larger values result in a faster render with less points rendered.
     *
     * \see setMaximumScreenError()
     */
    double maximumScreenError() const;

    /**
     * Sets the maximum screen \a error allowed when rendering the point cloud.
     *
     * Larger values result in a faster render with less points rendered.
     *
     * \see maximumScreenError()
     */
    void setMaximumScreenError( double error );

    /**
     * Returns whether bounding boxes will be visible when rendering the point cloud.
     *
     * \see setShowBoundingBoxes()
     */
    bool showBoundingBoxes() const;

    /**
     * Sets whether bounding boxes will be visible when rendering the point cloud.
     *
     * \see showBoundingBoxes()
     */
    void setShowBoundingBoxes( bool showBoundingBoxes );

    /**
     * Returns the maximum number of points that will be rendered to the scene
     */
    int pointRenderingBudget() const { return mPointBudget; };

    /**
     * Sets the maximum number of points to be rendered in the scene
     */
    void setPointRenderingBudget( int budget );

  private:
    QgsMapLayerRef mLayerRef; //!< Layer used to extract mesh data from
    std::unique_ptr< QgsPointCloud3DSymbol > mSymbol;
    double mMaximumScreenError = 1.0;
    bool mShowBoundingBoxes = false;
    int mPointBudget = 1000000;

  private:
#ifdef SIP_RUN
    QgsPointCloudLayer3DRenderer( const QgsPointCloudLayer3DRenderer & );
    QgsPointCloudLayer3DRenderer &operator=( const QgsPointCloudLayer3DRenderer & );
#endif
};


#endif // QGSPOINTCLOUDLAYER3DRENDERER_H
