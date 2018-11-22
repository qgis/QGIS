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
      //! Whole raster is used to compute statistics.
      WholeRaster,
      //! Current extent of the canvas (at the time of computation) is used to compute statistics.
      CurrentCanvas,
      //! Constantly updated extent of the canvas is used to compute statistics.
      UpdatedCanvas
    };

    enum StatAccuracy
    {
      //! Exact statistics.
      Exact,
      //! Approximated statistics.
      Estimated
    };

    QgsRasterMinMaxOrigin();
%Docstring
Default constructor.
%End

    bool operator ==( const QgsRasterMinMaxOrigin &other ) const;


    QgsRasterMinMaxOrigin::Limits limits() const;
%Docstring
Returns the raster limits.
%End

    QgsRasterMinMaxOrigin::Extent extent() const;
%Docstring
Returns the raster extent.
%End

    QgsRasterMinMaxOrigin::StatAccuracy statAccuracy() const;
%Docstring
Returns the raster statistic accuracy.
%End

    double cumulativeCutLower() const;
%Docstring
Returns the lower bound of cumulative cut method (between 0 and 1).
%End

    double cumulativeCutUpper() const;
%Docstring
Returns the upper bound of cumulative cut method (between 0 and 1).
%End

    double stdDevFactor() const;
%Docstring
Returns the factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
%End


    void setLimits( QgsRasterMinMaxOrigin::Limits limits );
%Docstring
Sets the limits.
%End

    void setExtent( QgsRasterMinMaxOrigin::Extent extent );
%Docstring
Sets the extent.
%End

    void setStatAccuracy( QgsRasterMinMaxOrigin::StatAccuracy accuracy );
%Docstring
Sets the statistics accuracy.
%End

    void setCumulativeCutLower( double val );
%Docstring
Sets the lower bound of cumulative cut method (between 0 and 1).
%End

    void setCumulativeCutUpper( double val );
%Docstring
Sets the upper bound of cumulative cut method (between 0 and 1).
%End

    void setStdDevFactor( double val );
%Docstring
Sets the factor f so that the min/max range is [ mean - f * stddev , mean + f * stddev ]
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
Returns a string to serialize Limits
%End

    static Limits limitsFromString( const QString &limits );
%Docstring
Deserialize Limits
%End

    static QString extentString( QgsRasterMinMaxOrigin::Extent extent );
%Docstring
Returns a string to serialize Extent
%End

    static QgsRasterMinMaxOrigin::Extent extentFromString( const QString &extent );
%Docstring
Deserialize Extent
%End

    static QString statAccuracyString( QgsRasterMinMaxOrigin::StatAccuracy accuracy );
%Docstring
Returns a string to serialize StatAccuracy
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
