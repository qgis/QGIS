/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgshistogram.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsHistogram
{
%Docstring
 Calculator for a numeric histogram from a list of values.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgshistogram.h"
%End
  public:

    QgsHistogram();
%Docstring
 Constructor for QgsHistogram.
%End

    virtual ~QgsHistogram();

    void setValues( const QList<double> &values );
%Docstring
 Assigns numeric source values for the histogram.
 \param values list of doubles
%End

    bool setValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, QgsFeedback *feedback = 0 );
%Docstring
 Assigns numeric source values for the histogram from a vector layer's field or as the
 result of an expression.
 \param layer vector layer
 \param fieldOrExpression field name or expression to be evaluated
 \param feedback optional feedback object to allow cancelation of calculation
 :return: true if values were successfully set
 :rtype: bool
%End

    double optimalBinWidth() const;
%Docstring
 Calculates the optimal bin width using the Freedman-Diaconis rule. Bins widths are
 determined by the inter-quartile range of values and the number of values.
 :return: optimal width for bins
.. seealso:: optimalNumberBins
.. note::

   values must first be specified using setValues()
 :rtype: float
%End

    int optimalNumberBins() const;
%Docstring
 Returns the optimal number of bins for the source values, calculated using the
 Freedman-Diaconis rule. The number of bins are determined by the inter-quartile range
 of values and the number of values.
 :return: optimal number of bins
.. seealso:: optimalBinWidth
.. note::

   values must first be specified using setValues()
 :rtype: int
%End

    QList<double> binEdges( int bins ) const;
%Docstring
 Returns a list of edges for the histogram for a specified number of bins. This list
 will be length bins + 1, as both the first and last value are also included.
 \param bins number of bins
 :return: list of bin edges
.. note::

   values must first be specified using setValues()
 :rtype: list of float
%End

    QList<int> counts( int bins ) const;
%Docstring
 Returns the calculated list of the counts for the histogram bins.
 \param bins number of histogram bins
 :return: list of histogram counts
.. note::

   values must first be specified using setValues()
 :rtype: list of int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgshistogram.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
