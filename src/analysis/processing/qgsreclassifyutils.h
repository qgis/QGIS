/***************************************************************************
                         qgsreclassifyutils.h
                         ---------------------
    begin                : June, 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRECLASSIFYUTILS
#define QGSRECLASSIFYUTILS

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_analysis.h"
#include "qgsrasterrange.h"
#include <QVector>

class QgsRasterInterface;
class QgsProcessingFeedback;
class QgsRasterDataProvider;
class QgsRectangle;

///@cond PRIVATE

/**
 * Utility functions for reclassifying raster layers.
 * \ingroup analysis
 * \since QGIS 3.2
 */
class ANALYSIS_EXPORT QgsReclassifyUtils
{

  public:

    /**
     * Represents a single class for a reclassification operation.
     */
    class RasterClass : public QgsRasterRange
    {
      public:

        //! Default constructor for an empty class
        RasterClass() = default;

        /**
         * Constructor for RasterClass, with the specified range of min to max values.
         * The \a value argument gives the desired output value for this raster class.
         */
        RasterClass( double minValue, double maxValue, QgsRasterRange::BoundsType type, double value )
          : QgsRasterRange( minValue, maxValue, type )
          , value( value )
        {}

        //! Desired output value for class
        double value = 0;
    };

    /**
     * Prints a list of classes contained within \a classes to specified \a feedback object.
     */
    static void reportClasses( const QVector< RasterClass > &classes, QgsProcessingFeedback *feedback );

    /**
     * Checks for overlaps in a set of \a classes, reporting any overlapping
     * classes the to specified \a feedback object.
     */
    static void checkForOverlaps( const QVector< RasterClass > &classes, QgsProcessingFeedback *feedback );

    /**
     * Performs a reclassification operation on a raster source \a sourceRaster, reclassifying to the given
     * list of \a classes.
     *
     * Parameters of the raster must be given by the \a band, \a extent, \a sourceWidthPixels and
     * \a sourceHeightPixels values.
     *
     * The raster data provider \a destinationRaster will be used to save the result of the
     * reclassification operation. The caller is responsible for ensuring that this data provider
     * has been created with the same extent, pixel dimensions and CRS as the input raster.
     *
     * The nodata value for the destination should be specified via \a destNoDataValue. This
     * will be used wherever the source raster has a no data value or a source pixel value
     * does not have a matching class.
     *
     * If \a useNoDataForMissingValues is true, then any raster values which do not match to
     * a class will be changed to the no data value. Otherwise they are saved unchanged.
     *
     * The \a feedback argument gives an optional processing feedback, for progress reports
     * and cancelation.
     */
    static void reclassify( const QVector< RasterClass > &classes,
                            QgsRasterInterface *sourceRaster,
                            int band,
                            const QgsRectangle &extent,
                            int sourceWidthPixels,
                            int sourceHeightPixels,
                            QgsRasterDataProvider *destinationRaster,
                            double destNoDataValue, bool useNoDataForMissingValues,
                            QgsProcessingFeedback *feedback = nullptr );

    /**
     * Reclassifies a single \a input value, using the specified list of \a classes.
     *
     * If a matching class was found, then \a reclassified will be set to true and the
     * class output value returned.
     *
     * If no matching class was found then \a reclassified will be set to false, and the
     * original \a input value returned unchanged.
     */
    static double reclassifyValue( const QVector< RasterClass > &classes, double input, bool &reclassified )
    {
      reclassified = false;
      for ( const QgsReclassifyUtils::RasterClass &c : classes )
      {
        if ( c.contains( input ) )
        {
          reclassified = true;
          return c.value;
        }
      }
      return input;
    }

};

Q_DECLARE_TYPEINFO( QgsReclassifyUtils::RasterClass, Q_MOVABLE_TYPE );


///@endcond PRIVATE

#endif // QGSRECLASSIFYUTILS


