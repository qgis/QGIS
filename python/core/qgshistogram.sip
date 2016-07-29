/** \ingroup core
 * \class QgsHistogram
 * \brief Calculator for a numeric histogram from a list of values.
 *
 * \note Added in version 2.9
 */

class QgsHistogram
{
%TypeHeaderCode
#include "qgshistogram.h"
%End

  public:

    QgsHistogram();

    virtual ~QgsHistogram();

    /** Assigns numeric source values for the histogram.
     * @param values list of doubles
     */
    void setValues( const QList<double>& values );

    /** Assigns numeric source values for the histogram from a vector layer's field or as the
     * result of an expression.
     * @param layer vector layer
     * @param fieldOrExpression field name or expression to be evaluated
     * @returns true if values were successfully set
     */
    bool setValues( QgsVectorLayer* layer, const QString& fieldOrExpression );

    /** Calculates the optimal bin width using the Freedman-Diaconis rule. Bins widths are
     * determined by the inter-quartile range of values and the number of values.
     * @returns optimal width for bins
     * @see optimalNumberBins
     * @note values must first be specified using @link setValues @endlink
     */
    double optimalBinWidth() const;

    /** Returns the optimal number of bins for the source values, calculated using the
     * Freedman-Diaconis rule. The number of bins are determined by the inter-quartile range
     * of values and the number of values.
     * @returns optimal number of bins
     * @see optimalBinWidth
     * @note values must first be specified using @link setValues @endlink
     */
    int optimalNumberBins() const;

    /** Returns a list of edges for the histogram for a specified number of bins. This list
     * will be length bins + 1, as both the first and last value are also included.
     * @param bins number of bins
     * @return list of bin edges
     * @note values must first be specified using @link setValues @endlink
     */
    QList<double> binEdges( int bins ) const;

    /** Returns the calculated list of the counts for the histogram bins.
     * @param bins number of histogram bins
     * @return list of histogram counts
     * @note values must first be specified using @link setValues @endlink
     */
    QList<int> counts( int bins ) const;

};

