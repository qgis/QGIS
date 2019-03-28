/***************************************************************************
                              qgsinterpolator.h
                              ------------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINTERPOLATOR_H
#define QGSINTERPOLATOR_H

#include <QVector>
#include "qgis_sip.h"
#include "qgis_analysis.h"

class QgsFeatureSource;
class QgsGeometry;
class QgsFeedback;

/**
 * Interpolation data for an individual source vertex.
 * \since QGIS 3.0
 */
struct ANALYSIS_EXPORT QgsInterpolatorVertexData
{

  /**
   * Constructor for QgsInterpolatorVertexData with the specified
   * \a x, \a y, and \a z coordinate.
   */
  QgsInterpolatorVertexData( double x, double y, double z )
    : x( x )
    , y( y )
    , z( z )
  {}

  //! Constructor for QgsInterpolatorVertexData
  QgsInterpolatorVertexData() = default;

  //! X-coordinate
  double x = 0.0;
  //! Y-coordinate
  double y = 0.0;
  //! Z-coordinate
  double z = 0.0;
};

/**
 * \ingroup analysis
 * Interface class for interpolations. Interpolators take
 * the vertices of a vector layer as base data. The z-Value
 * can be an attribute or the z-coordinates in case of 3D types.
*/
class ANALYSIS_EXPORT QgsInterpolator
{
  public:

    //! Describes the type of input data
    enum SourceType
    {
      SourcePoints, //!< Point source
      SourceStructureLines, //!< Structure lines
      SourceBreakLines, //!< Break lines
    };

    //! Source for interpolated values from features
    enum ValueSource
    {
      ValueAttribute, //!< Take value from feature's attribute
      ValueZ, //!< Use feature's geometry Z values for interpolation
      ValueM, //!< Use feature's geometry M values for interpolation
    };

    //! Result of an interpolation operation
    enum Result
    {
      Success = 0, //!< Operation was successful
      Canceled, //!< Operation was manually canceled
      InvalidSource, //!< Operation failed due to invalid source
      FeatureGeometryError, //!< Operation failed due to invalid feature geometry
    };

    //! A source together with the information about interpolation attribute / z-coordinate interpolation and the type (point, structure line, breakline)
    struct LayerData
    {
      //! Feature source
      QgsFeatureSource *source = nullptr;
      //! Source for feature values to interpolate
      QgsInterpolator::ValueSource valueSource = QgsInterpolator::ValueAttribute;
      //! Index of feature attribute to use for interpolation
      int interpolationAttribute = -1;
      //! Source type
      QgsInterpolator::SourceType sourceType = SourcePoints;
    };

    QgsInterpolator( const QList<QgsInterpolator::LayerData> &layerData );

    virtual ~QgsInterpolator() = default;

    /**
     * Calculates interpolation value for map coordinates x, y
     * \param x x-coordinate (in map units)
     * \param y y-coordinate (in map units)
     * \param result interpolation result
     * \param feedback optional feedback object for progress and cancellation support
     * \returns 0 in case of success
     */
    virtual int interpolatePoint( double x, double y, double &result SIP_OUT, QgsFeedback *feedback = nullptr ) = 0;

    //! \note not available in Python bindings
    QList<LayerData> layerData() const { return mLayerData; } SIP_SKIP

  protected:

    /**
     * Caches the vertex and value data from the provider. All the vertex data
     * will be held in virtual memory.
     *
     * An optional \a feedback argument may be specified to allow cancellation and
     * progress reports from the cache operation.
     *
     * \returns Success in case of success
    */
    Result cacheBaseData( QgsFeedback *feedback = nullptr );

    //! Cached vertex data for input sources
    QVector<QgsInterpolatorVertexData> mCachedBaseData;

    //! Flag that tells if the cache already has been filled
    bool mDataIsCached = false;

    //! Information about the input vector layers and the attributes (or z-values) that are used for interpolation
    QList<LayerData> mLayerData;

  private:
    QgsInterpolator() = delete;

    /**
     * Helper method that adds the vertices of a geometry to the mCachedBaseData
     * \param geom the geometry
     * \param source source for values to interpolate from the feature
     * \param attributeValue the attribute value for interpolation (if interpolating from attribute value)
     *\returns 0 in case of success
    */
    bool addVerticesToCache( const QgsGeometry &geom, ValueSource source, double attributeValue );
};

#endif
