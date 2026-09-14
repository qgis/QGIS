/***************************************************************************
                         qgsmeshvectorfieldvaluesource.h
                         -------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHVECTORFIELDVALUESOURCE_H
#define QGSMESHVECTORFIELDVALUESOURCE_H

#include "qgsmeshdataset.h"
#include "qgstriangularmesh.h"
#include "qgsvectorfieldvaluesource.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \ingroup core
 *
 * \brief Abstract vector field value source backed by a mesh dataset.
 *
 * Interpolates the vector value of a triangular mesh dataset at arbitrary map positions, caching
 * the last hit face because consecutive lookups of a trace usually land in the same triangle.
 *
 * \note not available in Python bindings
 * \since QGIS 4.2
 */
class QgsMeshVectorFieldValueSource : public QgsVectorFieldValueSource
{
  public:
    /**
     * Constructs a source for the vector dataset \a datasetVectorValues of \a triangularMesh.
     *
     * \a scalarActiveFaceFlagValues may be an invalid data block, in which case all faces are
     * considered active. \a datasetMagnitudeValues holds the precalculated magnitudes, used to
     * build the color ramp image, and may be empty. \a maximumMagnitude must be the maximum
     * magnitude of the whole dataset.
     */
    QgsMeshVectorFieldValueSource(
      const QgsTriangularMesh &triangularMesh,
      const QgsMeshDataBlock &datasetVectorValues,
      const QgsMeshDataBlock &scalarActiveFaceFlagValues,
      const QVector<double> &datasetMagnitudeValues,
      QgsMeshDatasetGroupMetadata::DataType dataType,
      const QgsRectangle &layerExtent,
      double maximumMagnitude
    );

    /**
     * Returns a source suitable for \a dataType, that is one interpolating from vertices for
     * QgsMeshDatasetGroupMetadata::DataOnVertices and one from faces otherwise.
     */
    static std::unique_ptr<QgsMeshVectorFieldValueSource> create(
      const QgsTriangularMesh &triangularMesh,
      const QgsMeshDataBlock &datasetVectorValues,
      const QgsMeshDataBlock &scalarActiveFaceFlagValues,
      const QVector<double> &datasetMagnitudeValues,
      QgsMeshDatasetGroupMetadata::DataType dataType,
      const QgsRectangle &layerExtent,
      double maximumMagnitude
    );

    QgsVector vectorValue( const QgsPointXY &point ) const override;
    QgsRectangle extent() const override;
    double maximumMagnitude() const override;
    QVector<QgsPointXY> seedPoints( const QgsRectangle &extent ) const override;

    /**
     * \note The returned interface keeps references to \a context, so it must not outlive it.
     */
    std::unique_ptr<QgsRasterInterface> magnitudeSource( const QgsRenderContext &context, QSize size ) const override;

  protected:
    virtual QgsVector interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const = 0;

    bool isVectorValid( const QgsVector &v ) const;

    QgsTriangularMesh mTriangularMesh;
    QgsMeshDataBlock mDatasetValues;
    QgsMeshDataBlock mActiveFaceFlagValues;
    QVector<double> mMagnitudeValues;
    QgsMeshDatasetGroupMetadata::DataType mDataType = QgsMeshDatasetGroupMetadata::DataOnVertices;
    QgsRectangle mExtent;
    double mMaximumMagnitude = 0;
    bool mUseScalarActiveFaceFlagValues = false;
    mutable int mCacheFaceIndex = -1;

  private:
    void activeFaceFilter( QgsVector &vector, int faceIndex ) const;
};

/**
 * \ingroup core
 *
 * \brief Vector field value source interpolating mesh dataset values defined on vertices.
 *
 * \note not available in Python bindings
 * \since QGIS 4.2
 */
class QgsMeshVectorFieldValueSourceFromVertex : public QgsMeshVectorFieldValueSource
{
  public:
    using QgsMeshVectorFieldValueSource::QgsMeshVectorFieldValueSource;

    QgsMeshVectorFieldValueSourceFromVertex *clone() const override;

  private:
    QgsVector interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const override;
};

/**
 * \ingroup core
 *
 * \brief Vector field value source interpolating mesh dataset values defined on faces.
 *
 * \note not available in Python bindings
 * \since QGIS 4.2
 */
class QgsMeshVectorFieldValueSourceFromFace : public QgsMeshVectorFieldValueSource
{
  public:
    using QgsMeshVectorFieldValueSource::QgsMeshVectorFieldValueSource;

    QgsMeshVectorFieldValueSourceFromFace *clone() const override;

  private:
    QgsVector interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const override;
};

///@endcond

#endif // QGSMESHVECTORFIELDVALUESOURCE_H
