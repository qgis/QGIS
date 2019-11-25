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

#define SIP_NO_FILE

#include <QVector>
#include <QSize>

#include "qgis_core.h"
#include "qgis.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshvectorrenderer.h"


///@cond PRIVATE

/**
 * \ingroup core
 *
 * Abstract class used to interpolate the value of the vector for a pixel
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class QgsMeshVectorValueInterpolator
{
  public:
    //! Constructor
    QgsMeshVectorValueInterpolator( const QgsTriangularMesh &triangularMesh,
                                    const QgsMeshDataBlock &datasetVectorValues );

    //! Constructor with scalar active face flag values to not interpolate on inactive face
    QgsMeshVectorValueInterpolator( const QgsTriangularMesh &triangularMesh,
                                    const QgsMeshDataBlock &datasetVectorValues,
                                    const QgsMeshDataBlock &scalarActiveFaceFlagValues );
    //! Destructor
    virtual ~QgsMeshVectorValueInterpolator() = default;


    //! Returns the interpolated vector
    virtual QgsVector vectorValue( const QgsPointXY &point ) const;

  protected:
    void updateCacheFaceIndex( const QgsPointXY &point ) const;

    QgsTriangularMesh mTriangularMesh;
    QgsMeshDataBlock mDatasetValues;
    QgsMeshDataBlock mActiveFaceFlagValues;
    mutable QgsMeshFace mFaceCache;
    mutable int mCacheFaceIndex = -1;
    bool mUseScalarActiveFaceFlagValues = false;
    bool isVectorValid( const QgsVector &v ) const;

  private:

    void activeFaceFilter( QgsVector &vector, int faceIndex ) const;

    virtual QgsVector interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const = 0;
};

/**
 * \ingroup core
 *
 * Class used to retrieve the value of the vector for a pixel from vertex
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class QgsMeshVectorValueInterpolatorFromVertex: public QgsMeshVectorValueInterpolator
{
  public:
    //! Constructor
    QgsMeshVectorValueInterpolatorFromVertex( const QgsTriangularMesh &triangularMesh,
        const QgsMeshDataBlock &datasetVectorValues );

    //! Constructor with scalar active face flag value to not interpolate on inactive face
    QgsMeshVectorValueInterpolatorFromVertex( const QgsTriangularMesh &triangularMesh,
        const QgsMeshDataBlock &datasetVectorValues,
        const QgsMeshDataBlock &scalarActiveFaceFlagValues );

  private:
    QgsVector interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const override;
};

/**
 * \ingroup core
 *
 * Class used to retrieve the value of the vector for a pixel from vertex
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class QgsMeshVectorValueInterpolatorFromFace: public QgsMeshVectorValueInterpolator
{
  public:
    //! Constructor
    QgsMeshVectorValueInterpolatorFromFace( const QgsTriangularMesh &triangularMesh,
                                            const QgsMeshDataBlock &datasetVectorValues );

    //! Constructor with scalar active face flag value to not interpolate on inactive face
    QgsMeshVectorValueInterpolatorFromFace( const QgsTriangularMesh &triangularMesh,
                                            const QgsMeshDataBlock &datasetVectorValues,
                                            const QgsMeshDataBlock &scalarActiveFaceFlagValues );

  private:
    QgsVector interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const override;
};

/**
 * \ingroup core
 *
 * Abstract class used to handle information about stream field;
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class QgsMeshStreamField
{
  public:
    //! Constructor
    QgsMeshStreamField( const QgsTriangularMesh &triangularMesh,
                        const QgsMeshDataBlock &dataSetVectorValues,
                        const QgsMeshDataBlock &scalarActiveFaceFlagValues,
                        const QgsRectangle &layerExtent,
                        double magMax, bool dataIsOnVertices, QgsRenderContext &rendererContext,
                        int resolution = 1 );

    //! Destructor
    virtual ~QgsMeshStreamField()
    {
      if ( mPainter )
        delete mPainter;
    }

    /*
    * Updates the size of the field and the QgsMapToPixel instance to retrieve map point
    * from pixel in the field depending on the resolution of the device
    * If the extent of renderer context and the resolution are not changed, do nothing
    * else, updates the size and cleans
    */
    void updateSize( const QgsRenderContext &renderContext );

    /*
    * Updates the size of the field and the QgsMapToPixel instance to retrieve map point
    * from pixel in the field depending on the resolution of the device
    */
    void updateSize( const QgsRenderContext &renderContext, int resolution );

    //! Returns true if the field is valid
    bool isValid() const;

    //! Returns the size of the field
    QSize size() const;

    //! Returns the topLeft of the field in the device coordinate
    QPoint topLeft() const;

    //! Adds a trace in the field from a start pixel
    void addTrace( QPoint startPixel );

    //! Adds a trace in the field from a map point
    void addTrace( QgsPointXY startPoint );

    //! Adds random traces in the field from random start points, the number of traces depends on the max filling density
    void addRandomTraces();

    //! Adds a trace in the field from one random start point
    void addRandomTrace();

    //! Adds traces in the field from gridded start points, pixelSpace is the space between points in pixel field
    void addGriddedTraces( int dx, int dy );

    //! Adds traces in the field from vertex on a mesh
    void addTracesOnMesh( const QgsTriangularMesh &mesh, const QgsRectangle &extent );

    //! Sets the resolution of the field
    void setResolution( int width );

    //! Returns the width of particle
    int resolution() const;

    //! Returns the size of the image that represents the trace field
    QSize imageSize() const;

    //! Returns the current render image of the field
    virtual QImage image();

    //! Sets the maximum pixel filling, eg, the rate of number pixel that can be filled with way.
    void setPixelFillingDensity( double maxFilling );

    //! Sets  color of the streamlines
    void setColor( QColor color );

    //! Sets line width of the streamlines
    void setLineWidth( double width );

    //! Sets min/max filter
    void setFilter( double min, double max );

  private:

    QgsPointXY positionToMapCoordinates( const QPoint &pixelPosition, const QgsPointXY &positionInPixel );
    QPointF fieldToDevice( const QPoint &pixel ) const;
    void initField();
    void storeInField( const QPoint &pixel );
    void drawChunkTrace( std::list<QPair<QPoint, double>> &chunkTrace );

    void simplifyChunkTrace( std::list<QPair<QPoint, double>> &chunkTrace );

    bool isTraceExists( const QPoint &pixel ) const;

    bool isTraceOutside( const QPoint &pixel ) const;

    bool filterMag( double value ) const;

    QSize mFieldSize;
    QRect mLayerPixelExtent;
    int mFieldResolution = 1;
    int mPixelFillingCount = 0;
    int mMaxPixelFillingCount = 0;
    std::unique_ptr<QgsMeshVectorValueInterpolator> mVectorValueInterpolator;
    QImage mTraceImage;
    QVector<bool> mField;
    QgsRectangle mLayerExtent;
    QgsRectangle mMapExtent;
    QgsMapToPixel mMapToFieldPixel;
    QPen mPen;
    QPainter *mPainter = nullptr;
    QPoint mFieldTopLeftInDeviceCoordinates;
    bool mValid = false;
    double mMagMax = 0;
    double mPixelFillingDensity;
    double mMinMagFilter = -1;
    double mMaxMagFilter = -1;
    QgsRenderContext &mRenderContext; //keep the renderer context only to know if the renderer is stopped
};

class QgsMeshVectorStreamLineRenderer: public QgsMeshVectorRenderer
{
  public:
    //!Constructor
    QgsMeshVectorStreamLineRenderer( const QgsTriangularMesh &triangularMesh,
                                     const QgsMeshDataBlock &dataSetVectorValues,
                                     const QgsMeshDataBlock &scalarActiveFaceFlagValues,
                                     bool dataIsOnVertices,
                                     const QgsMeshRendererVectorSettings &settings,
                                     QgsRenderContext &rendererContext,
                                     const QgsRectangle &layerExtent,
                                     double magMax );


    void draw() override;

  private:
    std::unique_ptr<QgsMeshStreamField> mStreamLineField;
    QgsRenderContext &mRendererContext;
};

///@endcond

#endif // QGSMESHTRACERENDERER_H
