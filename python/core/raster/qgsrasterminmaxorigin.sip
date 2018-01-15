/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterminmaxorigin.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterMinMaxOrigin
{
%Docstring
This class describes the origin of min/max values. It does not store by
itself the min/max values.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsrasterminmaxorigin.h"
%End
  public:

    static const double CUMULATIVE_CUT_LOWER;

    static const double CUMULATIVE_CUT_UPPER;

    static const double DEFAULT_STDDEV_FACTOR;

    enum Limits
    {
      None /PyName=None_/,
      MinMax,
      StdDev,
      CumulativeCut
    };

    enum Extent
    {
      //! Whole
      WholeRaster,
      //! Current
      CurrentCanvas,
      //! Constantly
      UpdatedCanvas
    };

    enum StatAccuracy
    {
      //! Exact
      Exact,
      //! Approximated
      Estimated
    };

    QgsRasterMinMaxOrigin();
%Docstring
 Default constructor.
%End

    bool operator ==( const QgsRasterMinMaxOrigin &other ) const;


    QgsRasterMinMaxOrigin::Limits limits() const;
%Docstring
 Return limits.
%End

    QgsRasterMinMaxOrigin::Extent extent() const;
%Docstring
 Return extent.
%End

    QgsRasterMinMaxOrigin::StatAccuracy statAccuracy() const;
%Docstring
 Return statistic accuracy.
%End

    double cumulativeCutLower() const;
%Docstring
 Return lower bound of cumulative cut method (between 0 and 1).
%End

    double cumulativeCutUpper() const;
%Docstring
 Return upper bound of cumulative cut method (between 0 and 1).
%End

    double stdDevFactor() const;
%Docstring
 Return factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
%End


    void setLimits( QgsRasterMinMaxOrigin::Limits limits );
%Docstring
 Set limits.
%End

    void setExtent( QgsRasterMinMaxOrigin::Extent extent );
%Docstring
 Set extent.
%End

    void setStatAccuracy( QgsRasterMinMaxOrigin::StatAccuracy accuracy );
%Docstring
 Set statistics accuracy.
%End

    void setCumulativeCutLower( double val );
%Docstring
 Set lower bound of cumulative cut method (between 0 and 1).
%End

    void setCumulativeCutUpper( double val );
%Docstring
 Set upper bound of cumulative cut method (between 0 and 1).
%End

    void setStdDevFactor( double val );
%Docstring
 Set factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
%End


    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;
%Docstring
 Serialize object.
%End

    void readXml( const QDomElement &elem );
%Docstring
 Deserialize object.
%End


    static QString limitsString( Limits limits );
%Docstring
 Return a string to serialize Limits
%End

    static Limits limitsFromString( const QString &limits );
%Docstring
 Deserialize Limits
%End

    static QString extentString( QgsRasterMinMaxOrigin::Extent extent );
%Docstring
 Return a string to serialize Extent
%End

    static QgsRasterMinMaxOrigin::Extent extentFromString( const QString &extent );
%Docstring
 Deserialize Extent
%End

    static QString statAccuracyString( QgsRasterMinMaxOrigin::StatAccuracy accuracy );
%Docstring
 Return a string to serialize StatAccuracy
%End

    static QgsRasterMinMaxOrigin::StatAccuracy statAccuracyFromString( const QString &accuracy );
%Docstring
 Deserialize StatAccuracy
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterminmaxorigin.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
