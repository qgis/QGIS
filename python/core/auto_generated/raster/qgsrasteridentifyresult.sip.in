/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasteridentifyresult.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRasterIdentifyResult
{
%Docstring
Raster identify results container.
%End

%TypeHeaderCode
#include "qgsrasteridentifyresult.h"
%End
  public:

    QgsRasterIdentifyResult();
%Docstring
Constructor for QgsRasterIdentifyResult.
%End

    QgsRasterIdentifyResult( QgsRaster::IdentifyFormat format, const QMap<int, QVariant> &results );
%Docstring
Constructor. Creates valid result.

:param format: the result format
:param results: the results
%End

    QgsRasterIdentifyResult( const QgsError &error );
%Docstring
Constructor. Creates invalid result with error.

:param error: the error
%End

    virtual ~QgsRasterIdentifyResult();

    bool isValid() const;
%Docstring
Returns ``True`` if valid
%End

    QgsRaster::IdentifyFormat format() const;
%Docstring
Returns the results format.
%End

    QMap<int, QVariant> results() const;
%Docstring
Returns the identify results. Results are different for each format:
QgsRaster.IdentifyFormatValue: map of values for each band, keys are band numbers (from 1).
QgsRaster.IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer (WMS)
QgsRaster.IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
%End

    void setParams( const QMap<QString, QVariant> &params );
%Docstring
Sets map of optional parameters
%End

    QMap<QString, QVariant> params() const;
%Docstring
Gets map of optional parameters
%End

    QgsError error() const;
%Docstring
Returns the last error
%End

    void setError( const QgsError &error );
%Docstring
Sets the last error
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasteridentifyresult.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
