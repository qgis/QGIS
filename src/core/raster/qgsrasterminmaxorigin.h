/***************************************************************************
    qgsrasterminmaxorigin.h - Origin of min/max values
     --------------------------------------
    Date                 : Dec 2016
    Copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERMINMAXORIGIN_H
#define QGSRASTERMINMAXORIGIN_H

#include <QDomDocument>
#include "qgis_sip.h"
#include <QDomElement>

#include "qgis_core.h"

/**
 * \ingroup core
 * This class describes the origin of min/max values. It does not store by
 * itself the min/max values.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRasterMinMaxOrigin
{

  public:

    //! \brief Default cumulative cut lower limit
    static constexpr double CUMULATIVE_CUT_LOWER = 0.02;

    //! \brief Default cumulative cut upper limit
    static constexpr double CUMULATIVE_CUT_UPPER = 0.98;

    //! \brief Default standard deviation factor
    static constexpr double DEFAULT_STDDEV_FACTOR = 2.0;

    //! \brief This enumerator describes the limits used to compute min/max values
    enum Limits
    {
      None SIP_PYNAME( None_ ), //!< User defined.
      MinMax, //!< Real min-max values
      StdDev, //!< Range is [ mean - stdDevFactor() * stddev, mean + stdDevFactor() * stddev ]
      CumulativeCut //!< Range is [ min + cumulativeCutLower() * (max - min), min + cumulativeCutUpper() * (max - min) ]
    };

    //! \brief This enumerator describes the extent used to compute min/max values
    enum Extent
    {
      //! Whole raster is used to compute statistics.
      WholeRaster,
      //! Current extent of the canvas (at the time of computation) is used to compute statistics.
      CurrentCanvas,
      //! Constantly updated extent of the canvas is used to compute statistics.
      UpdatedCanvas
    };

    //! \brief This enumerator describes the accuracy used to compute statistics.
    enum StatAccuracy
    {
      //! Exact statistics.
      Exact,
      //! Approximated statistics.
      Estimated
    };

    //! \brief Default constructor.
    QgsRasterMinMaxOrigin();

    //! \brief Equality operator.
    bool operator ==( const QgsRasterMinMaxOrigin &other ) const;

    //////// Getter methods /////////////////////

    //! Returns the raster limits.
    QgsRasterMinMaxOrigin::Limits limits() const { return mLimits; }

    //! Returns the raster extent.
    QgsRasterMinMaxOrigin::Extent extent() const { return mExtent; }

    //! Returns the raster statistic accuracy.
    QgsRasterMinMaxOrigin::StatAccuracy statAccuracy() const { return mAccuracy; }

    //! Returns the lower bound of cumulative cut method (between 0 and 1).
    double cumulativeCutLower() const { return mCumulativeCutLower; }

    //! Returns the upper bound of cumulative cut method (between 0 and 1).
    double cumulativeCutUpper() const { return mCumulativeCutUpper; }

    //! Returns the factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    double stdDevFactor() const { return mStdDevFactor; }

    //////// Setter methods /////////////////////

    //! Sets the limits.
    void setLimits( QgsRasterMinMaxOrigin::Limits limits ) { mLimits = limits; }

    //! Sets the extent.
    void setExtent( QgsRasterMinMaxOrigin::Extent extent ) { mExtent = extent; }

    //! Sets the statistics accuracy.
    void setStatAccuracy( QgsRasterMinMaxOrigin::StatAccuracy accuracy ) { mAccuracy = accuracy; }

    //! Sets the lower bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutLower( double val ) { mCumulativeCutLower = val; }

    //! Sets the upper bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutUpper( double val ) { mCumulativeCutUpper = val; }

    //! Sets the factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    void setStdDevFactor( double val ) { mStdDevFactor = val; }

    //////// XML serialization /////////////////////

    //! \brief Serialize object.
    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;

    //! \brief Deserialize object.
    void readXml( const QDomElement &elem );

    //////// Static methods /////////////////////

    //! Returns a string to serialize Limits
    static QString limitsString( Limits limits );

    //! \brief Deserialize Limits
    static Limits limitsFromString( const QString &limits );

    //! Returns a string to serialize Extent
    static QString extentString( QgsRasterMinMaxOrigin::Extent extent );

    //! \brief Deserialize Extent
    static QgsRasterMinMaxOrigin::Extent extentFromString( const QString &extent );

    //! Returns a string to serialize StatAccuracy
    static QString statAccuracyString( QgsRasterMinMaxOrigin::StatAccuracy accuracy );

    //! \brief Deserialize StatAccuracy
    static QgsRasterMinMaxOrigin::StatAccuracy statAccuracyFromString( const QString &accuracy );

  private:

    Limits mLimits = None;
    Extent mExtent = WholeRaster;
    StatAccuracy mAccuracy = Estimated;
    double mCumulativeCutLower;
    double mCumulativeCutUpper;
    double mStdDevFactor;
};

#endif // QGSRASTERMINMAXORIGIN_H
