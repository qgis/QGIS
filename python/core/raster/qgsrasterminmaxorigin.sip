/** \class QgsRasterMinMaxOrigin
 * This class describes the origin of min/max values. It does not store by
 * itself the min/max values.
 */

class QgsRasterMinMaxOrigin
{
%TypeHeaderCode
#include <qgsrasterminmaxorigin.h>
%End
  public:
    //! \brief Default cumulative cut lower limit
    static const double CUMULATIVE_CUT_LOWER;

    //! \brief Default cumulative cut upper limit
    static const double CUMULATIVE_CUT_UPPER;

    //! \brief Default standard deviation factor
    static const double DEFAULT_STDDEV_FACTOR;

    //! \brief This enumerator describes the limits used to compute min/max values
    enum Limits
    {
      //! User defined.
      None  /PyName=None_/,
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
    QgsRasterMinMaxOrigin::Limits limits() const;

    //! \brief Return extent.
    QgsRasterMinMaxOrigin::Extent extent() const;

    //! \brief Return statistic accuracy.
    QgsRasterMinMaxOrigin::StatAccuracy statAccuracy() const;

    //! \brief Return lower bound of cumulative cut method (between 0 and 1).
    double cumulativeCutLower() const;

    //! \brief Return upper bound of cumulative cut method (between 0 and 1).
    double cumulativeCutUpper() const;

    //! \brief Return factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    double stdDevFactor() const;

    //////// Setter methods /////////////////////

    //! \brief Set limits.
    void setLimits(QgsRasterMinMaxOrigin::Limits limits);

    //! \brief Set extent.
    void setExtent(QgsRasterMinMaxOrigin::Extent extent);

    //! \brief Set statistics accuracy.
    void setStatAccuracy(QgsRasterMinMaxOrigin::StatAccuracy accuracy);

    //! \brief Set lower bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutLower(double val);

    //! \brief Set upper bound of cumulative cut method (between 0 and 1).
    void setCumulativeCutUpper(double val);

    //! \brief Set factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
    void setStdDevFactor(double val);

    //////// XML serialization /////////////////////

    //! \brief Serialize object.
    void writeXml( QDomDocument& doc, QDomElement& parentElem ) const;

    //! \brief Deserialize object.
    void readXml( const QDomElement& elem );

    //////// Static methods /////////////////////

     //! \brief Return a string to serialize Limits
    static QString limitsString( QgsRasterMinMaxOrigin::Limits limits );

     //! \brief Deserialize Limits
    static QgsRasterMinMaxOrigin::Limits limitsFromString( const QString& limits );

    //! \brief Return a string to serialize Extent
    static QString extentString( QgsRasterMinMaxOrigin::Extent extent );

    //! \brief Deserialize Extent
    static QgsRasterMinMaxOrigin::Extent extentFromString( const QString& extent );

    //! \brief Return a string to serialize StatAccuracy
    static QString statAccuracyString( QgsRasterMinMaxOrigin::StatAccuracy accuracy );

    //! \brief Deserialize StatAccuracy
    static QgsRasterMinMaxOrigin::StatAccuracy statAccuracyFromString( const QString& accuracy );
};
