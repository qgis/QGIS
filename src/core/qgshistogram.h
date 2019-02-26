/***************************************************************************
                          qgshistogram.h
                          --------------
    begin                : May 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#ifndef QGSHISTOGRAM_H
#define QGSHISTOGRAM_H

#include <QList>

#include "qgis_core.h"
#include "qgsfeedback.h"

class QgsVectorLayer;


/**
 * \ingroup core
 * \class QgsHistogram
 * \brief Calculator for a numeric histogram from a list of values.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsHistogram
{
  public:

    /**
     * Constructor for QgsHistogram.
     */
    QgsHistogram() = default;

    virtual ~QgsHistogram() = default;

    /**
     * Assigns numeric source values for the histogram.
     * \param values list of doubles
     */
    void setValues( const QList<double> &values );

    /**
     * Assigns numeric source values for the histogram from a vector layer's field or as the
     * result of an expression.
     * \param layer vector layer
     * \param fieldOrExpression field name or expression to be evaluated
     * \param feedback optional feedback object to allow cancellation of calculation
     * \returns TRUE if values were successfully set
     */
    bool setValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, QgsFeedback *feedback = nullptr );

    /**
     * Calculates the optimal bin width using the Freedman-Diaconis rule. Bins widths are
     * determined by the inter-quartile range of values and the number of values.
     * \returns optimal width for bins
     * \see optimalNumberBins
     * \note values must first be specified using setValues()
     */
    double optimalBinWidth() const;

    /**
     * Returns the optimal number of bins for the source values, calculated using the
     * Freedman-Diaconis rule. The number of bins are determined by the inter-quartile range
     * of values and the number of values.
     * \returns optimal number of bins
     * \see optimalBinWidth
     * \note values must first be specified using setValues()
     */
    int optimalNumberBins() const;

    /**
     * Returns a list of edges for the histogram for a specified number of bins. This list
     * will be length bins + 1, as both the first and last value are also included.
     * \param bins number of bins
     * \returns list of bin edges
     * \note values must first be specified using setValues()
     */
    QList<double> binEdges( int bins ) const;

    /**
     * Returns the calculated list of the counts for the histogram bins.
     * \param bins number of histogram bins
     * \returns list of histogram counts
     * \note values must first be specified using setValues()
     */
    QList<int> counts( int bins ) const;

  private:

    QList<double> mValues;
    double mMax = 0;
    double mMin = 0;
    double mIQR = 0;

    void prepareValues();

};

#endif // QGSHISTOGRAM_H
