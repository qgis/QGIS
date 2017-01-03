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
#include <QDomElement>

#include "qgis_core.h"

/** \ingroup core
 * This class describes the origin of min/max values. It does not store by
 * itself the min/max values.
 * @note added in QGIS 3.0
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
      //! User defined.
      None,
      //! Real min-max values
      MinMax,
      //! Range is [ mean - stdDevFactor() * stddev, mean + stdDevFactor() * stddev ]
      StdDev,
      //! Range is [ min + cumulativeCutLower() * (max - min), min + cumulativeCutUpper() * (max - min) ]
      CumulativeCut
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
    bool operator ==( const QgsRasterMinMaxOrigin& other ) const;

    //////// Getter methods /////////////////////

    //! \brief Return limits.
    Limits limits() const { return mLimits; }

    //! \brief Return extent.
    Extent extent() const { return mExtent; }

    //! \brief Return statistic accuracy.
    StatAccuracy statAccuracy() const { return mAccuracy; }

    //! \brief Return lower bound of cumulative cut method (between 0 and 1).
    double cumulativeCutLower() const { return mCumulativeCutLower; }

    //! \brief Return upper bound of cumulative cut method (between 0 and 1).
    double cumulativeCutUpper() const { return mCumulativeCutUpper; }

    //! \brief Return factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    double stdDevFactor() const { return mStdDevFactor; }

    //////// Setter methods /////////////////////

    //! \brief Set limits.
    void setLimits( Limits theLimits ) { mLimits = theLimits; }

    //! \brief Set extent.
    void setExtent( Extent theExtent ) { mExtent = theExtent; }

    //! \brief Set statistics accuracy.
    void setStatAccuracy( StatAccuracy theAccuracy ) { mAccuracy = theAccuracy; }

    //! \brief Set lower bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutLower( double val ) { mCumulativeCutLower = val; }

    //! \brief Set upper bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutUpper( double val ) { mCumulativeCutUpper = val; }

    //! \brief Set factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    void setStdDevFactor( double val ) { mStdDevFactor = val; }

    //////// XML serialization /////////////////////

    //! \brief Serialize object.
    void writeXml( QDomDocument& doc, QDomElement& parentElem ) const;

    //! \brief Deserialize object.
    void readXml( const QDomElement& elem );

    //////// Static methods /////////////////////

    //! \brief Return a string to serialize Limits
    static QString limitsString( Limits theLimits );

    //! \brief Deserialize Limits
    static Limits limitsFromString( const QString& theLimits );

    //! \brief Return a string to serialize Extent
    static QString extentString( Extent theExtent );

    //! \brief Deserialize Extent
    static Extent extentFromString( const QString& theExtent );

    //! \brief Return a string to serialize StatAccuracy
    static QString statAccuracyString( StatAccuracy theAccuracy );

    //! \brief Deserialize StatAccuracy
    static StatAccuracy statAccuracyFromString( const QString& theAccuracy );

  private:

    Limits mLimits;
    Extent mExtent;
    StatAccuracy mAccuracy;
    double mCumulativeCutLower;
    double mCumulativeCutUpper;
    double mStdDevFactor;
};

#endif // QGSRASTERMINMAXORIGIN_H
