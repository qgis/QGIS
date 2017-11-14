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
#include "qgis.h"
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

    //! \brief Return limits.
    QgsRasterMinMaxOrigin::Limits limits() const { return mLimits; }

    //! \brief Return extent.
    QgsRasterMinMaxOrigin::Extent extent() const { return mExtent; }

    //! \brief Return statistic accuracy.
    QgsRasterMinMaxOrigin::StatAccuracy statAccuracy() const { return mAccuracy; }

    //! \brief Return lower bound of cumulative cut method (between 0 and 1).
    double cumulativeCutLower() const { return mCumulativeCutLower; }

    //! \brief Return upper bound of cumulative cut method (between 0 and 1).
    double cumulativeCutUpper() const { return mCumulativeCutUpper; }

    //! \brief Return factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    double stdDevFactor() const { return mStdDevFactor; }

    //////// Setter methods /////////////////////

    //! \brief Set limits.
    void setLimits( QgsRasterMinMaxOrigin::Limits limits ) { mLimits = limits; }

    //! \brief Set extent.
    void setExtent( QgsRasterMinMaxOrigin::Extent extent ) { mExtent = extent; }

    //! \brief Set statistics accuracy.
    void setStatAccuracy( QgsRasterMinMaxOrigin::StatAccuracy accuracy ) { mAccuracy = accuracy; }

    //! \brief Set lower bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutLower( double val ) { mCumulativeCutLower = val; }

    //! \brief Set upper bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutUpper( double val ) { mCumulativeCutUpper = val; }

    //! \brief Set factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    void setStdDevFactor( double val ) { mStdDevFactor = val; }

    //////// XML serialization /////////////////////

    //! \brief Serialize object.
    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;

    //! \brief Deserialize object.
    void readXml( const QDomElement &elem );

    //////// Static methods /////////////////////

    //! \brief Return a string to serialize Limits
    static QString limitsString( Limits limits );

    //! \brief Deserialize Limits
    static Limits limitsFromString( const QString &limits );

    //! \brief Return a string to serialize Extent
    static QString extentString( QgsRasterMinMaxOrigin::Extent extent );

    //! \brief Deserialize Extent
    static QgsRasterMinMaxOrigin::Extent extentFromString( const QString &extent );

    //! \brief Return a string to serialize StatAccuracy
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
